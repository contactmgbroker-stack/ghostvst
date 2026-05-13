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
// Oscilloscope avec cadre en forme de vague
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
// Séquenceur d'arpège interactif : 8 steps cliquables/draggables
class ArpEditor : public juce::Component, private juce::Timer
{
public:
    explicit ArpEditor(GhostSurfProcessor& p);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;

private:
    void timerCallback() override { repaint(); }
    GhostSurfProcessor& proc;
    int   getStepAt(float x) const;
    float getValForY(float y) const;
    int   dragStep    = -1;
    float dragStartY  = 0.f;
    float dragStartVal= 0.f;
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
    juce::ComboBox   presetBox;
    juce::ToggleButton tremSyncBtn;
    juce::ComboBox   tremDivBox;
    juce::ComboBox   arpPatternBox;   // charge un pattern dans les 8 steps
    juce::TextButton modeNormal, modeSwell, modeArpege;
    juce::Label      titleLabel;

    VUMeter        vuMeter;
    WaveformDisplay waveDisplay;
    ArpEditor       arpEditor;    // séquenceur interactif (panel bas)

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   tremSyncAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> tremDivAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> arpPatternAttach;

    int  currentMode = 0;
    bool arpBoxLive  = false;

    void setGuitarMode(int mode);
    void buildKnob(KnobWidget& kw, const char* paramID, const char* label, juce::Colour accent);
    void placeKnob(KnobWidget& kw, int cx, int cy, int size);
    // rank: 1=rouge, 2=orange, 3=jaune, 0=off
    void updateLiveHighlights(int presetIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GhostSurfEditor)
};
