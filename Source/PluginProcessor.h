#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <vector>
#include <cmath>

struct CombFilter {
    std::vector<float> buf; int pos=0; float lpState=0.f;
    void init(int n){buf.assign(n,0.f);pos=0;lpState=0.f;}
    void reset(){std::fill(buf.begin(),buf.end(),0.f);lpState=0.f;}
    inline float tick(float x,float fb,float damp) noexcept {
        float out=buf[pos];
        lpState=out*(1.f-damp)+lpState*damp;
        buf[pos]=x+lpState*fb;
        if(++pos>=(int)buf.size())pos=0;
        return out;
    }
};
struct AllpassFilter {
    std::vector<float> buf; int pos=0;
    static constexpr float g=0.5f;
    void init(int n){buf.assign(n,0.f);pos=0;}
    void reset(){std::fill(buf.begin(),buf.end(),0.f);}
    inline float tick(float x) noexcept {
        float w=buf[pos],y=w-g*x; buf[pos]=x+g*y;
        if(++pos>=(int)buf.size())pos=0; return y;
    }
};
struct VibeStage {
    float c=0.f,xm1=0.f,ym1=0.f;
    void setFreq(float fc,float sr) noexcept {
        float t=std::tan(juce::MathConstants<float>::pi*fc/sr);
        c=(t-1.f)/(t+1.f);
    }
    float tick(float x) noexcept { float y=c*x+xm1-c*ym1; xm1=x;ym1=y;return y; }
};

//==============================================================================
class GhostSurfProcessor : public juce::AudioProcessor
{
public:
    static constexpr int NUM_PRESETS = 12;
    static constexpr int SCOPE_SIZE  = 1024;
    static constexpr int FREEZE_BUF  = 176400;
    static constexpr int FLAN_BUF    = 4096;

    GhostSurfProcessor();
    ~GhostSurfProcessor() override = default;

    void prepareToPlay(double sampleRate,int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& l) const override {
        return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();
    }
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "GhostSurf"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }

    int getNumPrograms()    override { return NUM_PRESETS; }
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int,const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // ── UI getters ──────────────────────────────────────────────────────────
    float getOutputLevel() const { return outputLevel.load(); }
    float getSurfScore()   const { return surfScore.load(); }
    int   getSurfCombo()   const { return surfCombo.load(); }
    const float* getScopePtr()      const { return scopeData; }
    int          getScopeWritePos() const { return scopeWritePos.load(std::memory_order_relaxed); }

    // Freeze
    void setFreezeActive(bool on);
    bool isFreezing() const { return freezeActive.load(); }

    // Specter filter
    void  setSpecterCutoff(float hz){ specterCutoff.store(juce::jlimit(20.f,18000.f,hz)); }
    void  setSpecterReso  (float r) { specterReso  .store(juce::jlimit(0.f,1.f,r)); }
    void  setSpecterShape (int s)   { specterShape .store(juce::jlimit(0,3,s)); }
    float getSpecterCutoff() const  { return specterCutoff.load(); }
    float getSpecterReso()   const  { return specterReso.load(); }
    int   getSpecterShape()  const  { return specterShape.load(); }

private:
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    int currentPreset=0;

    // Spring Reverb
    static constexpr int NUM_COMBS=4,NUM_AP=2;
    static const int BASE_COMB_LEN[NUM_COMBS];
    static const int BASE_AP_LEN[NUM_AP];
    CombFilter    combL[NUM_COMBS],combR[NUM_COMBS];
    AllpassFilter apL[NUM_AP],apR[NUM_AP];
    static float reverbFeedback(float decay,int delay,double sr);

    // Tremolo
    float tremoloPhase=0.f,currentBPM=120.f;

    // Uni-Vibe
    VibeStage vibeL[4],vibeR[4];
    float vibeLfoPhase=0.f;

    // Flanger
    float flanBufL[FLAN_BUF]{},flanBufR[FLAN_BUF]{};
    int   flanWritePos=0;
    float flanLfoPhase=0.f;

    // Auto-Wah (LFO → bandpass)
    float wahLfoPhase=0.f;
    float wahZ1L=0.f,wahZ2L=0.f,wahZ1R=0.f,wahZ2R=0.f;
    float wahB0=1.f,wahB1=0.f,wahB2=0.f,wahA1=0.f,wahA2=0.f;
    void  updateWahCoeffs(float fc,float q);

    // Spectral Freeze
    std::vector<float> freezeBufL,freezeBufR;
    int   freezeWritePos=0;
    float freezeReadL=0.f,freezeReadR=0.f;
    int   freezeLoopStart=0,freezeLoopLen=22050;
    float shimReadL=0.f,shimReadR=0.f,shimXL=0.f,shimXR=0.f;
    bool  prevFreezeState=false;
    std::atomic<bool> freezeActive{false};

    // Specter Filter (biquad)
    std::atomic<float> specterCutoff{2000.f};
    std::atomic<float> specterReso  {0.f};
    std::atomic<int>   specterShape {0};
    float spB0=1.f,spB1=0.f,spB2=0.f,spA1=0.f,spA2=0.f;
    float spZ1L=0.f,spZ2L=0.f,spZ1R=0.f,spZ2R=0.f;
    float chaosOffset=0.f;
    void  updateSpecterCoeffs(float fc,float reso,int shape);

    // Slide
    float slidePhase=0.f,slideTarget=0.f,slideCurrent=0.f;
    std::vector<float> slideDelayL,slideDelayR;
    int slideWritePos=0;

    // LoFi
    juce::Random rng;
    float lofiLpL=0.f,lofiLpR=0.f,noiseGateEnv=0.f;

    // DC blocker
    float dcXL=0.f,dcYL=0.f,dcXR=0.f,dcYR=0.f;

    // EQ
    using Coeffs  = juce::dsp::IIR::Coefficients<float>;
    using IIRFilt = juce::dsp::IIR::Filter<float>;
    IIRFilt bassL,bassR,trebleL,trebleR;
    float prevBassGain=-999.f,prevTrebleGain=-999.f; // cache for EQ coefficients

    double sr=44100.0;
    std::atomic<float> outputLevel{0.f};

    // Surf Score
    std::atomic<float> surfScore{0.f};
    std::atomic<int>   surfCombo{0};
    float scoreSmooth=0.f;
    int   comboCount=0;
    float comboTimer=0.f;

    // Scope
    float scopeData[SCOPE_SIZE]{};
    std::atomic<int> scopeWritePos{0};

    void updateEQ();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GhostSurfProcessor)
};
