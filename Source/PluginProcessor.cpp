#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Spring reverb delay tunings (samples @ 44100 Hz)
// Short comb delays = metallic/spring character
const int GhostSurfProcessor::BASE_COMB_LEN[NUM_COMBS] = { 1557, 1617, 1491, 1422 };
const int GhostSurfProcessor::BASE_AP_LEN[NUM_AP]      = { 556, 441 };

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout GhostSurfProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    // Spring Reverb
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "reverbMix",   "Reverb Mix",   0.0f, 1.0f, 0.35f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "reverbDecay", "Reverb Decay",
        juce::NormalisableRange<float>(0.3f, 6.0f, 0.01f), 2.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "reverbTone",  "Reverb Tone",
        juce::NormalisableRange<float>(500.0f, 8000.0f, 1.0f, 0.5f), 3000.0f));

    // Tremolo
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "tremSpeed", "Tremolo Speed",
        juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f, 0.5f), 4.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "tremDepth", "Tremolo Depth", 0.0f, 1.0f, 0.0f));

    // Drive (tube saturation)
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive", "Drive", 0.0f, 1.0f, 0.2f));

    // LoFi
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lofi", "LoFi", 0.0f, 1.0f, 0.1f));

    // EQ
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "bass",   "Bass",   -12.0f, 12.0f, 0.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "treble", "Treble", -12.0f, 12.0f, 0.0f));

    return { p.begin(), p.end() };
}

GhostSurfProcessor::GhostSurfProcessor()
    : AudioProcessor(BusesProperties()
          .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameters())
{
}

//==============================================================================
float GhostSurfProcessor::reverbFeedback(float decaySec, int delaySamples, double sampleRate)
{
    // T60 formula: fb = 10^(-3 * D / T60)
    float D = (float)delaySamples / (float)sampleRate;
    float fb = std::pow(10.0f, -3.0f * D / decaySec);
    return juce::jlimit(0.0f, 0.975f, fb);
}

void GhostSurfProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    sr = sampleRate;
    const double scale = sampleRate / 44100.0;

    // Scale reverb delay lines to current sample rate
    for (int i = 0; i < NUM_COMBS; ++i) {
        int lenL = (int)(BASE_COMB_LEN[i] * scale);
        int lenR = lenL + 23; // Stereo spread
        combL[i].init(lenL);
        combR[i].init(lenR);
    }
    for (int i = 0; i < NUM_AP; ++i) {
        int len = (int)(BASE_AP_LEN[i] * scale);
        apL[i].init(len);
        apR[i].init(len);
    }

    bassL.reset();   bassR.reset();
    trebleL.reset(); trebleR.reset();

    tremoloPhase = 0.0f;
    lofiLpL = lofiLpR = 0.0f;
    outputLevel.store(0.0f);

    updateEQ();
}

void GhostSurfProcessor::updateEQ()
{
    float bassDB   = apvts.getRawParameterValue("bass")->load();
    float trebleDB = apvts.getRawParameterValue("treble")->load();

    float bassGain   = juce::Decibels::decibelsToGain(bassDB);
    float trebleGain = juce::Decibels::decibelsToGain(trebleDB);

    auto bCoeffs = Coeffs::makeLowShelf  (sr, 200.0f,  0.707f, bassGain);
    auto tCoeffs = Coeffs::makeHighShelf (sr, 4000.0f, 0.707f, trebleGain);

    *bassL.coefficients   = *bCoeffs;
    *bassR.coefficients   = *bCoeffs;
    *trebleL.coefficients = *tCoeffs;
    *trebleR.coefficients = *tCoeffs;
}

