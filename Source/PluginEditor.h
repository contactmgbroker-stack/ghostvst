#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class OceanLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OceanLookAndFeel();
    void drawRotarySlider(juce::Graphics&,int x,int y,int w,int h,
                          float pos,float start,float end,juce::Slider&) override;
    void drawComboBox(juce::Graphics&,int w,int h,bool,int bx,int by,int bw,int bh,juce::ComboBox&) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool hi,bool) override;
    // Hide default tick/checkbox for LED buttons
    void drawTickBox(juce::Graphics&,juce::Component&,float x,float y,float w,float h,
                     bool,bool,bool,bool) override;
    juce::Font getLabelFont(juce::Label&) override;
    void drawLabel(juce::Graphics&,juce::Label&) override;
};

//==============================================================================
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
class SpecterPad : public juce::Component, private juce::Timer
{
public:
    explicit SpecterPad(GhostSurfProcessor& p);
    ~SpecterPad() override = default;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseUp  (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    GhostSurfProcessor& proc;

    // filterX/Y = committed audio filter position (click/drag only)
    float filterX=0.5f, filterY=0.3f;
    // curX/Y = visual cursor preview (hover)
    float curX=0.5f, curY=0.3f;
    bool  dragging=false;

    struct Particle { float x,y,vx,vy,life,maxLife,size; juce::Colour col; };
    std::vector<Particle> particles;
    void spawnParticles(float x,float y,juce::Colour c,int n=4);
    void drawFilterCurve(juce::Graphics& g,juce::Rectangle<float> area,int shape);
};

//==============================================================================
class FreezePanel : public juce::Component, private juce::Timer
{
public:
    explicit FreezePanel(GhostSurfProcessor& p);
    void paint(juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
private:
    void timerCallback() override;
    GhostSurfProcessor& proc;
    float pulse=0.f, glowAnim=0.f;
    bool  wasActive=false;
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
    float displayLevel=0.f;
};

//==============================================================================
struct KnobWidget {
    juce::Slider slider;
    juce::Label  label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
};

//==============================================================================
class GhostSurfEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit GhostSurfEditor(GhostSurfProcessor&);
    ~GhostSurfEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& k) override;

private:
    void timerCallback() override;
    GhostSurfProcessor& proc;
    OceanLookAndFeel lf;

    // ── Knobs ────────────────────────────────────────────────────────────────
    KnobWidget reverbMix, reverbDecay, reverbTone;
    KnobWidget tremSpeed, tremDepth;
    KnobWidget flangerRate, flangerDepth, flangerFeedback;
    KnobWidget drive, lofi, bass, treble;
    KnobWidget wahDepth, wahRate;
    KnobWidget autoPanRate;
    KnobWidget slideAmount, slideSpeed;
    KnobWidget vibeSpeed, vibeDepth;
    KnobWidget freezeGrain, freezeShimmer, freezeDecay;

    // ── Controls ─────────────────────────────────────────────────────────────
    juce::ComboBox     presetBox;
    juce::ToggleButton tremSyncBtn, vibeModeBtn;
    juce::ComboBox     tremDivBox;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   tremSyncAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   vibeModeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> tremDivAttach;

    // ── LED bypass buttons ───────────────────────────────────────────────────
    juce::ToggleButton ledReverb, ledTremolo, ledFlanger, ledWah,
                       ledSlide, ledVibe, ledSpecter, ledAutoPan;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        ledReverbA, ledTremoloA, ledFlangerA, ledWahA,
        ledSlideA, ledVibeA, ledSpecterA, ledAutoPanA;

    // ── Subcomponents ────────────────────────────────────────────────────────
    VUMeter         vuMeter;
    WaveformDisplay waveDisplay;
    SpecterPad      specterPad;
    FreezePanel     freezePanel;

    // ── Surf Score animation ─────────────────────────────────────────────────
    float scoreAnim=0.f;
    int   comboFlash=0;

    // ── Live highlight colors (random per preset) ────────────────────────────
    juce::Colour liveColours[3];

    // ── Helpers ──────────────────────────────────────────────────────────────
    void updateLiveHighlights(int presetIndex);
    void buildKnob(KnobWidget&,const char* id,const char* label,juce::Colour accent);
    void placeKnob(KnobWidget&,int cx,int cy,int size);
    juce::Slider* findRankedSlider(int rank);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GhostSurfEditor)
};
