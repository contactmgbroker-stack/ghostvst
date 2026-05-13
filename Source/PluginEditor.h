#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class OceanLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OceanLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;
    void drawComboBox(juce::Graphics&, int w, int h, bool isDown,
                      int bx, int by, int bw, int bh, juce::ComboBox&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour& bg, bool highlighted, bool down) override;
    juce::Font getLabelFont(juce::Label&) override;
    void drawLabel(juce::Graphics&, juce::Label&) override;
};

//==============================================================================
// Waveform oscilloscope display
class WaveformDisplay : public juce::Component, private juce::Timer
{
public:
    explicit WaveformDisplay(GhostSurfProcessor& p);
    void paint(juce::Graphics&) override;
private:
    void timerCallback() override { repaint(); }
    GhostSurfProcessor& proc;
};

//==============================================================================
// Arpeggiator step display (8 lit boxes)
class ArpStepDisplay : public juce::Component, private juce::Timer
{
public:
    explicit ArpStepDisplay(GhostSurfProcessor& p);
    void paint(juce::Graphics&) override;
    void setPattern(int patternIndex);
private:
    void timerCallback() override { repaint(); }
    GhostSurfProcessor& proc;
    int pattern = 0;
};

//==============================================================================
class VUMeter : public juce::Component, private juce::Timer
{
public:
    explicit VUMeter(GhostSurfProcessor& p);
    void paint(juce::Graphics&) override;
private:
    void timerCallback() override;
    GhostSurfProcessor& proc;
    float displayLevel = 0.f;
};

//==============================================================================
struct KnobWidget {
    juce::Slider slider;
    juce::Label  label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
};

//==============================================================================
class GhostSurfEditor : public juce::AudioProcessorEditor,
                        private juce::Timer
{
public:
    explicit GhostSurfEditor(GhostSurfProcessor&);
    ~GhostSurfEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    GhostSurfProcessor& proc;
    OceanLookAndFeel lf;

    // Knobs
    KnobWidget reverbMix, reverbDecay, reverbTone;
    KnobWidget tremSpeed, tremDepth;
    KnobWidget drive, lofi, bass, treble;
    KnobWidget swellAttack, swellAmount;
    KnobWidget slideAmount, slideSpeed;

    // Controls
    juce::ComboBox  presetBox;
    juce::ToggleButton tremSyncBtn;
    juce::ComboBox  tremDivBox;
    juce::ComboBox  arpPatternBox;
    juce::TextButton modeNormal, modeSwell, modeArpege;
    juce::Label titleLabel;

    VUMeter        vuMeter;
    WaveformDisplay waveDisplay;
    ArpStepDisplay  arpDisplay;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>     tremSyncAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>   tremDivAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>   presetAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>   arpPatternAttach;

    int currentMode = 0;
    void setGuitarMode(int mode);
    void buildKnob(KnobWidget& kw, const char* paramID, const char* label, juce::Colour accent);
    void placeKnob(KnobWidget& kw, int cx, int cy, int size);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GhostSurfEditor)
};