//==============================================================================
void GhostSurfProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numCh      = buffer.getNumChannels();
    auto* bufL = buffer.getWritePointer(0);
    auto* bufR = (numCh > 1) ? buffer.getWritePointer(1) : bufL;

    // Load parameters (atomic reads — thread-safe)
    const float reverbMix   = apvts.getRawParameterValue("reverbMix")->load();
    const float reverbDecay = apvts.getRawParameterValue("reverbDecay")->load();
    const float reverbTone  = apvts.getRawParameterValue("reverbTone")->load();
    const float tremSpeed   = apvts.getRawParameterValue("tremSpeed")->load();
    const float tremDepth   = apvts.getRawParameterValue("tremDepth")->load();
    const float drive       = apvts.getRawParameterValue("drive")->load();
    const float lofi        = apvts.getRawParameterValue("lofi")->load();

    // Reverb — compute feedback and damping once per block
    const double scale  = sr / 44100.0;
    const int    refLen = (int)(BASE_COMB_LEN[0] * scale);
    const float  fb     = reverbFeedback(reverbDecay, refLen, sr);

    // damp: lower tone → more damping (higher LP coeff)
    // LP coeff a = exp(-2π * f / sr) → higher a = darker
    const float damp = std::exp(-2.0f * juce::MathConstants<float>::pi * reverbTone / (float)sr);
    // Invert so damp=0 means bright, damp=0.99 means dark
    const float combDamp = juce::jlimit(0.0f, 0.99f, 1.0f - damp);

    // Drive parameters
    const float driveGain = 1.0f + drive * 9.0f;
    const float driveNorm = std::tanh(driveGain) + 1e-6f;

    // LoFi parameters
    const float noiseAmt = lofi * 0.012f;
    // Tape bandwidth LP: higher lofi = darker (lower cutoff)
    const float tapeCut  = 14000.0f - lofi * 8000.0f;
    const float tapeLpA  = std::exp(-2.0f * juce::MathConstants<float>::pi * tapeCut / (float)sr);

    // Tremolo increment
    const float tremInc = 2.0f * juce::MathConstants<float>::pi * tremSpeed / (float)sr;

    // Update EQ (cheap — just coefficient calculation)
    updateEQ();

    float peakLevel = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = bufL[i];
        float inR = bufR[i];

        // ── TUBE SATURATION ─────────────────────────────────────────────────
        if (drive > 0.001f) {
            inL = std::tanh(inL * driveGain) / driveNorm;
            inR = std::tanh(inR * driveGain) / driveNorm;
        }

        // ── LOFI / TAPE ──────────────────────────────────────────────────────
        if (lofi > 0.001f) {
            // Analog noise
            inL += (rng.nextFloat() * 2.0f - 1.0f) * noiseAmt;
            inR += (rng.nextFloat() * 2.0f - 1.0f) * noiseAmt;
            // Tape bandwidth rolloff
            lofiLpL = lofiLpL * tapeLpA + inL * (1.0f - tapeLpA);
            lofiLpR = lofiLpR * tapeLpA + inR * (1.0f - tapeLpA);
            inL = lofiLpL;
            inR = lofiLpR;
        }

        // ── SPRING REVERB ────────────────────────────────────────────────────
        if (reverbMix > 0.001f) {
            // Allpass diffusion (input smearing — reduces metallic attack)
            float diffL = inL, diffR = inR;
            for (int j = 0; j < NUM_AP; ++j) {
                diffL = apL[j].tick(diffL);
                diffR = apR[j].tick(diffR);
            }

            // 4 parallel comb filters (spring resonances)
            float wetL = 0.0f, wetR = 0.0f;
            for (int j = 0; j < NUM_COMBS; ++j) {
                wetL += combL[j].tick(diffL, fb, combDamp);
                wetR += combR[j].tick(diffR, fb, combDamp);
            }
            wetL *= 0.25f;
            wetR *= 0.25f;

            bufL[i] = inL * (1.0f - reverbMix) + wetL * reverbMix;
            bufR[i] = inR * (1.0f - reverbMix) + wetR * reverbMix;
        } else {
            bufL[i] = inL;
            bufR[i] = inR;
        }

        // ── EQ ───────────────────────────────────────────────────────────────
        bufL[i] = trebleL.processSample(bassL.processSample(bufL[i]));
        bufR[i] = trebleR.processSample(bassR.processSample(bufR[i]));

        // ── TREMOLO ──────────────────────────────────────────────────────────
        if (tremDepth > 0.001f) {
            // Amplitude modulation: (1 - depth/2) to 1.0, sine wave
            float mod = 1.0f - tremDepth * 0.5f * (1.0f + std::sin(tremoloPhase));
            bufL[i] *= mod;
            bufR[i] *= mod;
            tremoloPhase += tremInc;
            if (tremoloPhase >= juce::MathConstants<float>::twoPi)
                tremoloPhase -= juce::MathConstants<float>::twoPi;
        }

        // Peak detection for VU
        float lvl = std::max(std::abs(bufL[i]), std::abs(bufR[i]));
        if (lvl > peakLevel) peakLevel = lvl;
    }

    // VU meter: instant attack, slow release (~400ms)
    float cur = outputLevel.load();
    if (peakLevel > cur) {
        outputLevel.store(peakLevel);
    } else {
        float releaseAlpha = std::exp(-1.0f / (0.4f * (float)sr / (float)numSamples));
        outputLevel.store(cur * releaseAlpha);
    }
}

//==============================================================================
// Presets
struct PresetData {
    const char* name;
    float reverbMix, reverbDecay, reverbTone;
    float tremSpeed, tremDepth;
    float drive, lofi;
    float bass, treble;
};

static const PresetData PRESETS[5] = {
    // name              mix   decay  tone    spd   dpt    drv   lfi    bs    tr
    { "Surf Clean",    0.40f, 3.0f, 4200.f, 4.0f, 0.30f, 0.10f, 0.05f,  1.f,  2.f },
    { "Dirty Cramps",  0.55f, 2.0f, 1800.f, 7.0f, 0.65f, 0.75f, 0.45f, -1.f,  4.f },
    { "Night Waves",   0.60f, 5.0f, 3500.f, 2.0f, 0.40f, 0.20f, 0.12f,  2.f, -1.f },
    { "Haunted Motel", 0.72f, 5.5f, 1400.f, 3.2f, 0.72f, 0.55f, 0.52f,  3.f, -2.f },
    { "Zen Surf",      0.30f, 4.0f, 5000.f, 1.0f, 0.18f, 0.00f, 0.05f,  0.f,  1.f },
};

void GhostSurfProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= 5) return;
    currentPreset = index;
    const auto& d = PRESETS[index];

    auto set = [&](const char* id, float val) {
        if (auto* p = apvts.getParameter(id))
            p->setValueNotifyingHost(p->convertTo0to1(val));
    };

    set("reverbMix",   d.reverbMix);
    set("reverbDecay", d.reverbDecay);
    set("reverbTone",  d.reverbTone);
    set("tremSpeed",   d.tremSpeed);
    set("tremDepth",   d.tremDepth);
    set("drive",       d.drive);
    set("lofi",        d.lofi);
    set("bass",        d.bass);
    set("treble",      d.treble);
}

const juce::String GhostSurfProcessor::getProgramName(int index)
{
    if (index >= 0 && index < 5) return PRESETS[index].name;
    return {};
}

//==============================================================================
void GhostSurfProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    auto xml = state.createXml();
    copyXmlToBinary(*xml, destData);
}

void GhostSurfProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
juce::AudioProcessorEditor* GhostSurfProcessor::createEditor()
{
    return new GhostSurfEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GhostSurfProcessor();
}
