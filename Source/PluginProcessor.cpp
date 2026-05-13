#include "PluginProcessor.h"
#include "PluginEditor.h"

const int GhostSurfProcessor::BASE_COMB_LEN[NUM_COMBS] = { 1557, 1617, 1491, 1422 };
const int GhostSurfProcessor::BASE_AP_LEN[NUM_AP]      = { 556, 441 };

// 8-step gate patterns (1=on, 0=off) — one step = one 1/8 note
static const float ARP_PATTERNS[4][8] = {
    { 1.f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, 0.f }, // 1/8 Gate
    { 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f }, // Triolets
    { 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f }, // A Forest (sparse)
    { 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f, 1.f }, // Syncope
};

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout GhostSurfProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    // Reverb
    p.push_back(std::make_unique<juce::AudioParameterFloat>("reverbMix",   "Reverb Mix",   0.0f, 1.0f, 0.35f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("reverbDecay", "Decay",
        juce::NormalisableRange<float>(0.3f, 6.0f, 0.01f), 2.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("reverbTone",  "Tone",
        juce::NormalisableRange<float>(500.f, 8000.f, 1.f, 0.5f), 3000.f));

    // Tremolo
    p.push_back(std::make_unique<juce::AudioParameterFloat>("tremSpeed", "Vitesse",
        juce::NormalisableRange<float>(0.1f, 20.f, 0.01f, 0.5f), 4.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("tremDepth", "Intensite", 0.f, 1.f, 0.f));
    p.push_back(std::make_unique<juce::AudioParameterBool> ("tremSync",  "Sync BPM", false));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("tremDiv", "Division",
        juce::StringArray{"1/4","1/8","1/16"}, 1));

    // Drive
    p.push_back(std::make_unique<juce::AudioParameterFloat>("drive", "Saturation", 0.f, 1.f, 0.2f));

    // LoFi
    p.push_back(std::make_unique<juce::AudioParameterFloat>("lofi", "Lo-Fi", 0.f, 1.f, 0.1f));

    // EQ
    p.push_back(std::make_unique<juce::AudioParameterFloat>("bass",   "Basses",   -12.f, 12.f, 0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("treble", "Aigus",    -12.f, 12.f, 0.f));

    // Guitar modes
    p.push_back(std::make_unique<juce::AudioParameterChoice>("guitarMode", "Mode Guitare",
        juce::StringArray{"Normal","Auto-Swell","Arpege"}, 0));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("swellAttack", "Attaque Swell",
        juce::NormalisableRange<float>(0.01f, 2.0f, 0.01f), 0.3f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("swellAmount", "Intensite Swell", 0.f, 1.f, 0.7f));

    // Bottleneck / Slide
    p.push_back(std::make_unique<juce::AudioParameterFloat>("slideAmount", "Glissement",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("slideSpeed",  "Vitesse Gliss",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.01f), 2.f));

    // Arpeggiator pattern
    p.push_back(std::make_unique<juce::AudioParameterChoice>("arpPattern", "Pattern Arpege",
        juce::StringArray{"1/8 Gate","Triolets","A Forest","Syncope"}, 0));

    return { p.begin(), p.end() };
}

GhostSurfProcessor::GhostSurfProcessor()
    : AudioProcessor(BusesProperties()
          .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameters())
{}

//==============================================================================
float GhostSurfProcessor::reverbFeedback(float decaySec, int delaySamples, double sampleRate)
{
    float D = (float)delaySamples / (float)sampleRate;
    return juce::jlimit(0.f, 0.88f, std::pow(10.f, -3.f * D / decaySec));
}

void GhostSurfProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    sr = sampleRate;
    const double scale = sampleRate / 44100.0;

    for (int i = 0; i < NUM_COMBS; ++i) {
        combL[i].init((int)(BASE_COMB_LEN[i] * scale));
        combR[i].init((int)(BASE_COMB_LEN[i] * scale) + 23);
    }
    for (int i = 0; i < NUM_AP; ++i) {
        int len = (int)(BASE_AP_LEN[i] * scale);
        apL[i].init(len); apR[i].init(len);
    }

    int slideMax = (int)(sampleRate * 0.1);
    slideDelayL.assign(slideMax, 0.f);
    slideDelayR.assign(slideMax, 0.f);
    slideWritePos = 0;
    slideCurrent = slideTarget = 0.f;

    bassL.reset(); bassR.reset(); trebleL.reset(); trebleR.reset();
    tremoloPhase = gatePhase = 0.f;
    swellEnvL = swellEnvR = 0.f;
    lofiLpL = lofiLpR = 0.f;
    dcXL = dcYL = dcXR = dcYR = 0.f;
    outputLevel.store(0.f);
    updateEQ();
}

void GhostSurfProcessor::updateEQ()
{
    float bG = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("bass")->load());
    float tG = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("treble")->load());
    *bassL.coefficients   = *Coeffs::makeLowShelf (sr, 200.f,  0.707f, bG);
    *bassR.coefficients   = *Coeffs::makeLowShelf (sr, 200.f,  0.707f, bG);
    *trebleL.coefficients = *Coeffs::makeHighShelf(sr, 4000.f, 0.707f, tG);
    *trebleR.coefficients = *Coeffs::makeHighShelf(sr, 4000.f, 0.707f, tG);
}

