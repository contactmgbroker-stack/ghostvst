#include "PluginProcessor.h"
#include "PluginEditor.h"

const int GhostSurfProcessor::BASE_COMB_LEN[NUM_COMBS]={1557,1617,1491,1422};
const int GhostSurfProcessor::BASE_AP_LEN[NUM_AP]={556,441};

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout GhostSurfProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back(std::make_unique<juce::AudioParameterFloat>("reverbMix",  "Reverb Mix",  0.f,1.f,0.40f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("reverbDecay","Decay",
        juce::NormalisableRange<float>(0.3f,6.f,0.01f),2.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("reverbTone", "Tone",
        juce::NormalisableRange<float>(500.f,8000.f,1.f,0.5f),3000.f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>("tremSpeed","Vitesse",
        juce::NormalisableRange<float>(0.1f,20.f,0.01f,0.5f),4.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("tremDepth","Profondeur",0.f,1.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterBool> ("tremSync","Sync BPM",false));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("tremDiv","Division",
        juce::StringArray{"1/4","1/8","1/16"},1));

    // Flanger
    p.push_back(std::make_unique<juce::AudioParameterFloat>("flangerRate","Flanger Rate",
        juce::NormalisableRange<float>(0.05f,8.f,0.01f,0.5f),0.5f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("flangerDepth","Flanger Depth",0.f,1.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("flangerFeedback","Flanger FB",0.f,0.92f,0.f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>("drive","Saturation",0.f,1.f,0.2f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("lofi", "Lo-Fi",     0.f,1.f,0.05f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("bass", "Basses",   -12.f,12.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("treble","Aigus",   -12.f,12.f,0.f));

    // Auto-Wah
    p.push_back(std::make_unique<juce::AudioParameterFloat>("wahDepth","Wah Depth",0.f,1.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("wahRate", "Wah Rate",
        juce::NormalisableRange<float>(0.05f,8.f,0.01f,0.5f),1.f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>("slideAmount","Glissement",
        juce::NormalisableRange<float>(0.f,1.f,0.01f),0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("slideSpeed","Vitesse Gliss",
        juce::NormalisableRange<float>(0.1f,10.f,0.01f),2.f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>("vibeSpeed","Vibe Speed",
        juce::NormalisableRange<float>(0.1f,10.f,0.01f,0.5f),1.5f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("vibeDepth","Vibe Depth",0.f,1.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterBool> ("vibeMode","Vibrato",false));

    p.push_back(std::make_unique<juce::AudioParameterFloat>("freezeGrain",  "Grain",  0.f,1.f,0.5f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("freezeShimmer","Shimmer",0.f,1.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("freezeDecay",  "Wet",    0.f,1.f,0.8f));

    return {p.begin(),p.end()};
}

GhostSurfProcessor::GhostSurfProcessor()
    : AudioProcessor(BusesProperties()
          .withInput ("Input", juce::AudioChannelSet::stereo(),true)
          .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
      apvts(*this,nullptr,"PARAMS",createParameters())
{
    std::fill(scopeData,scopeData+SCOPE_SIZE,0.f);
    std::fill(flanBufL,flanBufL+FLAN_BUF,0.f);
    std::fill(flanBufR,flanBufR+FLAN_BUF,0.f);
}

//==============================================================================
float GhostSurfProcessor::reverbFeedback(float decay,int delay,double sr_)
{
    return juce::jlimit(0.f,0.88f,std::pow(10.f,-3.f*(float)delay/((float)sr_*decay)));
}

void GhostSurfProcessor::prepareToPlay(double sampleRate,int)
{
    sr=sampleRate;
    const double sc=sr/44100.0;
    for(int i=0;i<NUM_COMBS;++i){
        combL[i].init((int)(BASE_COMB_LEN[i]*sc));
        combR[i].init((int)(BASE_COMB_LEN[i]*sc)+23);
    }
    for(int i=0;i<NUM_AP;++i){ int n=(int)(BASE_AP_LEN[i]*sc); apL[i].init(n);apR[i].init(n); }

    int sdMax=(int)(sr*0.12);
    slideDelayL.assign(sdMax,0.f); slideDelayR.assign(sdMax,0.f);
    slideWritePos=0; slideCurrent=slideTarget=slidePhase=0.f;

    freezeBufL.assign(FREEZE_BUF,0.f); freezeBufR.assign(FREEZE_BUF,0.f);
    freezeWritePos=0; freezeReadL=freezeReadR=0.f;
    freezeLoopLen=(int)(sr*0.5f);

    std::fill(flanBufL,flanBufL+FLAN_BUF,0.f);
    std::fill(flanBufR,flanBufR+FLAN_BUF,0.f);
    flanWritePos=0; flanLfoPhase=0.f; wahLfoPhase=0.f;
    wahZ1L=wahZ2L=wahZ1R=wahZ2R=0.f;

    bassL.reset(); bassR.reset(); trebleL.reset(); trebleR.reset();
    tremoloPhase=vibeLfoPhase=0.f;
    lofiLpL=lofiLpR=noiseGateEnv=0.f;
    dcXL=dcYL=dcXR=dcYR=0.f;
    spZ1L=spZ2L=spZ1R=spZ2R=0.f;
    outputLevel.store(0.f); surfScore.store(0.f); surfCombo.store(0);
    std::fill(scopeData,scopeData+SCOPE_SIZE,0.f);
    scopeWritePos.store(0);
    updateEQ();
}

void GhostSurfProcessor::updateEQ()
{
    float bG=juce::Decibels::decibelsToGain(apvts.getRawParameterValue("bass")->load());
    float tG=juce::Decibels::decibelsToGain(apvts.getRawParameterValue("treble")->load());
    *bassL.coefficients   = *Coeffs::makeLowShelf (sr,200.f, 0.707f,bG);
    *bassR.coefficients   = *Coeffs::makeLowShelf (sr,200.f, 0.707f,bG);
    *trebleL.coefficients = *Coeffs::makeHighShelf(sr,4000.f,0.707f,tG);
    *trebleR.coefficients = *Coeffs::makeHighShelf(sr,4000.f,0.707f,tG);
}

void GhostSurfProcessor::updateSpecterCoeffs(float fc,float reso,int shape)
{
    const float pi=juce::MathConstants<float>::pi;
    fc=juce::jlimit(30.f,17000.f,fc);
    float Q=0.5f+reso*19.5f;
    float w0=2.f*pi*fc/(float)sr;
    float cosw=std::cos(w0),sinw=std::sin(w0),alpha=sinw/(2.f*Q),a0;
    switch(shape){
    case 1: // bandpass
        spB0=sinw*0.5f/( a0=1.f+alpha); spB1=0.f; spB2=-sinw*0.5f/a0;
        spA1=-2.f*cosw/a0; spA2=(1.f-alpha)/a0; break;
    case 2: // highpass
        spB0=(1.f+cosw)*0.5f/(a0=1.f+alpha); spB1=-(1.f+cosw)/a0; spB2=spB0;
        spA1=-2.f*cosw/a0; spA2=(1.f-alpha)/a0; break;
    default: // lowpass + chaos
        spB0=(1.f-cosw)*0.5f/(a0=1.f+alpha); spB1=(1.f-cosw)/a0; spB2=spB0;
        spA1=-2.f*cosw/a0; spA2=(1.f-alpha)/a0; break;
    }
}

void GhostSurfProcessor::updateWahCoeffs(float fc,float q)
{
    fc=juce::jlimit(200.f,4500.f,fc);
    float w0=2.f*juce::MathConstants<float>::pi*fc/(float)sr;
    float cosw=std::cos(w0),sinw=std::sin(w0),alpha=sinw/(2.f*q);
    float a0=1.f+alpha;
    wahB0=sinw*0.5f/a0; wahB1=0.f; wahB2=-sinw*0.5f/a0;
    wahA1=-2.f*cosw/a0; wahA2=(1.f-alpha)/a0;
}

void GhostSurfProcessor::setFreezeActive(bool on)
{
    bool wasOff=!freezeActive.load(); freezeActive.store(on);
    if(on&&wasOff){
        float grain=apvts.getRawParameterValue("freezeGrain")->load();
        freezeLoopLen=juce::jlimit(2205,(int)(sr*3.f),(int)(sr*(0.1f+grain*2.9f)));
        freezeLoopStart=(freezeWritePos-freezeLoopLen+FREEZE_BUF)%FREEZE_BUF;
        freezeReadL=freezeReadR=0.f;
        shimReadL=0.f; shimReadR=(float)(freezeLoopLen/2); shimXL=shimXR=0.f;
    }
}

//==============================================================================
void GhostSurfProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    const int NS=buffer.getNumSamples(),NC=buffer.getNumChannels();
    auto* bL=buffer.getWritePointer(0);
    auto* bR=(NC>1)?buffer.getWritePointer(1):bL;

    const float reverbMix  =apvts.getRawParameterValue("reverbMix")  ->load();
    const float reverbDecay=apvts.getRawParameterValue("reverbDecay")->load();
    const float reverbTone =apvts.getRawParameterValue("reverbTone") ->load();
    const float tremSpeed  =apvts.getRawParameterValue("tremSpeed")  ->load();
    const float tremDepth  =apvts.getRawParameterValue("tremDepth")  ->load();
    const bool  tremSync   =apvts.getRawParameterValue("tremSync")   ->load()>0.5f;
    const int   tremDiv    =(int)apvts.getRawParameterValue("tremDiv")->load();
    const float flanRate   =apvts.getRawParameterValue("flangerRate")    ->load();
    const float flanDepth  =apvts.getRawParameterValue("flangerDepth")   ->load();
    const float flanFB     =apvts.getRawParameterValue("flangerFeedback")->load();
    const float drive      =apvts.getRawParameterValue("drive")      ->load();
    const float lofi       =apvts.getRawParameterValue("lofi")       ->load();
    const float slideAmt   =apvts.getRawParameterValue("slideAmount")->load();
    const float slideSpd   =apvts.getRawParameterValue("slideSpeed") ->load();
    const float wahDepth   =apvts.getRawParameterValue("wahDepth")   ->load();
    const float wahRate    =apvts.getRawParameterValue("wahRate")    ->load();
    const float vibeSpd    =apvts.getRawParameterValue("vibeSpeed")  ->load();
    const float vibeDepth  =apvts.getRawParameterValue("vibeDepth")  ->load();
    const bool  vibeVibrato=apvts.getRawParameterValue("vibeMode")   ->load()>0.5f;
    const float freezeShim =apvts.getRawParameterValue("freezeShimmer")->load();
    const float freezeDec  =apvts.getRawParameterValue("freezeDecay")  ->load();
    const bool  freezeOn   =freezeActive.load();

    if(auto* ph=getPlayHead()) if(auto pos=ph->getPosition()) if(auto bpm=pos->getBpm()) currentBPM=(float)*bpm;

    const double sc=sr/44100.0;
    const float fb  =reverbFeedback(reverbDecay,(int)(BASE_COMB_LEN[0]*sc),sr);
    const float damp=std::exp(-2.f*juce::MathConstants<float>::pi*reverbTone/(float)sr);
    const float cd  =juce::jlimit(0.05f,0.99f,1.f-damp);
    const float driveGain=1.f+drive*9.f;
    const float driveNorm=std::tanh(driveGain)+1e-6f;
    const float fuzzMix  =juce::jlimit(0.f,1.f,(drive-0.5f)*2.f);
    float tremRate=tremSpeed;
    if(tremSync){float dm=(tremDiv==0)?1.f:(tremDiv==1)?2.f:4.f;tremRate=currentBPM/60.f*dm;}
    const float tremInc=2.f*juce::MathConstants<float>::pi*tremRate/(float)sr;
    const float vibeInc=2.f*juce::MathConstants<float>::pi*vibeSpd/(float)sr;
    const float flanInc =2.f*juce::MathConstants<float>::pi*flanRate/(float)sr;
    const float wahInc  =2.f*juce::MathConstants<float>::pi*wahRate /(float)sr;
    const float noiseAmt=lofi*0.015f;
    const float tapeLpA =std::exp(-2.f*juce::MathConstants<float>::pi*(14000.f-lofi*9000.f)/(float)sr);
    const float ngAtk   =1.f-std::exp(-1.f/(0.005f*(float)sr));
    const float ngRel   =1.f-std::exp(-1.f/(0.200f*(float)sr));
    const float slideC  =1.f-std::exp(-slideSpd/(float)sr);
    const int   sdSize  =(int)slideDelayL.size();

    // Specter coeffs (once per block)
    { float fc=specterCutoff.load(),res=specterReso.load(); int sh=specterShape.load();
      if(sh==3){ chaosOffset+=(rng.nextFloat()-0.5f)*80.f; chaosOffset=juce::jlimit(-1000.f,1000.f,chaosOffset);
                 fc=juce::jlimit(80.f,16000.f,fc+chaosOffset); }
      updateSpecterCoeffs(fc,res,sh); }

    // Wah coeffs (once per block, LFO step)
    if(wahDepth>0.001f){
        wahLfoPhase+=wahInc*(float)NS;
        if(wahLfoPhase>juce::MathConstants<float>::twoPi) wahLfoPhase-=juce::MathConstants<float>::twoPi;
        float lfo=0.5f+0.5f*std::sin(wahLfoPhase);
        float wahFc=400.f+lfo*wahDepth*2800.f; // 400-3200 Hz sweep
        updateWahCoeffs(wahFc,3.5f);
    }

    updateEQ();
    float peak=0.f;
    int swp=scopeWritePos.load(std::memory_order_relaxed);

    for(int i=0;i<NS;++i)
    {
        float inL=bL[i],inR=bR[i];

        // ── DRIVE ──────────────────────────────────────────────────────────
        if(drive>0.001f){
            float tL=(std::tanh(inL*driveGain*1.1f)*0.6f+std::tanh(inL*driveGain*0.9f)*0.4f)/driveNorm;
            float tR=(std::tanh(inR*driveGain*1.1f)*0.6f+std::tanh(inR*driveGain*0.9f)*0.4f)/driveNorm;
            float fL=juce::jlimit(-0.9f,0.9f,inL*driveGain*2.f);
            float fR=juce::jlimit(-0.9f,0.9f,inR*driveGain*2.f);
            inL=tL+fuzzMix*(fL-tL); inR=tR+fuzzMix*(fR-tR);
        }

        // ── AUTO-WAH ───────────────────────────────────────────────────────
        if(wahDepth>0.001f){
            float sL=wahB0*inL+wahZ1L; wahZ1L=wahB1*inL-wahA1*sL+wahZ2L; wahZ2L=wahB2*inL-wahA2*sL; inL=sL;
            float sR=wahB0*inR+wahZ1R; wahZ1R=wahB1*inR-wahA1*sR+wahZ2R; wahZ2R=wahB2*inR-wahA2*sR; inR=sR;
        }

        // ── SLIDE ──────────────────────────────────────────────────────────
        if(slideAmt>0.001f){
            slideDelayL[slideWritePos]=inL; slideDelayR[slideWritePos]=inR;
            slideCurrent+=(slideTarget-slideCurrent)*slideC;
            slideTarget=std::sin(slidePhase)*slideAmt*80.f;
            slidePhase+=2.f*juce::MathConstants<float>::pi*0.5f/(float)sr;
            if(slidePhase>juce::MathConstants<float>::twoPi) slidePhase-=juce::MathConstants<float>::twoPi;
            int rp=(slideWritePos-40-(int)slideCurrent+sdSize)%sdSize;
            float fr=slideCurrent-std::floor(slideCurrent);
            int rp2=(rp+1)%sdSize;
            inL=inL*(1.f-slideAmt)+(slideDelayL[rp]*(1.f-fr)+slideDelayL[rp2]*fr)*slideAmt;
            inR=inR*(1.f-slideAmt)+(slideDelayR[rp]*(1.f-fr)+slideDelayR[rp2]*fr)*slideAmt;
            slideWritePos=(slideWritePos+1)%sdSize;
        }

        // ── SPRING REVERB ──────────────────────────────────────────────────
        if(reverbMix>0.001f){
            float dL=inL,dR=inR;
            for(int j=0;j<NUM_AP;++j){dL=apL[j].tick(dL);dR=apR[j].tick(dR);}
            float wL=0.f,wR=0.f;
            for(int j=0;j<NUM_COMBS;++j){wL+=combL[j].tick(dL,fb,cd);wR+=combR[j].tick(dR,fb,cd);}
            bL[i]=inL*(1.f-reverbMix)+wL*0.25f*reverbMix;
            bR[i]=inR*(1.f-reverbMix)+wR*0.25f*reverbMix;
        } else { bL[i]=inL; bR[i]=inR; }

        // ── EQ ─────────────────────────────────────────────────────────────
        bL[i]=trebleL.processSample(bassL.processSample(bL[i]));
        bR[i]=trebleR.processSample(bassR.processSample(bR[i]));

        // ── TREMOLO ────────────────────────────────────────────────────────
        if(tremDepth>0.001f){
            float mod=1.f-tremDepth*0.5f*(1.f+std::sin(tremoloPhase));
            bL[i]*=mod; bR[i]*=mod;
            tremoloPhase+=tremInc;
            if(tremoloPhase>=juce::MathConstants<float>::twoPi) tremoloPhase-=juce::MathConstants<float>::twoPi;
        }

        // ── FLANGER ────────────────────────────────────────────────────────
        if(flanDepth>0.001f){
            flanLfoPhase+=flanInc;
            if(flanLfoPhase>=juce::MathConstants<float>::twoPi) flanLfoPhase-=juce::MathConstants<float>::twoPi;
            float lfo=0.5f+0.5f*std::sin(flanLfoPhase);
            float delaySamples=1.f+lfo*flanDepth*(float)(FLAN_BUF/4); // 1 to ~23ms @44100
            int   di=(int)delaySamples;
            float df=delaySamples-di;
            int r0=(flanWritePos-di+FLAN_BUF)%FLAN_BUF;
            int r1=(flanWritePos-di-1+FLAN_BUF)%FLAN_BUF;
            float fL=flanBufL[r0]*(1.f-df)+flanBufL[r1]*df;
            float fR=flanBufR[r0]*(1.f-df)+flanBufR[r1]*df;
            flanBufL[flanWritePos]=bL[i]+fL*flanFB;
            flanBufR[flanWritePos]=bR[i]+fR*flanFB;
            bL[i]=bL[i]*0.5f+fL*0.5f;
            bR[i]=bR[i]*0.5f+fR*0.5f;
            flanWritePos=(flanWritePos+1)%FLAN_BUF;
        }

        // ── UNI-VIBE ────────────────────────────────────────────────────────
        if(vibeDepth>0.001f){
            vibeLfoPhase+=vibeInc;
            if(vibeLfoPhase>=juce::MathConstants<float>::twoPi) vibeLfoPhase-=juce::MathConstants<float>::twoPi;
            if(i%16==0){
                for(int s=0;s<4;++s){
                    float lfo=0.5f+0.5f*std::sin(vibeLfoPhase+s*0.28f);
                    vibeL[s].setFreq(80.f+lfo*1600.f,(float)sr);
                    vibeR[s].setFreq(80.f+lfo*1600.f,(float)sr);
                }
            }
            float wL=bL[i],wR=bR[i];
            for(int s=0;s<4;++s){wL=vibeL[s].tick(wL);wR=vibeR[s].tick(wR);}
            if(vibeVibrato){ bL[i]=bL[i]*(1.f-vibeDepth)+wL*vibeDepth; bR[i]=bR[i]*(1.f-vibeDepth)+wR*vibeDepth; }
            else            { bL[i]+=wL*vibeDepth*0.5f; bR[i]+=wR*vibeDepth*0.5f; }
        }

        // ── SPECTER FILTER ─────────────────────────────────────────────────
        { float sL=spB0*bL[i]+spZ1L; spZ1L=spB1*bL[i]-spA1*sL+spZ2L; spZ2L=spB2*bL[i]-spA2*sL; bL[i]=sL;
          float sR=spB0*bR[i]+spZ1R; spZ1R=spB1*bR[i]-spA1*sR+spZ2R; spZ2R=spB2*bR[i]-spA2*sR; bR[i]=sR; }

        // ── SPECTRAL FREEZE ─────────────────────────────────────────────────
        freezeBufL[freezeWritePos]=bL[i]; freezeBufR[freezeWritePos]=bR[i];
        freezeWritePos=(freezeWritePos+1)%FREEZE_BUF;
        if(freezeOn){
            const int LL=freezeLoopLen,LS=freezeLoopStart;
            auto readLoop=[&](float& rp,const std::vector<float>& buf)->float{
                int p0=(int)rp,p1=(p0+1)%LL; float fr=rp-p0;
                float s=buf[(LS+p0)%FREEZE_BUF]*(1.f-fr)+buf[(LS+p1)%FREEZE_BUF]*fr;
                rp+=1.f; if(rp>=(float)LL)rp=0.f; return s;
            };
            float fL=readLoop(freezeReadL,freezeBufL);
            float fR=readLoop(freezeReadR,freezeBufR);
            if(freezeShim>0.01f){
                auto readShim=[&](float& rp,float& xp,const std::vector<float>& buf)->float{
                    rp+=2.f; if(rp>=(float)LL)rp-=(float)LL;
                    int p0=(int)rp; float fr=rp-p0;
                    float s=buf[(LS+p0)%FREEZE_BUF]*(1.f-fr)+buf[(LS+(p0+1)%LL)%FREEZE_BUF]*fr;
                    xp+=1.f/(float)(sr*0.025f); if(xp>1.f){xp=0.f;rp=std::fmod(rp+(float)(LL/2),(float)LL);}
                    float xw=xp<0.5f?xp*2.f:(1.f-xp)*2.f; return s*xw;
                };
                fL+=readShim(shimReadL,shimXL,freezeBufL)*freezeShim;
                fR+=readShim(shimReadR,shimXR,freezeBufR)*freezeShim;
            }
            bL[i]=bL[i]*(1.f-freezeDec)+fL*freezeDec;
            bR[i]=bR[i]*(1.f-freezeDec)+fR*freezeDec;
        }

        // ── LOFI ───────────────────────────────────────────────────────────
        if(lofi>0.001f){
            float absOut=std::max(std::abs(bL[i]),std::abs(bR[i]));
            noiseGateEnv+=(absOut-noiseGateEnv)*(absOut>noiseGateEnv?ngAtk:ngRel);
            float nm=juce::jlimit(0.f,1.f,noiseGateEnv*30.f);
            lofiLpL=lofiLpL*tapeLpA+bL[i]*(1.f-tapeLpA);
            lofiLpR=lofiLpR*tapeLpA+bR[i]*(1.f-tapeLpA);
            bL[i]=lofiLpL+(rng.nextFloat()*2.f-1.f)*noiseAmt*nm;
            bR[i]=lofiLpR+(rng.nextFloat()*2.f-1.f)*noiseAmt*nm;
        }

        // ── DC BLOCKER + LIMITER ────────────────────────────────────────────
        float dcL=bL[i]-dcXL+0.995f*dcYL; dcXL=bL[i]; dcYL=dcL; bL[i]=std::tanh(dcL);
        float dcR=bR[i]-dcXR+0.995f*dcYR; dcXR=bR[i]; dcYR=dcR; bR[i]=std::tanh(dcR);

        float lvl=std::max(std::abs(bL[i]),std::abs(bR[i]));
        if(lvl>peak) peak=lvl;
        scopeData[swp]=(bL[i]+bR[i])*0.5f;
        swp=(swp+1)%SCOPE_SIZE;
    }

    scopeWritePos.store(swp,std::memory_order_relaxed);
    float cur=outputLevel.load();
    outputLevel.store(peak>cur?peak:cur*std::exp(-1.f/(0.3f*(float)sr/(float)NS)));

    // Surf Score
    { float t=peak*80.f; scoreSmooth+=(t-scoreSmooth)*0.12f;
      comboTimer-=(float)NS/(float)sr;
      if(peak>0.15f){comboCount=juce::jmin(comboCount+1,8);comboTimer=1.5f;}
      else if(comboTimer<0.f){comboCount=juce::jmax(0,comboCount-1);}
      surfScore.store(juce::jlimit(0.f,100.f,scoreSmooth*(1.f+(float)comboCount*0.3f)));
      surfCombo.store(comboCount); }
}

//==============================================================================
struct PresetData {
    const char* name;
    float reverbMix,reverbDecay,reverbTone;
    float tremSpeed,tremDepth; bool tremSync; int tremDiv;
    float flanRate,flanDepth,flanFB;
    float drive,lofi,bass,treble;
    float wahDepth,wahRate;
    float slideAmt;
    float vibeSpd,vibeDepth; bool vibeVibrato;
    float freezeGrain,freezeShimmer,freezeDecay;
};

static const PresetData PRESETS[GhostSurfProcessor::NUM_PRESETS]={
    {"The Cure - A Forest",     0.78f,5.5f,2200.f, 3.0f,0.05f,true, 1, 0.3f,0.4f,0.5f,  0.05f,0.06f, 4.f,-1.f, 0.f,0.5f, 0.00f, 0.8f,0.35f,false, 0.6f,0.00f,0.70f},
    {"Lil Peep - Ghost",        0.62f,4.0f,3500.f, 2.0f,0.28f,false,1, 0.1f,0.2f,0.3f,  0.18f,0.38f, 3.f,-3.f, 0.f,1.0f, 0.00f, 1.5f,0.20f,false, 0.7f,0.35f,0.80f},
    {"Iggy Pop - Raw Power",    0.20f,1.5f,7000.f, 8.0f,0.65f,false,1, 0.0f,0.0f,0.0f,  0.95f,0.15f, 4.f, 6.f, 0.f,1.0f, 0.15f, 2.0f,0.10f,false, 0.2f,0.00f,0.00f},
    {"Surf Clean",              0.40f,2.8f,4200.f, 4.5f,0.32f,false,1, 0.0f,0.0f,0.0f,  0.10f,0.04f, 1.f, 2.f, 0.f,1.0f, 0.00f, 1.2f,0.00f,false, 0.4f,0.00f,0.00f},
    {"Night Waves",             0.70f,5.2f,3200.f, 1.5f,0.38f,false,1, 0.2f,0.5f,0.4f,  0.18f,0.10f, 2.f,-1.f, 0.f,0.5f, 0.00f, 0.5f,0.45f,false, 0.8f,0.60f,0.90f},
    {"Dick Dale - Misirlou",    0.38f,2.0f,5500.f,15.0f,0.90f,true, 1, 0.0f,0.0f,0.0f,  0.08f,0.04f, 2.f, 4.f, 0.f,1.0f, 0.00f, 0.3f,0.00f,false, 0.3f,0.00f,0.00f},
    {"The Pixies - Monkey",     0.55f,4.5f,3800.f, 1.5f,0.15f,false,1, 0.1f,0.3f,0.2f,  0.12f,0.05f, 0.f,-2.f, 0.f,1.0f, 0.00f, 2.5f,0.30f,false, 0.5f,0.10f,0.40f},
    {"Joy Division - Trans.",   0.82f,5.8f,1800.f, 0.5f,0.10f,false,1, 0.4f,0.6f,0.3f,  0.08f,0.12f,-2.f,-4.f, 0.f,0.8f, 0.00f, 1.0f,0.50f,true,  0.9f,0.00f,0.60f},
    {"Jack White - Slide",      0.25f,2.5f,6000.f, 3.0f,0.20f,false,1, 0.0f,0.0f,0.0f,  0.70f,0.08f, 5.f, 3.f, 0.f,1.0f, 0.45f, 1.8f,0.15f,false, 0.3f,0.00f,0.00f},
    {"Haunted Motel",           0.68f,4.5f,2800.f, 5.0f,0.42f,false,1, 0.3f,0.7f,0.5f,  0.15f,0.15f, 0.f, 1.f, 0.f,1.0f, 0.00f, 3.5f,0.60f,false, 0.7f,0.75f,0.85f},
    {"Jimi Hendrix - Woodstock",0.30f,2.0f,6000.f, 0.5f,0.00f,false,1, 0.0f,0.0f,0.0f,  0.82f,0.12f, 3.f, 5.f, 0.65f,2.f,0.12f, 2.2f,0.80f,false, 0.3f,0.15f,0.00f},
    {"Nile Rodgers - Le Freak", 0.12f,0.8f,5500.f, 0.5f,0.00f,false,1, 0.0f,0.0f,0.0f,  0.03f,0.02f,-1.f, 6.f, 0.5f,4.f,0.00f, 4.0f,0.10f,false, 0.4f,0.20f,0.30f},
};

void GhostSurfProcessor::setCurrentProgram(int index)
{
    if(index<0||index>=NUM_PRESETS)return;
    currentPreset=index;
    const auto& d=PRESETS[index];
    auto set=[&](const char* id,float v){
        if(auto* p=apvts.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(v));
    };
    set("reverbMix",d.reverbMix); set("reverbDecay",d.reverbDecay); set("reverbTone",d.reverbTone);
    set("tremSpeed",d.tremSpeed); set("tremDepth",d.tremDepth);
    set("flangerRate",d.flanRate); set("flangerDepth",d.flanDepth); set("flangerFeedback",d.flanFB);
    set("drive",d.drive); set("lofi",d.lofi); set("bass",d.bass); set("treble",d.treble);
    set("wahDepth",d.wahDepth); set("wahRate",d.wahRate);
    set("slideAmount",d.slideAmt); set("slideSpeed",3.f);
    set("vibeSpeed",d.vibeSpd); set("vibeDepth",d.vibeDepth);
    set("freezeGrain",d.freezeGrain); set("freezeShimmer",d.freezeShimmer); set("freezeDecay",d.freezeDecay);
    if(auto* p=apvts.getParameter("tremSync"))  p->setValueNotifyingHost(d.tremSync?1.f:0.f);
    if(auto* p=apvts.getParameter("tremDiv"))   p->setValueNotifyingHost(p->convertTo0to1((float)d.tremDiv));
    if(auto* p=apvts.getParameter("vibeMode"))  p->setValueNotifyingHost(d.vibeVibrato?1.f:0.f);
}

const juce::String GhostSurfProcessor::getProgramName(int i)
{ return (i>=0&&i<NUM_PRESETS)?PRESETS[i].name:juce::String{}; }

void GhostSurfProcessor::getStateInformation(juce::MemoryBlock& d)
{ auto x=apvts.copyState().createXml(); copyXmlToBinary(*x,d); }
void GhostSurfProcessor::setStateInformation(const void* d,int s)
{ auto x=getXmlFromBinary(d,s); if(x&&x->hasTagName(apvts.state.getType())) apvts.replaceState(juce::ValueTree::fromXml(*x)); }

juce::AudioProcessorEditor* GhostSurfProcessor::createEditor(){ return new GhostSurfEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){ return new GhostSurfProcessor(); }
