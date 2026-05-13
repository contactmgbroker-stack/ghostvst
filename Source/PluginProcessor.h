#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <array>
#include <vector>
#include <cmath>

//==============================================================================
struct CombFilter {
    std::vector<float> buf;
    int pos = 0;
    float lpState = 0.0f;
    void init(int size) { buf.assign(size, 0.0f); pos = 0; lpState = 0.0f; }
    void reset() { std::fill(buf.begin(), buf.end(), 0.0f); lpState = 0.0f; }
    inline float tick(float x, float fb, float damp) noexcept {
        float out = buf[pos];
        lpState = out * (1.0f - damp) + lpState * damp;
        buf[pos] = x + lpState * fb;
        if (++pos >= (int)buf.size()) pos = 0;
        return out;
    }
};

struct AllpassFilter {
    std::vector<float> buf;
    int pos = 0;
    static constexpr float g = 0.5f;
    void init(int size) { buf.assign(size, 0.0f); pos = 0; }
    void reset() { std::fill(buf.begin(), buf.end(), 0.0f); }
    inline float tick(float x) noexcept {
        float w = buf[pos];
        float y = w - g * x;
        buf[pos] = x + g * y;
        if (++pos >= (int)buf.size()) pos = 0;
        return y;
    }
};

//==============================================================================
class GhostSurfProcessor : public juce::AudioProcessor
{
public:
    static constexpr int NUM_PRESETS = 12;
    static constexpr int SCOPE_SIZE  = 1024;

    GhostSurfProcessor();
    ~GhostSurfProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override {
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) return false;
        return true;
    }

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "GhostSurf"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 5.0; }

    int getNumPrograms() override { return NUM_PRESETS; }
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    float getOutputLevel() const { return outputLevel.load(); }

    // Scope buffer (lock-free ring buffer for waveform display)
    const float* getScopePtr()      const { return scopeData; }
    int          getScopeWritePos() const { return scopeWritePos.load(std::memory_order_relaxed); }

    // Current arpeggiator step (for step display)
    int getCurrentArpStep() const { return currentArpStep.load(std::memory_order_relaxed); }

private:
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    int currentPreset = 0;

    // Spring Reverb
    static constexpr int NUM_COMBS = 4;
    static constexpr int NUM_AP    = 2;
    static const int BASE_COMB_LEN[NUM_COMBS];
    static const int BASE_AP_LEN[NUM_AP];
    CombFilter    combL[NUM_COMBS], combR[NUM_COMBS];
    AllpassFilter apL[NUM_AP],      apR[NUM_AP];
    static float reverbFeedback(float decaySec, int delaySamples, double sr);

    // Tremolo
    float tremoloPhase = 0.0f;
    float currentBPM   = 120.0f;

    // Auto-Swell
    float swellEnvL = 0.0f, swellEnvR = 0.0f;

    // Bottleneck / Slide
    float slidePhase    = 0.0f;
    float slideTarget   = 0.0f;
    float slideCurrent  = 0.0f;
    std::vector<float> slideDelayL, slideDelayR;
    int   slideWritePos = 0;

    // Rhythmic Gate (arpège)
    float gatePhase    = 0.0f;
    float gateSmoothed = 1.0f;
    std::atomic<int> currentArpStep { 0 };

    // LoFi
    juce::Random rng;
    float lofiLpL = 0.0f, lofiLpR = 0.0f;
    float noiseGateEnv = 0.0f;   // envelope follower to gate noise

    // DC blocker
    float dcXL = 0.0f, dcYL = 0.0f;
    float dcXR = 0.0f, dcYR = 0.0f;

    // EQ
    using Coeffs  = juce::dsp::IIR::Coefficients<float>;
    using IIRFilt = juce::dsp::IIR::Filter<float>;
    IIRFilt bassL, bassR, trebleL, trebleR;

    double sr = 44100.0;
    std::atomic<float> outputLevel { 0.0f };

    // Scope ring buffer
    float scopeData[SCOPE_SIZE] {};
    std::atomic<int> scopeWritePos { 0 };

    void updateEQ();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GhostSurfProcessor)
};