//==============================================================================
void GhostSurfProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numCh      = buffer.getNumChannels();
    auto* bufL = buffer.getWritePointer(0);
    auto* bufR = (numCh > 1) ? buffer.getWritePointer(1) : bufL;

    // Read params
    const float reverbMix   = apvts.getRawParameterValue("reverbMix")->load();
    const float reverbDecay = apvts.getRawParameterValue("reverbDecay")->load();
    const float reverbTone  = apvts.getRawParameterValue("reverbTone")->load();
    const float tremSpeed   = apvts.getRawParameterValue("tremSpeed")->load();
    const float tremDepth   = apvts.getRawParameterValue("tremDepth")->load();
    const bool  tremSync    = apvts.getRawParameterValue("tremSync")->load() > 0.5f;
    const int   tremDiv     = (int)apvts.getRawParameterValue("tremDiv")->load();
    const float drive       = apvts.getRawParameterValue("drive")->load();
    const float lofi        = apvts.getRawParameterValue("lofi")->load();
    const int   guitarMode  = (int)apvts.getRawParameterValue("guitarMode")->load();
    const float swellAtk    = apvts.getRawParameterValue("swellAttack")->load();
    const float swellAmt    = apvts.getRawParameterValue("swellAmount")->load();
    const float slideAmt    = apvts.getRawParameterValue("slideAmount")->load();
    const float slideSpd    = apvts.getRawParameterValue("slideSpeed")->load();
    const int   arpPat      = (int)apvts.getRawParameterValue("arpPattern")->load();

    // Get BPM from host
    if (auto* ph = getPlayHead()) {
        if (auto pos = ph->getPosition()) {
            if (auto bpm = pos->getBpm())
                currentBPM = (float)*bpm;
        }
    }

    // Reverb params
    const double scale  = sr / 44100.0;
    const float  fb     = reverbFeedback(reverbDecay, (int)(BASE_COMB_LEN[0]*scale), sr);
    const float  damp   = std::exp(-2.f * juce::MathConstants<float>::pi * reverbTone / (float)sr);
    const float  combDamp = juce::jlimit(0.05f, 0.99f, 1.f - damp);

    // Drive — tube saturation blends into fuzz above drive=0.5
    const float driveGain = 1.f + drive * 9.f;
    const float driveNorm = std::tanh(driveGain) + 1e-6f;
    const float fuzzMix   = juce::jlimit(0.f, 1.f, (drive - 0.5f) * 2.f);

    // LoFi
    const float noiseAmt = lofi * 0.012f;
    const float tapeLpA  = std::exp(-2.f * juce::MathConstants<float>::pi * (14000.f - lofi*8000.f) / (float)sr);

    // Tremolo rate
    float tremRate = tremSpeed;
    if (tremSync) {
        float divMult = (tremDiv == 0) ? 1.f : (tremDiv == 1) ? 2.f : 4.f;
        tremRate = currentBPM / 60.f * divMult;
    }
    const float tremInc = 2.f * juce::MathConstants<float>::pi * tremRate / (float)sr;

    // Auto-swell: coeff from attack time
    const float swellCoeff = 1.f - std::exp(-1.f / (swellAtk * (float)sr));

    // Gate rate — gatePhase goes 0→8 over 1 bar (8 eighth notes)
    // Each step in ARP_PATTERNS = 1 eighth note
    const float gateInc = (currentBPM / 60.f * 2.f) / (float)sr;

    // Slide: smooth portamento
    const float slideCoeff = 1.f - std::exp(-slideSpd / (float)sr);
    const int   slideDelaySize = (int)slideDelayL.size();

    updateEQ();

    float peakLevel = 0.f;

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = bufL[i], inR = bufR[i];

        // ── SATURATION ──────────────────────────────────────────────────────
        if (drive > 0.001f) {
            // Tube saturation (tanh) blends to hard fuzz clipping at high drive
            float tubL = (std::tanh(inL * driveGain * 1.1f) * 0.6f + std::tanh(inL * driveGain * 0.9f) * 0.4f) / driveNorm;
            float tubR = (std::tanh(inR * driveGain * 1.1f) * 0.6f + std::tanh(inR * driveGain * 0.9f) * 0.4f) / driveNorm;
            float fzzL = juce::jlimit(-0.9f, 0.9f, inL * driveGain * 2.f);
            float fzzR = juce::jlimit(-0.9f, 0.9f, inR * driveGain * 2.f);
            inL = tubL + fuzzMix * (fzzL - tubL);
            inR = tubR + fuzzMix * (fzzR - tubR);
        }

        // ── LOFI ─────────────────────────────────────────────────────────────
        if (lofi > 0.001f) {
            inL += (rng.nextFloat() * 2.f - 1.f) * noiseAmt;
            inR += (rng.nextFloat() * 2.f - 1.f) * noiseAmt;
            lofiLpL = lofiLpL * tapeLpA + inL * (1.f - tapeLpA);
            lofiLpR = lofiLpR * tapeLpA + inR * (1.f - tapeLpA);
            inL = lofiLpL; inR = lofiLpR;
        }

        // ── BOTTLENECK / SLIDE ───────────────────────────────────────────────
        if (slideAmt > 0.001f) {
            slideDelayL[slideWritePos] = inL;
            slideDelayR[slideWritePos] = inR;

            slideCurrent += (slideTarget - slideCurrent) * slideCoeff;
            slideTarget = std::sin(slidePhase) * slideAmt * 80.f;
            slidePhase += 2.f * juce::MathConstants<float>::pi * 0.5f / (float)sr;
            if (slidePhase > juce::MathConstants<float>::twoPi) slidePhase -= juce::MathConstants<float>::twoPi;

            float readOffset = slideCurrent;
            int readPos  = (slideWritePos - 40 - (int)readOffset + slideDelaySize) % slideDelaySize;
            float frac   = readOffset - std::floor(readOffset);
            int readPos2 = (readPos + 1) % slideDelaySize;

            float slideL = slideDelayL[readPos] * (1.f - frac) + slideDelayL[readPos2] * frac;
            float slideR = slideDelayR[readPos] * (1.f - frac) + slideDelayR[readPos2] * frac;

            inL = inL * (1.f - slideAmt) + slideL * slideAmt;
            inR = inR * (1.f - slideAmt) + slideR * slideAmt;

            slideWritePos = (slideWritePos + 1) % slideDelaySize;
        }

        // ── SPRING REVERB ────────────────────────────────────────────────────
        if (reverbMix > 0.001f) {
            float diffL = inL, diffR = inR;
            for (int j = 0; j < NUM_AP; ++j) {
                diffL = apL[j].tick(diffL);
                diffR = apR[j].tick(diffR);
            }
            float wetL = 0.f, wetR = 0.f;
            for (int j = 0; j < NUM_COMBS; ++j) {
                wetL += combL[j].tick(diffL, fb, combDamp);
                wetR += combR[j].tick(diffR, fb, combDamp);
            }
            wetL *= 0.25f; wetR *= 0.25f;
            bufL[i] = inL * (1.f - reverbMix) + wetL * reverbMix;
            bufR[i] = inR * (1.f - reverbMix) + wetR * reverbMix;
        } else {
            bufL[i] = inL; bufR[i] = inR;
        }

        // ── EQ ───────────────────────────────────────────────────────────────
        bufL[i] = trebleL.processSample(bassL.processSample(bufL[i]));
        bufR[i] = trebleR.processSample(bassR.processSample(bufR[i]));

        // ── GUITAR MODES ─────────────────────────────────────────────────────
        if (guitarMode == 1) {
            // AUTO-SWELL: slow volume attack
            float envL = std::abs(bufL[i]);
            float envR = std::abs(bufR[i]);
            swellEnvL += (envL - swellEnvL) * swellCoeff;
            swellEnvR += (envR - swellEnvR) * swellCoeff;
            float gainL = swellEnvL > 0.0001f ? juce::jlimit(0.f, 1.f, swellEnvL / (std::abs(bufL[i]) + 0.0001f)) : 0.f;
            float gainR = swellEnvR > 0.0001f ? juce::jlimit(0.f, 1.f, swellEnvR / (std::abs(bufR[i]) + 0.0001f)) : 0.f;
            bufL[i] *= (1.f - swellAmt) + swellAmt * gainL;
            bufR[i] *= (1.f - swellAmt) + swellAmt * gainR;
        }
        else if (guitarMode == 2) {
            // ARPEGE: pattern gate synced to BPM (8 steps = 1 bar)
            gatePhase += gateInc;
            if (gatePhase >= 8.f) gatePhase -= 8.f;
            int   step       = juce::jlimit(0, 7, (int)gatePhase);
            float gateTarget = ARP_PATTERNS[juce::jlimit(0,3,arpPat)][step];
            // Fast attack, slower release for natural feel
            float sc = gateTarget > gateSmoothed ? 0.25f : 0.05f;
            gateSmoothed += (gateTarget - gateSmoothed) * sc;
            bufL[i] *= gateSmoothed;
            bufR[i] *= gateSmoothed;
        }

        // ── TREMOLO ──────────────────────────────────────────────────────────
        if (tremDepth > 0.001f) {
            float mod = 1.f - tremDepth * 0.5f * (1.f + std::sin(tremoloPhase));
            bufL[i] *= mod; bufR[i] *= mod;
            tremoloPhase += tremInc;
            if (tremoloPhase >= juce::MathConstants<float>::twoPi)
                tremoloPhase -= juce::MathConstants<float>::twoPi;
        }

        // ── DC BLOCKER ───────────────────────────────────────────────────────
        float dcL = bufL[i] - dcXL + 0.995f * dcYL;
        dcXL = bufL[i]; dcYL = dcL; bufL[i] = dcL;
        float dcR = bufR[i] - dcXR + 0.995f * dcYR;
        dcXR = bufR[i]; dcYR = dcR; bufR[i] = dcR;

        // ── OUTPUT LIMITER ───────────────────────────────────────────────────
        bufL[i] = std::tanh(bufL[i]);
        bufR[i] = std::tanh(bufR[i]);

        float lvl = std::max(std::abs(bufL[i]), std::abs(bufR[i]));
        if (lvl > peakLevel) peakLevel = lvl;
    }

    float cur = outputLevel.load();
    outputLevel.store(peakLevel > cur ? peakLevel :
                      cur * std::exp(-1.f / (0.4f * (float)sr / (float)numSamples)));
}

