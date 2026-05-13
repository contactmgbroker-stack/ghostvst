#include "PluginEditor.h"

using namespace juce;

//=============================================================================
// Color palette
namespace C {
    const Colour bg         { 0xFF0E0600 };
    const Colour panelDark  { 0xFF180900 };
    const Colour panelMid   { 0xFF221200 };
    const Colour border     { 0xFF5A3010 };
    const Colour gold       { 0xFFE8C060 };
    const Colour goldDim    { 0xFF9A7830 };
    const Colour cream      { 0xFFF0E0B0 };
    const Colour creamDark  { 0xFFB89860 };
    const Colour chrome     { 0xFFCCCCCC };
    const Colour chromeDark { 0xFF888888 };
    const Colour labelText  { 0xFFC8A060 };
    const Colour amber      { 0xFFFF9020 };
    const Colour vuGreen    { 0xFF40DD20 };
    const Colour vuAmber    { 0xFFFFAA20 };
    const Colour vuRed      { 0xFFFF3030 };
}

//=============================================================================
// RetroLookAndFeel
//=============================================================================
RetroLookAndFeel::RetroLookAndFeel()
{
    setColour(ComboBox::backgroundColourId,    C::panelDark);
    setColour(ComboBox::textColourId,          C::gold);
    setColour(ComboBox::arrowColourId,         C::gold);
    setColour(ComboBox::outlineColourId,       C::border);
    setColour(PopupMenu::backgroundColourId,   C::panelMid);
    setColour(PopupMenu::textColourId,         C::cream);
    setColour(PopupMenu::highlightedBackgroundColourId, C::border);
    setColour(Label::textColourId,             C::labelText);
}

void RetroLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int w, int h,
                                         float sliderPos, float startAngle, float endAngle,
                                         Slider& /*slider*/)
{
    auto bounds = Rectangle<float>((float)x, (float)y, (float)w, (float)h);
    auto centre = bounds.getCentre();
    float r = jmin(bounds.getWidth(), bounds.getHeight()) * 0.44f;

    // Drop shadow
    g.setColour(Colour(0x60000000));
    g.fillEllipse(centre.x - r + 1.5f, centre.y - r + 2.5f, r * 2.f, r * 2.f);

    // Chrome outer ring
    ColourGradient rimGrad(C::chrome, centre.x - r * 0.7f, centre.y - r * 0.7f,
                           C::chromeDark, centre.x + r * 0.7f, centre.y + r * 0.7f, true);
    g.setGradientFill(rimGrad);
    g.fillEllipse(centre.x - r, centre.y - r, r * 2.f, r * 2.f);

    // Cream knob body
    float ir = r * 0.84f;
    ColourGradient bodyGrad(Colour(0xFFEED9A0), centre.x - ir * 0.35f, centre.y - ir * 0.35f,
                             Colour(0xFFB89050), centre.x + ir * 0.35f, centre.y + ir * 0.35f, true);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(centre.x - ir, centre.y - ir, ir * 2.f, ir * 2.f);

    // Knurling lines (vintage look)
    g.setColour(Colour(0x18000000));
    for (int k = 0; k < 16; ++k) {
        float a = (float)k / 16.f * MathConstants<float>::twoPi;
        float x0 = centre.x + std::cos(a) * ir * 0.58f;
        float y0 = centre.y + std::sin(a) * ir * 0.58f;
        float x1 = centre.x + std::cos(a) * ir * 0.96f;
        float y1 = centre.y + std::sin(a) * ir * 0.96f;
        g.drawLine(x0, y0, x1, y1, 0.6f);
    }

    // Indicator line (dark, crisp)
    float angle = startAngle + sliderPos * (endAngle - startAngle);
    auto lineA = centre.getPointOnCircumference(ir * 0.18f, angle);
    auto lineB = centre.getPointOnCircumference(ir * 0.86f, angle);
    g.setColour(Colour(0xFF1A0800));
    g.drawLine(lineA.x, lineA.y, lineB.x, lineB.y, 2.5f);

    // Center dot
    g.setColour(Colour(0xFF301800));
    g.fillEllipse(centre.x - 2.8f, centre.y - 2.8f, 5.6f, 5.6f);

    // Tick marks
    float tickR = r + 3.5f;
    g.setColour(C::goldDim.withAlpha(0.7f));
    for (int k = 0; k <= 10; ++k) {
        float t = (float)k / 10.f;
        float a = startAngle + t * (endAngle - startAngle);
        float len = (k % 5 == 0) ? 4.5f : 2.5f;
        float cx = centre.x, cy = centre.y;
        float ca = std::cos(a), sa = std::sin(a);
        g.drawLine(cx + ca * tickR, cy + sa * tickR,
                   cx + ca * (tickR + len), cy + sa * (tickR + len), 1.0f);
    }

    // Active arc
    Path arc;
    arc.addArc(centre.x - r - 5.f, centre.y - r - 5.f,
               (r + 5.f) * 2.f, (r + 5.f) * 2.f,
               startAngle, angle, true);
    g.setColour(C::amber.withAlpha(0.6f));
    g.strokePath(arc, PathStrokeType(1.8f));
}

