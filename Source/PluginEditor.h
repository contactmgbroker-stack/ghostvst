#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Vintage cream & chrome rotary knob look and feel
class RetroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RetroLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;

    void drawComboBox(juce::Graphics&, int w, int h, bool isDown,
                      int bx, int by, int bw, int bh,
                      juce::ComboBox&) override;

    void drawPopupMenuBackground(juce::Graphics&, int w, int h) override;

    juce::Font getComboBoxFont(juce::ComboBox&) override
    { return juce::Font("Arial", 13.0f, juce::Font::plain); }

    juce::Font getLabelFont(juce::Label&) override
    { return juce::Font("Arial", 10.0f, juce::Font::bold); }

    void drawLabel(juce::Graphics&, juce::Label&) override;
};

//==============================================================================
// LED-segment VU meter
class VUMeter : public juce::Component, private juce::Timer
{
public:
    explicit VUMeter(GhostSurfProcessor& p);
    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;
    GhostSurfProcessor& proc;
    float displayLevel = 0.0f;
};

//==============================================================================
// One knob + label + APVTS attachment
struct KnobWidget
{
    juce::Slider slider;
    juce::Label  label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
};

//==============================================================================
class GhostSurfEditor : public juce::AudioProcessorEditor
{
public:
    explicit GhostSurfEditor(GhostSurfProcessor&);
    ~GhostSurfEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GhostSurfProcessor& proc;
    RetroLookAndFeel retroLF;

    KnobWidget reverbMix, reverbDecay, reverbTone;
    KnobWidget tremSpeed, tremDepth;
    KnobWidget drive;
    KnobWidget lofi;
    KnobWidget bass, treble;

    juce::ComboBox presetBox;
    juce::Label    titleLabel;
    juce::Label    subtitleLabel;

    VUMeter vuMeter;

    void buildKnob(KnobWidget& kw, const char* paramID, const char* labelText);
    void placeKnob(KnobWidget& kw, int cx, int knobY, int kSize);

    static void paintSection(juce::Graphics& g, juce::Rectangle<int> r, const char* title);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GhostSurfEditor)
};
