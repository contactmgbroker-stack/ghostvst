#include "PluginEditor.h"
#include <BinaryData.h>

using namespace juce;

// ── Couleurs ──────────────────────────────────────────────────────────────────
namespace C {
    const Colour bg       { 0xFF0A0A0F };
    const Colour panel    { 0xCC0D0D18 };
    const Colour border   { 0x55FFFFFF };
    const Colour cyan     { 0xFF00CCFF };
    const Colour amber    { 0xFFFF8820 };
    const Colour purple   { 0xFFBB44FF };
    const Colour green    { 0xFF44FF88 };
    const Colour white    { 0xFFEEEEEE };
    const Colour dimWhite { 0xFF888899 };
    const Colour btnActive{ 0xFF00CCFF };
    const Colour btnOff   { 0xFF1A1A2E };
}

// ── ModernLookAndFeel ─────────────────────────────────────────────────────────
ModernLookAndFeel::ModernLookAndFeel()
{
    setColour(ComboBox::backgroundColourId,  C::panel);
    setColour(ComboBox::textColourId,        C::white);
    setColour(ComboBox::arrowColourId,       C::cyan);
    setColour(ComboBox::outlineColourId,     C::border);
    setColour(PopupMenu::backgroundColourId, Colour(0xFF0D0D18));
    setColour(PopupMenu::textColourId,       C::white);
    setColour(PopupMenu::highlightedBackgroundColourId, C::cyan.withAlpha(0.3f));
    setColour(Label::textColourId,           C::dimWhite);
    setColour(ToggleButton::textColourId,    C::white);
    setColour(ToggleButton::tickColourId,    C::cyan);
}

void ModernLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int w, int h,
                                          float sliderPos, float startAngle, float endAngle,
                                          Slider& slider)
{
    auto bounds = Rectangle<float>((float)x, (float)y, (float)w, (float)h);
    auto centre = bounds.getCentre();
    float r = jmin(bounds.getWidth(), bounds.getHeight()) * 0.42f;

    // Accent color from tag
    Colour accent = C::cyan;
    auto tag = slider.getComponentID();
    if (tag == "amber")  accent = C::amber;
    if (tag == "purple") accent = C::purple;
    if (tag == "green")  accent = C::green;

    // Outer ring (dark)
    g.setColour(Colour(0xFF1A1A2E));
    g.fillEllipse(centre.x - r, centre.y - r, r*2, r*2);

    // Arc background
    Path arcBg;
    arcBg.addArc(centre.x - r + 3, centre.y - r + 3, (r-3)*2, (r-3)*2,
                 startAngle, endAngle, true);
    g.setColour(Colour(0xFF252540));
    g.strokePath(arcBg, PathStrokeType(3.f));

    // Arc value
    float angle = startAngle + sliderPos * (endAngle - startAngle);
    Path arcVal;
    arcVal.addArc(centre.x - r + 3, centre.y - r + 3, (r-3)*2, (r-3)*2,
                  startAngle, angle, true);
    g.setColour(accent);
    g.strokePath(arcVal, PathStrokeType(3.f, PathStrokeType::curved, PathStrokeType::rounded));

    // Glow on arc end
    g.setColour(accent.withAlpha(0.5f));
    auto arcEnd = centre.getPointOnCircumference(r - 3, angle);
    g.fillEllipse(arcEnd.x - 4, arcEnd.y - 4, 8, 8);

    // Inner knob
    float ir = r * 0.72f;
    ColourGradient kg(Colour(0xFF252540), centre.x, centre.y - ir,
                      Colour(0xFF0D0D18), centre.x, centre.y + ir, false);
    g.setGradientFill(kg);
    g.fillEllipse(centre.x - ir, centre.y - ir, ir*2, ir*2);

    // Indicator dot
    auto dot = centre.getPointOnCircumference(ir * 0.72f, angle);
    g.setColour(accent);
    g.fillEllipse(dot.x - 3, dot.y - 3, 6, 6);

    // Value text
    g.setColour(C::white.withAlpha(0.9f));
    g.setFont(Font("Arial", 8.f, Font::plain));
    String valStr;
    double val = slider.getValue();
    if (val == (int)val) valStr = String((int)val);
    else valStr = String(val, 1);
    g.drawText(valStr, (int)(centre.x - 20), (int)(centre.y + ir + 2), 40, 12, Justification::centred);
}