void RetroLookAndFeel::drawComboBox(Graphics& g, int w, int h, bool /*isDown*/,
                                     int bx, int by, int bw, int bh,
                                     ComboBox& box)
{
    g.setColour(C::panelDark);
    g.fillRoundedRectangle(0.f, 0.f, (float)w, (float)h, 4.f);
    g.setColour(C::border);
    g.drawRoundedRectangle(0.5f, 0.5f, w - 1.f, h - 1.f, 4.f, 1.f);

    // Arrow
    auto arrow = Rectangle<int>(bx, by, bw, bh).toFloat();
    Path arrowPath;
    arrowPath.startNewSubPath(arrow.getX() + 4, arrow.getCentreY() - 2);
    arrowPath.lineTo(arrow.getCentreX(), arrow.getCentreY() + 3);
    arrowPath.lineTo(arrow.getRight() - 4, arrow.getCentreY() - 2);
    g.setColour(C::gold);
    g.strokePath(arrowPath, PathStrokeType(1.5f));
}

void RetroLookAndFeel::drawPopupMenuBackground(Graphics& g, int w, int h)
{
    g.setColour(C::panelMid);
    g.fillRoundedRectangle(0.f, 0.f, (float)w, (float)h, 4.f);
    g.setColour(C::border);
    g.drawRoundedRectangle(0.5f, 0.5f, w - 1.f, h - 1.f, 4.f, 1.f);
}

void RetroLookAndFeel::drawLabel(Graphics& g, Label& label)
{
    g.setColour(C::labelText);
    g.setFont(getLabelFont(label));
    g.drawText(label.getText(), label.getLocalBounds(), Justification::centred, false);
}

//=============================================================================
// VU Meter
//=============================================================================
VUMeter::VUMeter(GhostSurfProcessor& p) : proc(p) { startTimerHz(30); }

void VUMeter::timerCallback()
{
    float target = proc.getOutputLevel();
    displayLevel += (target - displayLevel) * 0.25f;
    repaint();
}