//==============================================================================
struct PresetData {
    const char* name;
    float reverbMix, reverbDecay, reverbTone;
    float tremSpeed, tremDepth;
    bool  tremSync; int tremDiv;
    float drive, lofi, bass, treble;
    int   guitarMode;
    float swellAtk, swellAmt, slideAmt;
    int   arpPattern;
};

static const PresetData PRESETS[5] = {
    // name                  rvbMix  rvbDcy  rvbTon  tSpd  tDpt  sync  div   drv   lofi  bass  treb  mode  sAtk  sAmt  slAmt arpPat
    { "The Cure - A Forest", 0.75f,  5.5f,  2200.f,  3.0f, 0.05f, true,  1,  0.05f, 0.08f,  4.f, -1.f,  2,  0.5f, 0.7f, 0.0f,  2 },
    { "Lil Peep - Ghost",    0.60f,  4.0f,  3500.f,  2.0f, 0.30f, false, 1,  0.18f, 0.42f,  3.f, -3.f,  1,  0.6f, 0.85f,0.0f,  0 },
    { "Iggy Pop - Dog",      0.20f,  1.5f,  7000.f,  8.0f, 0.65f, false, 1,  0.95f, 0.18f,  4.f,  6.f,  0,  0.3f, 0.7f, 0.15f, 0 },
    { "Surf Clean",          0.40f,  3.0f,  4200.f,  4.0f, 0.30f, false, 1,  0.10f, 0.05f,  1.f,  2.f,  0,  0.3f, 0.7f, 0.0f,  0 },
    { "Night Waves",         0.68f,  5.0f,  3200.f,  2.0f, 0.40f, false, 1,  0.20f, 0.12f,  2.f, -1.f,  1,  0.5f, 0.8f, 0.0f,  0 },
};

void GhostSurfProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= 5) return;
    currentPreset = index;
    const auto& d = PRESETS[index];
    auto set = [&](const char* id, float val) {
        if (auto* param = apvts.getParameter(id)) param->setValueNotifyingHost(param->convertTo0to1(val));
    };
    set("reverbMix", d.reverbMix); set("reverbDecay", d.reverbDecay);
    set("reverbTone", d.reverbTone); set("tremSpeed", d.tremSpeed);
    set("tremDepth", d.tremDepth); set("drive", d.drive);
    set("lofi", d.lofi); set("bass", d.bass); set("treble", d.treble);
    set("swellAttack", d.swellAtk); set("swellAmount", d.swellAmt);
    set("slideAmount", d.slideAmt);
    if (auto* param = apvts.getParameter("guitarMode"))
        param->setValueNotifyingHost(param->convertTo0to1((float)d.guitarMode));
    if (auto* param = apvts.getParameter("tremSync"))
        param->setValueNotifyingHost(d.tremSync ? 1.f : 0.f);
    if (auto* param = apvts.getParameter("tremDiv"))
        param->setValueNotifyingHost(param->convertTo0to1((float)d.tremDiv));
    if (auto* param = apvts.getParameter("arpPattern"))
        param->setValueNotifyingHost(param->convertTo0to1((float)d.arpPattern));
}

const juce::String GhostSurfProcessor::getProgramName(int index)
{
    if (index >= 0 && index < 5) return PRESETS[index].name;
    return {};
}

void GhostSurfProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto xml = apvts.copyState().createXml();
    copyXmlToBinary(*xml, destData);
}

void GhostSurfProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* GhostSurfProcessor::createEditor()
{
    return new GhostSurfEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GhostSurfProcessor();
}