void ModernLookAndFeel::drawComboBox(Graphics& g, int w, int h, bool /*isDown*/,
                                      int bx, int by, int bw, int bh, ComboBox&)
{
    g.setColour(C::panel);
    g.fillRoundedRectangle(0.f, 0.f, (float)w, (float)h, 4.f);
    g.setColour(C::border);
    g.drawRoundedRectangle(0.5f, 0.5f, w-1.f, h-1.f, 4.f, 1.f);

    Path arrow;
    auto a = Rectangle<int>(bx, by, bw, bh).toFloat();
    arrow.startNewSubPath(a.getX()+4, a.getCentreY()-2);
    arrow.lineTo(a.getCentreX(), a.getCentreY()+3);
    arrow.lineTo(a.getRight()-4, a.getCentreY()-2);
    g.setColour(C::cyan);
    g.strokePath(arrow, PathStrokeType(1.5f));
}

void ModernLookAndFeel::drawButtonBackground(Graphics& g, Button& btn,
                                              const Colour& /*bg*/, bool highlighted, bool down)
{
    bool active = btn.getToggleState();
    Colour fill = active ? C::cyan.withAlpha(0.25f) : C::btnOff;
    Colour bord = active ? C::cyan : C::border;

    g.setColour(fill);
    g.fillRoundedRectangle(btn.getLocalBounds().toFloat(), 6.f);
    g.setColour(highlighted ? bord.brighter(0.3f) : bord);
    g.drawRoundedRectangle(btn.getLocalBounds().toFloat().reduced(0.5f), 6.f, 1.5f);
}

Font ModernLookAndFeel::getLabelFont(Label&) { return Font("Arial", 10.f, Font::bold); }

void ModernLookAndFeel::drawLabel(Graphics& g, Label& label)
{
    g.setColour(label.findColour(Label::textColourId));
    g.setFont(getLabelFont(label));
    g.drawText(label.getText(), label.getLocalBounds(), Justification::centred, false);
}

// ── VU Meter ──────────────────────────────────────────────────────────────────
VUMeter::VUMeter(GhostSurfProcessor& p) : proc(p) { startTimerHz(30); }

void VUMeter::timerCallback() { displayLevel += (proc.getOutputLevel() - displayLevel) * 0.25f; repaint(); }

void VUMeter::paint(Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(Colour(0xFF060610));
    g.fillRoundedRectangle(b, 4.f);
    g.setColour(C::border);
    g.drawRoundedRectangle(b.reduced(0.5f), 4.f, 1.f);

    const int SEGS = 20;
    float segH = (b.getHeight() - 8.f) / SEGS;
    float dbVal = Decibels::gainToDecibels(displayLevel, -60.f);
    int litN = jlimit(0, SEGS, (int)(jmap(dbVal, -48.f, 0.f, 0.f, 1.f) * SEGS));

    for (int i = 0; i < SEGS; ++i) {
        float segY = b.getBottom() - 4.f - (i+1)*segH;
        auto seg = Rectangle<float>(b.getX()+4, segY, b.getWidth()-8, segH-1.f);
        if (i < litN) {
            if      (i >= SEGS-2) g.setColour(Colour(0xFFFF3030));
            else if (i >= SEGS-5) g.setColour(Colour(0xFFFFAA20));
            else                  g.setColour(C::green);
        } else {
            g.setColour(Colour(0xFF111120));
        }
        g.fillRoundedRectangle(seg, 1.f);
    }
    g.setColour(C::dimWhite);
    g.setFont(8.f);
    g.drawText("dB", b.removeFromBottom(12.f), Justification::centred);
}

// ── Editor ────────────────────────────────────────────────────────────────────
void GhostSurfEditor::buildKnob(KnobWidget& kw, const char* paramID,
                                  const char* labelText, Colour accent)
{
    kw.slider.setSliderStyle(Slider::RotaryVerticalDrag);
    kw.slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    kw.slider.setLookAndFeel(&lf);
    // Store accent color as component ID tag
    if      (accent == C::amber)  kw.slider.setComponentID("amber");
    else if (accent == C::purple) kw.slider.setComponentID("purple");
    else if (accent == C::green)  kw.slider.setComponentID("green");
    else                          kw.slider.setComponentID("cyan");
    addAndMakeVisible(kw.slider);

    kw.label.setText(labelText, dontSendNotification);
    kw.label.setFont(Font("Arial", 9.f, Font::bold));
    kw.label.setColour(Label::textColourId, accent);
    kw.label.setJustificationType(Justification::centred);
    addAndMakeVisible(kw.label);

    kw.attach = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        proc.getAPVTS(), paramID, kw.slider);
}