void VUMeter::paint(Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // Background
    g.setColour(Colour(0xFF060300));
    g.fillRoundedRectangle(b, 3.f);
    g.setColour(C::border);
    g.drawRoundedRectangle(b.reduced(0.5f), 3.f, 1.f);

    const int SEGS = 24;
    const float segH = (b.getHeight() - 6.f) / SEGS;
    const float segW = b.getWidth() - 6.f;

    float dbVal = Decibels::gainToDecibels(displayLevel, -60.f);
    float norm  = jmap(dbVal, -48.f, 0.f, 0.f, 1.f);
    int   litN  = jlimit(0, SEGS, (int)(norm * SEGS));

    for (int i = 0; i < SEGS; ++i) {
        float segY = b.getBottom() - 3.f - (i + 1) * segH;
        auto  seg  = Rectangle<float>(b.getX() + 3.f, segY, segW, segH - 1.f);

        if (i < litN) {
            if      (i >= SEGS - 3) g.setColour(C::vuRed);
            else if (i >= SEGS - 7) g.setColour(C::vuAmber);
            else                    g.setColour(C::vuGreen);
        } else {
            g.setColour(Colour(0xFF0D0600));
        }
        g.fillRect(seg);

        // Dark separator
        g.setColour(Colour(0x50000000));
        g.drawRect(seg, 0.4f);
    }

    // Label
    g.setColour(C::goldDim);
    g.setFont(Font("Arial", 8.f, Font::bold));
    g.drawText("OUT", b.removeFromBottom(14.f), Justification::centred, false);
}

//=============================================================================
// Helper — draw a section panel with title
//=============================================================================
void GhostSurfEditor::paintSection(Graphics& g, Rectangle<int> r, const char* title)
{
    auto rf = r.toFloat();
    g.setColour(C::panelDark);
    g.fillRoundedRectangle(rf, 5.f);
    g.setColour(C::border);
    g.drawRoundedRectangle(rf.reduced(0.5f), 5.f, 1.f);

    g.setColour(C::gold);
    g.setFont(Font("Arial", 9.f, Font::bold));
    g.drawText(title, r.withHeight(20), Justification::centredTop, false);
}

//=============================================================================
// KnobWidget setup
//=============================================================================
void GhostSurfEditor::buildKnob(KnobWidget& kw, const char* paramID, const char* labelText)
{
    kw.slider.setSliderStyle(Slider::RotaryVerticalDrag);
    kw.slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    kw.slider.setLookAndFeel(&retroLF);
    kw.slider.setPopupDisplayEnabled(true, false, this); // tooltip value on hover
    addAndMakeVisible(kw.slider);

    kw.label.setText(labelText, dontSendNotification);
    kw.label.setJustificationType(Justification::centred);
    kw.label.setFont(Font("Arial", 9.f, Font::bold));
    kw.label.setColour(Label::textColourId, C::labelText);
    addAndMakeVisible(kw.label);

    kw.attach = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        proc.getAPVTS(), paramID, kw.slider);
}

// Place a knob centred at (cx, knobY)
void GhostSurfEditor::placeKnob(KnobWidget& kw, int cx, int knobY, int kSize)
{
    kw.slider.setBounds(cx - kSize / 2, knobY, kSize, kSize);
    kw.label.setBounds(cx - 30, knobY + kSize + 1, 60, 14);
}