void GhostSurfEditor::placeKnob(KnobWidget& kw, int cx, int cy, int size)
{
    kw.slider.setBounds(cx - size/2, cy - size/2, size, size);
    kw.label.setBounds(cx - 28, cy + size/2 + 2, 56, 13);
}

GhostSurfEditor::GhostSurfEditor(GhostSurfProcessor& p)
    : AudioProcessorEditor(&p), proc(p), vuMeter(p)
{
    setSize(740, 500);
    setLookAndFeel(&lf);

    // Load background photo from binary data
    bgPhoto = ImageCache::getFromMemory(BinaryData::cigare_png, BinaryData::cigare_pngSize);

    // Title
    titleLabel.setText("GHOST SURF", dontSendNotification);
    titleLabel.setFont(Font("Arial", 22.f, Font::bold));
    titleLabel.setColour(Label::textColourId, C::cyan);
    addAndMakeVisible(titleLabel);

    // Preset
    for (int i = 0; i < 5; ++i) presetBox.addItem(p.getProgramName(i), i+1);
    presetBox.setSelectedId(p.getCurrentProgram()+1, dontSendNotification);
    presetBox.onChange = [&] { proc.setCurrentProgram(presetBox.getSelectedId()-1); };
    addAndMakeVisible(presetBox);

    // Mode buttons
    auto setupModeBtn = [&](TextButton& btn, const String& text) {
        btn.setButtonText(text);
        btn.setClickingTogglesState(true);
        btn.setRadioGroupId(1);
        btn.setLookAndFeel(&lf);
        addAndMakeVisible(btn);
    };
    setupModeBtn(modeNormal,  "NORMAL");
    setupModeBtn(modeSwell,   "AUTO-SWELL");
    setupModeBtn(modeArpege,  "ARPEGE");
    modeNormal.setToggleState(true, dontSendNotification);

    modeNormal.onClick  = [&] { setGuitarMode(0); };
    modeSwell.onClick   = [&] { setGuitarMode(1); };
    modeArpege.onClick  = [&] { setGuitarMode(2); };

    // Tremolo sync
    tremSyncBtn.setButtonText("SYNC BPM");
    tremSyncBtn.setLookAndFeel(&lf);
    addAndMakeVisible(tremSyncBtn);
    tremSyncAttach = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(
        proc.getAPVTS(), "tremSync", tremSyncBtn);

    tremDivBox.addItem("1/4", 1); tremDivBox.addItem("1/8", 2); tremDivBox.addItem("1/16", 3);
    addAndMakeVisible(tremDivBox);
    tremDivAttach = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        proc.getAPVTS(), "tremDiv", tremDivBox);

    // Build knobs — REVERB (cyan)
    buildKnob(reverbMix,   "reverbMix",   "MIX",    C::cyan);
    buildKnob(reverbDecay, "reverbDecay", "DECAY",  C::cyan);
    buildKnob(reverbTone,  "reverbTone",  "TONE",   C::cyan);

    // TREMOLO (amber)
    buildKnob(tremSpeed, "tremSpeed", "VITESSE",  C::amber);
    buildKnob(tremDepth, "tremDepth", "INTENSITE",C::amber);

    // EFFETS (purple)
    buildKnob(drive, "drive", "SATURATION", C::purple);
    buildKnob(lofi,  "lofi",  "LO-FI",     C::purple);
    buildKnob(bass,  "bass",  "BASSES",    C::purple);
    buildKnob(treble,"treble","AIGUS",     C::purple);

    // GUITAR (green)
    buildKnob(swellAttack, "swellAttack", "ATTAQUE",  C::green);
    buildKnob(swellAmount, "swellAmount", "INTENSITE",C::green);
    buildKnob(slideAmount, "slideAmount", "GLISS",    C::green);
    buildKnob(slideSpeed,  "slideSpeed",  "VITESSE",  C::green);

    addAndMakeVisible(vuMeter);
    startTimerHz(10);
}

GhostSurfEditor::~GhostSurfEditor() { setLookAndFeel(nullptr); }

void GhostSurfEditor::timerCallback()
{
    // Sync mode buttons to processor state
    int mode = (int)proc.getAPVTS().getRawParameterValue("guitarMode")->load();
    if (mode != currentMode) {
        currentMode = mode;
        modeNormal.setToggleState(mode==0, dontSendNotification);
        modeSwell.setToggleState (mode==1, dontSendNotification);
        modeArpege.setToggleState(mode==2, dontSendNotification);
        resized();
    }
}