//=============================================================================
// Editor
//=============================================================================
GhostSurfEditor::GhostSurfEditor(GhostSurfProcessor& p)
    : AudioProcessorEditor(&p), proc(p), vuMeter(p)
{
    setSize(700, 420);
    setLookAndFeel(&retroLF);

    // Title
    titleLabel.setText("GHOST SURF", dontSendNotification);
    titleLabel.setFont(Font("Arial", 26.f, Font::bold));
    titleLabel.setColour(Label::textColourId, C::gold);
    titleLabel.setJustificationType(Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    // Subtitle
    subtitleLabel.setText("VINTAGE SURF & GARAGE FX", dontSendNotification);
    subtitleLabel.setFont(Font("Arial", 9.f, Font::italic));
    subtitleLabel.setColour(Label::textColourId, C::goldDim);
    subtitleLabel.setJustificationType(Justification::centredLeft);
    addAndMakeVisible(subtitleLabel);

    // Preset combo
    for (int i = 0; i < 5; ++i)
        presetBox.addItem(p.getProgramName(i), i + 1);
    presetBox.setSelectedId(p.getCurrentProgram() + 1, dontSendNotification);
    presetBox.onChange = [&] {
        proc.setCurrentProgram(presetBox.getSelectedId() - 1);
    };
    addAndMakeVisible(presetBox);

    // Build knobs
    buildKnob(reverbMix,   "reverbMix",   "MIX");
    buildKnob(reverbDecay, "reverbDecay", "DECAY");
    buildKnob(reverbTone,  "reverbTone",  "TONE");
    buildKnob(tremSpeed,   "tremSpeed",   "SPEED");
    buildKnob(tremDepth,   "tremDepth",   "DEPTH");
    buildKnob(drive,       "drive",       "DRIVE");
    buildKnob(lofi,        "lofi",        "LOFI");
    buildKnob(bass,        "bass",        "BASS");
    buildKnob(treble,      "treble",      "TREBLE");

    addAndMakeVisible(vuMeter);
}

GhostSurfEditor::~GhostSurfEditor()
{
    setLookAndFeel(nullptr);
}

//=============================================================================
void GhostSurfEditor::paint(Graphics& g)
{
    // Dark background with subtle grain texture
    ColourGradient bgGrad(Colour(0xFF120800), 0.f, 0.f,
                           Colour(0xFF080300), (float)getWidth(), (float)getHeight(), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    // Scan-line texture (retro CRT feel)
    g.setColour(Colour(0x06C8A060));
    for (int y = 0; y < getHeight(); y += 3)
        g.drawHorizontalLine(y, 0.f, (float)getWidth());

    // Header bar
    g.setColour(C::panelDark);
    g.fillRect(0, 0, getWidth(), 58);
    g.setColour(C::border);
    g.drawHorizontalLine(58, 0.f, (float)getWidth());
    g.setColour(C::gold.withAlpha(0.15f));
    g.drawHorizontalLine(59, 0.f, (float)getWidth());

    // Section panels
    paintSection(g, {8,   62, 220, 340}, "~~ SPRING REVERB ~~");
    paintSection(g, {234, 62, 150, 340}, "~~ TREMOLO ~~");
    paintSection(g, {390, 62, 90,  340}, "~~ DRIVE ~~");
    paintSection(g, {486, 62, 90,  340}, "~~ LO-FI ~~");
    paintSection(g, {582, 62, 110, 155}, "~~ EQ ~~");

    // Output / VU panel
    g.setColour(C::panelDark);
    g.fillRoundedRectangle(582.f, 223.f, 110.f, 179.f, 5.f);
    g.setColour(C::border);
    g.drawRoundedRectangle(582.5f, 223.5f, 109.f, 178.f, 5.f, 1.f);
    g.setColour(C::gold);
    g.setFont(Font("Arial", 9.f, Font::bold));
    g.drawText("~~ LEVEL ~~", Rectangle<int>{582, 223, 110, 20}, Justification::centredTop, false);
}

void GhostSurfEditor::resized()
{
    // Header
    titleLabel.setBounds   (14, 8,  280, 30);
    subtitleLabel.setBounds(14, 38, 280, 14);
    presetBox.setBounds    (310, 14, 220, 28);

    // Knob size and base Y
    const int KS  = 54;    // knob diameter
    const int KY  = 95;    // top of knob area

    // SPRING REVERB — 3 knobs at x centres: 48, 118, 188
    placeKnob(reverbMix,   48,  KY, KS);
    placeKnob(reverbDecay, 118, KY, KS);
    placeKnob(reverbTone,  188, KY, KS);

    // TREMOLO — 2 knobs at x: 272, 342
    placeKnob(tremSpeed, 272, KY, KS);
    placeKnob(tremDepth, 342, KY, KS);

    // DRIVE — 1 knob at x: 435
    placeKnob(drive, 435, KY, KS);

    // LOFI — 1 knob at x: 531
    placeKnob(lofi, 531, KY, KS);

    // EQ — 2 knobs at x: 614, 669
    placeKnob(bass,   614, KY,        KS);
    placeKnob(treble, 669, KY,        KS);

    // VU meter
    vuMeter.setBounds(598, 240, 78, 155);
}