void GhostSurfEditor::setGuitarMode(int mode)
{
    currentMode = mode;
    if (auto* p = proc.getAPVTS().getParameter("guitarMode"))
        p->setValueNotifyingHost(p->convertTo0to1((float)mode));
    resized();
}

// ── Paint ─────────────────────────────────────────────────────────────────────
void GhostSurfEditor::paint(Graphics& g)
{
    // Background photo
    if (bgPhoto.isValid()) {
        g.drawImageWithin(bgPhoto, 0, 0, getWidth(), getHeight(),
                          RectanglePlacement::fillDestination);
        // Dark overlay for readability
        g.setColour(Colour(0xD5050510));
        g.fillAll();
    } else {
        g.setColour(C::bg);
        g.fillAll();
    }

    // Scanlines
    g.setColour(Colour(0x0A00CCFF));
    for (int y = 0; y < getHeight(); y += 4)
        g.drawHorizontalLine(y, 0, (float)getWidth());

    // Header bar
    g.setColour(Colour(0xBB080815));
    g.fillRect(0, 0, getWidth(), 60);
    g.setColour(C::cyan.withAlpha(0.3f));
    g.drawHorizontalLine(60, 0, (float)getWidth());

    auto drawPanel = [&](Rectangle<int> r, const char* title, Colour accent) {
        g.setColour(Colour(0xBB0A0A18));
        g.fillRoundedRectangle(r.toFloat(), 8.f);
        g.setColour(accent.withAlpha(0.4f));
        g.drawRoundedRectangle(r.toFloat().reduced(0.5f), 8.f, 1.f);
        // Top accent line
        g.setColour(accent.withAlpha(0.7f));
        g.fillRoundedRectangle((float)r.getX()+10, (float)r.getY(), (float)r.getWidth()-20, 2.f, 1.f);
        // Title
        g.setColour(accent);
        g.setFont(Font("Arial", 9.f, Font::bold));
        g.drawText(title, r.withHeight(22), Justification::centredTop, false);
    };

    drawPanel({8,   64, 215, 420}, "SPRING REVERB",    C::cyan);
    drawPanel({230, 64, 160, 420}, "TREMOLO",          C::amber);
    drawPanel({397, 64, 230, 420}, "EFFETS",           C::purple);
    drawPanel({634, 64, 98,  200}, "GUITARE",          C::green);
    drawPanel({634, 270, 98, 214}, "NIVEAU",           C::dimWhite);
}

// ── Resized ───────────────────────────────────────────────────────────────────
void GhostSurfEditor::resized()
{
    titleLabel.setBounds(10, 12, 220, 36);
    presetBox.setBounds(240, 16, 200, 28);

    // Mode buttons
    modeNormal.setBounds(450, 16, 85, 28);
    modeSwell.setBounds(540, 16, 85, 28);
    modeArpege.setBounds(630, 16, 85, 28);

    const int KS = 52;
    const int KY = 130;

    // REVERB section (x=8, w=215) — knob centres: 55, 115, 175
    placeKnob(reverbMix,   55,  KY, KS);
    placeKnob(reverbDecay, 115, KY, KS);
    placeKnob(reverbTone,  175, KY, KS);

    // TREMOLO section (x=230, w=160)
    placeKnob(tremSpeed, 280, KY, KS);
    placeKnob(tremDepth, 350, KY, KS);

    tremSyncBtn.setBounds(238, 200, 80, 22);
    tremDivBox.setBounds(325, 200, 56, 22);

    // EFFETS section (x=397, w=230) — drive, lofi, bass, treble
    placeKnob(drive,  445, KY, KS);
    placeKnob(lofi,   505, KY, KS);
    placeKnob(bass,   565, KY, KS);
    placeKnob(treble, 620, KY, KS);

    // GUITARE section (x=634, w=98) — shows different knobs by mode
    swellAttack.slider.setVisible(currentMode == 1);
    swellAttack.label.setVisible(currentMode == 1);
    swellAmount.slider.setVisible(currentMode == 1);
    swellAmount.label.setVisible(currentMode == 1);
    slideAmount.slider.setVisible(currentMode != 1);
    slideAmount.label.setVisible(currentMode != 1);
    slideSpeed.slider.setVisible(currentMode != 1);
    slideSpeed.label.setVisible(currentMode != 1);

    placeKnob(swellAttack, 683, 105, KS);
    placeKnob(swellAmount, 683, 185, KS);
    placeKnob(slideAmount, 683, 105, KS);
    placeKnob(slideSpeed,  683, 185, KS);

    // VU meter
    vuMeter.setBounds(642, 278, 82, 196);
}
