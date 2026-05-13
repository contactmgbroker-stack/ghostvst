#include "PluginEditor.h"

using namespace juce;

// ── Couleurs Océan ────────────────────────────────────────────────────────────
namespace C {
    // Backgrounds
    const Colour bg        { 0xFF02060E };   // Deep ocean night
    const Colour bgMid     { 0xFF030C1A };   // Mid ocean
    const Colour panel     { 0xCC080F1E };   // Dark glass panel
    const Colour panelTop  { 0xCC0E1A2E };   // Lighter top of panel (hemisphere)

    // Section accents
    const Colour cyan      { 0xFF00CCFF };   // Electric ocean (Reverb)
    const Colour teal      { 0xFF00E5CC };   // Aquamarine (Tremolo)
    const Colour cobalt    { 0xFF5599FF };   // Cobalt blue (Effets)
    const Colour seafoam   { 0xFF00DD88 };   // Seafoam green (Guitare)

    // UI elements
    const Colour white     { 0xFFDDF0FF };   // Ice white (blue tint)
    const Colour dimWhite  { 0xFF4A6888 };   // Dim ocean
    const Colour border    { 0x4400AACC };   // Subtle blue border
    const Colour btnActive { 0xFF00CCFF };
    const Colour btnOff    { 0xFF04101E };

    // Compat aliases (for buildKnob comparisons)
    const Colour amber  = teal;
    const Colour purple = cobalt;
    const Colour green  = seafoam;
}

// ── Arp pattern data (mirrors processor) ────────────────────────────────────
static const float ARP_DISPLAY[8][8] = {
    { 1.f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, 0.f },
    { 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f },
    { 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f },
    { 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f, 1.f },
    { 1.f, 1.f, 0.5f,1.f, 1.f, 0.5f,0.f, 0.f },
    { 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f },
    { 1.f, 0.5f,0.f, 1.f, 0.5f,0.f, 1.f, 0.f },
    { 1.f, 0.f, 0.5f,0.f, 1.f, 0.f, 0.5f,0.f },
};

// ── OceanLookAndFeel ──────────────────────────────────────────────────────────
OceanLookAndFeel::OceanLookAndFeel()
{
    setColour(ComboBox::backgroundColourId,  C::panel);
    setColour(ComboBox::textColourId,        C::white);
    setColour(ComboBox::arrowColourId,       C::cyan);
    setColour(ComboBox::outlineColourId,     C::border);
    setColour(PopupMenu::backgroundColourId, Colour(0xFF080F1E));
    setColour(PopupMenu::textColourId,       C::white);
    setColour(PopupMenu::highlightedBackgroundColourId, C::cyan.withAlpha(0.25f));
    setColour(Label::textColourId,           C::dimWhite);
    setColour(ToggleButton::textColourId,    C::white);
    setColour(ToggleButton::tickColourId,    C::cyan);
}

void OceanLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int w, int h,
                                         float sliderPos, float startAngle, float endAngle,
                                         Slider& slider)
{
    auto bounds = Rectangle<float>((float)x, (float)y, (float)w, (float)h);
    auto centre = bounds.getCentre();
    float r = jmin(bounds.getWidth(), bounds.getHeight()) * 0.42f;

    // Accent color from tag
    Colour accent = C::cyan;
    auto tag = slider.getComponentID();
    if (tag == "teal")    accent = C::teal;
    if (tag == "cobalt")  accent = C::cobalt;
    if (tag == "seafoam") accent = C::seafoam;

    // Outer ring — dark navy with subtle glow
    g.setColour(Colour(0xFF04101E));
    g.fillEllipse(centre.x - r, centre.y - r, r*2, r*2);

    // Outer ring border glow
    g.setColour(accent.withAlpha(0.2f));
    g.drawEllipse(centre.x - r, centre.y - r, r*2, r*2, 1.5f);

    // Arc background
    Path arcBg;
    arcBg.addArc(centre.x - r + 4, centre.y - r + 4, (r-4)*2, (r-4)*2,
                 startAngle, endAngle, true);
    g.setColour(Colour(0xFF0A1828));
    g.strokePath(arcBg, PathStrokeType(3.5f));

    // Arc value (filled with gradient-like color)
    float angle = startAngle + sliderPos * (endAngle - startAngle);
    Path arcVal;
    arcVal.addArc(centre.x - r + 4, centre.y - r + 4, (r-4)*2, (r-4)*2,
                  startAngle, angle, true);
    g.setColour(accent);
    g.strokePath(arcVal, PathStrokeType(3.5f, PathStrokeType::curved, PathStrokeType::rounded));

    // Glow dot at arc end
    g.setColour(accent.withAlpha(0.6f));
    auto arcEnd = centre.getPointOnCircumference(r - 4, angle);
    g.fillEllipse(arcEnd.x - 5, arcEnd.y - 5, 10, 10);
    g.setColour(accent.brighter(0.4f));
    g.fillEllipse(arcEnd.x - 2.5f, arcEnd.y - 2.5f, 5, 5);

    // Inner knob — hemisphere gradient (bright top, dark bottom = 3D dome)
    float ir = r * 0.70f;
    ColourGradient kg(
        Colour(0xFF1A2E44), centre.x, centre.y - ir,   // lighter top
        Colour(0xFF060E1C), centre.x, centre.y + ir,   // darker bottom
        false
    );
    g.setGradientFill(kg);
    g.fillEllipse(centre.x - ir, centre.y - ir, ir*2, ir*2);

    // Rim highlight (hemisphere top edge reflection)
    g.setColour(accent.withAlpha(0.15f));
    g.drawEllipse(centre.x - ir, centre.y - ir, ir*2, ir*2, 1.f);

    // Indicator dot (bright, on knob face)
    auto dot = centre.getPointOnCircumference(ir * 0.65f, angle);
    g.setColour(accent.brighter(0.2f));
    g.fillEllipse(dot.x - 3, dot.y - 3, 6, 6);

    // Value text
    g.setColour(C::white.withAlpha(0.85f));
    g.setFont(Font("Arial", 8.f, Font::plain));
    String valStr;
    double val = slider.getValue();
    if (val == (int)val) valStr = String((int)val);
    else valStr = String(val, 1);
    g.drawText(valStr, (int)(centre.x - 20), (int)(centre.y + ir + 3), 40, 12,
               Justification::centred);
}

void OceanLookAndFeel::drawComboBox(Graphics& g, int w, int h, bool /*isDown*/,
                                     int bx, int by, int bw, int bh, ComboBox&)
{
    // Glass background
    ColourGradient bg(Colour(0xFF0C1828), 0, 0, Colour(0xFF060E1C), 0, h, false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(0.f, 0.f, (float)w, (float)h, 5.f);
    g.setColour(C::cyan.withAlpha(0.4f));
    g.drawRoundedRectangle(0.5f, 0.5f, w-1.f, h-1.f, 5.f, 1.f);

    Path arrow;
    auto a = Rectangle<int>(bx, by, bw, bh).toFloat();
    arrow.startNewSubPath(a.getX()+4, a.getCentreY()-2);
    arrow.lineTo(a.getCentreX(), a.getCentreY()+3);
    arrow.lineTo(a.getRight()-4, a.getCentreY()-2);
    g.setColour(C::cyan);
    g.strokePath(arrow, PathStrokeType(1.5f));
}

void OceanLookAndFeel::drawButtonBackground(Graphics& g, Button& btn,
                                             const Colour& /*bg*/, bool highlighted, bool /*down*/)
{
    bool active = btn.getToggleState();
    auto b = btn.getLocalBounds().toFloat();

    if (active) {
        // Active: ocean glow
        ColourGradient grad(C::cyan.withAlpha(0.30f), b.getX(), b.getY(),
                            C::cyan.withAlpha(0.10f), b.getX(), b.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(b, 7.f);
        g.setColour(C::cyan.withAlpha(0.8f));
        g.drawRoundedRectangle(b.reduced(0.5f), 7.f, 1.5f);
    } else {
        ColourGradient grad(Colour(0xFF0C1828), b.getX(), b.getY(),
                            Colour(0xFF060E1C), b.getX(), b.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(b, 7.f);
        g.setColour(highlighted ? C::cyan.withAlpha(0.3f) : C::border);
        g.drawRoundedRectangle(b.reduced(0.5f), 7.f, 1.f);
    }
}

Font OceanLookAndFeel::getLabelFont(Label&) { return Font("Arial", 10.f, Font::bold); }

void OceanLookAndFeel::drawLabel(Graphics& g, Label& label)
{
    g.setColour(label.findColour(Label::textColourId));
    g.setFont(getLabelFont(label));
    g.drawText(label.getText(), label.getLocalBounds(), Justification::centred, false);
}

// ── WaveformDisplay ───────────────────────────────────────────────────────────
WaveformDisplay::WaveformDisplay(GhostSurfProcessor& p) : proc(p)
{
    startTimerHz(30);
}

void WaveformDisplay::paint(Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float cornerR = 6.f;

    // Background gradient (deep ocean)
    ColourGradient bg(Colour(0xFF030A14), b.getX(), b.getY(),
                      Colour(0xFF050C18), b.getX(), b.getBottom(), false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(b, cornerR);

    // Grid lines
    g.setColour(Colour(0x1200AACC));
    for (int gi = 1; gi < 4; ++gi) {
        float gy = b.getY() + b.getHeight() * gi / 4.f;
        g.drawHorizontalLine((int)gy, b.getX() + 4, b.getRight() - 4);
    }
    for (int gi = 1; gi < 6; ++gi) {
        float gx = b.getX() + b.getWidth() * gi / 6.f;
        g.drawVerticalLine((int)gx, b.getY() + 4, b.getBottom() - 4);
    }

    // Center line (ocean horizon)
    g.setColour(Colour(0x3500CCFF));
    float cy = b.getCentreY();
    g.drawHorizontalLine((int)cy, b.getX() + 4, b.getRight() - 4);

    // Draw waveform
    const float* data = proc.getScopePtr();
    int wp = proc.getScopeWritePos();
    const int N = GhostSurfProcessor::SCOPE_SIZE;
    const int W = (int)b.getWidth() - 4;

    Path wavePath;
    bool started = false;
    for (int xi = 0; xi < W; ++xi) {
        int idx = (wp + (int)(xi * N / W)) % N;
        float sample = data[idx];
        float px = b.getX() + 2 + xi;
        float py = cy - jlimit(-1.f, 1.f, sample) * (b.getHeight() * 0.42f);
        if (!started) { wavePath.startNewSubPath(px, py); started = true; }
        else wavePath.lineTo(px, py);
    }

    // Glow layer (thick, transparent)
    g.setColour(Colour(0xFF00CCFF).withAlpha(0.20f));
    g.strokePath(wavePath, PathStrokeType(4.0f, PathStrokeType::curved));

    // Mid layer
    g.setColour(Colour(0xFF00CCFF).withAlpha(0.50f));
    g.strokePath(wavePath, PathStrokeType(2.0f, PathStrokeType::curved));

    // Sharp bright line on top
    g.setColour(Colour(0xFF00EEFF).withAlpha(0.90f));
    g.strokePath(wavePath, PathStrokeType(1.0f, PathStrokeType::curved));

    // Border
    g.setColour(Colour(0xFF00CCFF).withAlpha(0.35f));
    g.drawRoundedRectangle(b.reduced(0.5f), cornerR, 1.f);
}

// ── ArpStepDisplay ────────────────────────────────────────────────────────────
ArpStepDisplay::ArpStepDisplay(GhostSurfProcessor& p) : proc(p)
{
    startTimerHz(30);
}

void ArpStepDisplay::setPattern(int p) { pattern = jlimit(0, 7, p); }

void ArpStepDisplay::paint(Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    int currentStep = proc.getCurrentArpStep();

    // Background
    g.setColour(Colour(0xFF030A14));
    g.fillRoundedRectangle(b, 5.f);
    g.setColour(C::cyan.withAlpha(0.3f));
    g.drawRoundedRectangle(b.reduced(0.5f), 5.f, 1.f);

    const int STEPS = 8;
    float cellW = (b.getWidth() - 6.f) / STEPS;
    float cellH = b.getHeight() - 6.f;

    for (int s = 0; s < STEPS; ++s) {
        float val = ARP_DISPLAY[jlimit(0,7,pattern)][s];  // 0, 0.5, or 1
        bool isActive = (s == currentStep);
        float cellX = b.getX() + 3.f + s * cellW;
        float cellY = b.getY() + 3.f;

        auto cell = Rectangle<float>(cellX, cellY, cellW - 2.f, cellH);

        if (val > 0.01f) {
            // Lit step
            float brightness = val; // 0.5 or 1.0
            Colour stepColor = isActive ? C::cyan.brighter(0.3f) : C::cyan.withAlpha(brightness);
            ColourGradient grad(stepColor.withAlpha(isActive ? 0.9f : 0.6f * brightness),
                                cell.getX(), cell.getY(),
                                stepColor.withAlpha(isActive ? 0.5f : 0.3f * brightness),
                                cell.getX(), cell.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(cell, 3.f);

            if (isActive) {
                g.setColour(C::cyan.brighter(0.5f));
                g.drawRoundedRectangle(cell.reduced(0.5f), 3.f, 1.5f);
            }
        } else {
            // Off step
            g.setColour(isActive ? Colour(0xFF0A1828) : Colour(0xFF060E1C));
            g.fillRoundedRectangle(cell, 3.f);
            g.setColour(Colour(0x2200AACC));
            g.drawRoundedRectangle(cell.reduced(0.5f), 3.f, 1.f);
        }
    }
}

// ── VU Meter ──────────────────────────────────────────────────────────────────
VUMeter::VUMeter(GhostSurfProcessor& p) : proc(p) { startTimerHz(30); }

void VUMeter::timerCallback()
{
    displayLevel += (proc.getOutputLevel() - displayLevel) * 0.25f;
    repaint();
}

void VUMeter::paint(Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // Background
    ColourGradient bg(Colour(0xFF030A14), b.getX(), b.getY(),
                      Colour(0xFF050C18), b.getX(), b.getBottom(), false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(b, 4.f);
    g.setColour(C::cyan.withAlpha(0.3f));
    g.drawRoundedRectangle(b.reduced(0.5f), 4.f, 1.f);

    const int SEGS = 20;
    float segH = (b.getHeight() - 20.f) / SEGS;
    float dbVal = Decibels::gainToDecibels(displayLevel, -60.f);
    int litN = jlimit(0, SEGS, (int)(jmap(dbVal, -48.f, 0.f, 0.f, 1.f) * SEGS));

    for (int i = 0; i < SEGS; ++i) {
        float segY = b.getBottom() - 14.f - (i+1)*segH;
        auto seg = Rectangle<float>(b.getX()+5, segY, b.getWidth()-10, segH-1.5f);
        if (i < litN) {
            if      (i >= SEGS-2) g.setColour(Colour(0xFFFF2244));  // Red peak
            else if (i >= SEGS-5) g.setColour(Colour(0xFFFFAA00));  // Amber
            else                  g.setColour(C::seafoam);          // Ocean green
        } else {
            g.setColour(Colour(0xFF080F1E));
        }
        g.fillRoundedRectangle(seg, 1.5f);
    }

    // dB label
    g.setColour(C::dimWhite);
    g.setFont(8.f);
    g.drawText("dB", b.removeFromBottom(14.f), Justification::centred);
}

// ── Editor ────────────────────────────────────────────────────────────────────
void GhostSurfEditor::buildKnob(KnobWidget& kw, const char* paramID,
                                  const char* labelText, Colour accent)
{
    kw.slider.setSliderStyle(Slider::RotaryVerticalDrag);
    kw.slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    kw.slider.setLookAndFeel(&lf);
    // Store accent color as component ID tag
    if      (accent == C::teal)    kw.slider.setComponentID("teal");
    else if (accent == C::cobalt)  kw.slider.setComponentID("cobalt");
    else if (accent == C::seafoam) kw.slider.setComponentID("seafoam");
    else                           kw.slider.setComponentID("cyan");
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
    : AudioProcessorEditor(&p), proc(p), vuMeter(p), waveDisplay(p), arpDisplay(p)
{
    setSize(760, 510);
    setLookAndFeel(&lf);

    // Title
    titleLabel.setText("GHOST SURF", dontSendNotification);
    titleLabel.setFont(Font("Arial", 22.f, Font::bold));
    titleLabel.setColour(Label::textColourId, C::cyan);
    addAndMakeVisible(titleLabel);

    // Preset — 10 presets
    for (int i = 0; i < GhostSurfProcessor::NUM_PRESETS; ++i)
        presetBox.addItem(p.getProgramName(i), i+1);
    presetBox.setSelectedId(p.getCurrentProgram()+1, dontSendNotification);
    presetBox.onChange = [&] { proc.setCurrentProgram(presetBox.getSelectedId()-1); };
    addAndMakeVisible(presetBox);

    // Mode buttons
    auto setupModeBtn = [&](TextButton& btn, const String& text) {
        btn.setButtonText(text);
        btn.setClickingTogglesState(true);
        btn.setRadioGroupId(1);
        btn.setLookAndFeel(&lf);
        btn.setColour(TextButton::textColourOffId, C::white);
        btn.setColour(TextButton::textColourOnId,  C::cyan);
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

    // Arpeggiator — 8 patterns
    arpPatternBox.addItem("1/8 Gate", 1);
    arpPatternBox.addItem("Triolets", 2);
    arpPatternBox.addItem("A Forest", 3);
    arpPatternBox.addItem("Syncope",  4);
    arpPatternBox.addItem("Gallop",   5);
    arpPatternBox.addItem("Off-Beat", 6);
    arpPatternBox.addItem("Surf Beat",7);
    arpPatternBox.addItem("Waltz",    8);
    addAndMakeVisible(arpPatternBox);
    arpPatternAttach = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        proc.getAPVTS(), "arpPattern", arpPatternBox);
    arpPatternBox.onChange = [&] {
        arpDisplay.setPattern(arpPatternBox.getSelectedId() - 1);
    };

    // Build knobs — REVERB (cyan)
    buildKnob(reverbMix,   "reverbMix",   "MIX",    C::cyan);
    buildKnob(reverbDecay, "reverbDecay", "DECAY",  C::cyan);
    buildKnob(reverbTone,  "reverbTone",  "TONE",   C::cyan);

    // TREMOLO (teal/aquamarine)
    buildKnob(tremSpeed, "tremSpeed", "VITESSE",  C::teal);
    buildKnob(tremDepth, "tremDepth", "INTENSITE",C::teal);

    // EFFETS (cobalt)
    buildKnob(drive, "drive", "DRIVE",  C::cobalt);
    buildKnob(lofi,  "lofi",  "LO-FI", C::cobalt);
    buildKnob(bass,  "bass",  "BASSES",C::cobalt);
    buildKnob(treble,"treble","AIGUS", C::cobalt);

    // GUITAR (seafoam)
    buildKnob(swellAttack, "swellAttack", "ATTAQUE",C::seafoam);
    buildKnob(swellAmount, "swellAmount", "AMOUNT", C::seafoam);
    buildKnob(slideAmount, "slideAmount", "GLISS",  C::seafoam);
    buildKnob(slideSpeed,  "slideSpeed",  "VITESSE",C::seafoam);

    addAndMakeVisible(vuMeter);
    addAndMakeVisible(waveDisplay);
    addAndMakeVisible(arpDisplay);

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
    if (auto* param = proc.getAPVTS().getParameter("guitarMode"))
        param->setValueNotifyingHost(param->convertTo0to1((float)mode));
    resized();
}

// ── Paint ─────────────────────────────────────────────────────────────────────
void GhostSurfEditor::paint(Graphics& g)
{
    const int W = getWidth(), H = getHeight();

    // ── Deep ocean background ────────────────────────────────────────────────
    ColourGradient bgGrad(Colour(0xFF020609), 0, 0,
                          Colour(0xFF040C1A), 0, H, false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    // Subtle underwater caustic shapes (decorative wave bands)
    for (int wi = 0; wi < 4; ++wi) {
        Path wave;
        float waveY = H * (0.15f + wi * 0.22f);
        float amp   = 18.f + wi * 4.f;
        wave.startNewSubPath(0, waveY);
        for (int x = 0; x <= W; x += 3) {
            float y = waveY + std::sin(x * 0.018f + wi * 1.1f) * amp
                            + std::sin(x * 0.007f + wi * 0.4f) * amp * 0.5f;
            wave.lineTo((float)x, y);
        }
        wave.lineTo((float)W, (float)H);
        wave.lineTo(0.f, (float)H);
        wave.closeSubPath();
        g.setColour(Colour(0x060A1E38).withAlpha(0.04f + wi * 0.02f));
        g.fillPath(wave);
    }

    // Scanlines (very subtle)
    g.setColour(Colour(0x0400BBDD));
    for (int y = 0; y < H; y += 3)
        g.drawHorizontalLine(y, 0, (float)W);

    // ── Header bar ────────────────────────────────────────────────────────────
    ColourGradient headerGrad(Colour(0xCC020810), 0, 0,
                              Colour(0xAA040C1A), 0, 62, false);
    g.setGradientFill(headerGrad);
    g.fillRect(0, 0, W, 62);

    // Header bottom border (cyan glow)
    g.setColour(C::cyan.withAlpha(0.5f));
    g.drawHorizontalLine(62, 0, (float)W);
    g.setColour(C::cyan.withAlpha(0.1f));
    g.drawHorizontalLine(63, 0, (float)W);

    // Title glow background
    g.setColour(C::cyan.withAlpha(0.05f));
    g.fillRoundedRectangle(6, 6, 200, 50, 8.f);

    // ── Hemisphere panels ─────────────────────────────────────────────────────
    auto drawHemiPanel = [&](Rectangle<int> r, const char* title, Colour accent) {
        // Drop shadow
        g.setColour(Colour(0x50000010));
        g.fillRoundedRectangle(r.toFloat().translated(1, 3).expanded(0.5f), 10.f);

        // Panel fill — gradient (hemisphere: brighter blue top, dark bottom)
        ColourGradient panelGrad(
            Colour(0xFF0D1B2E), (float)r.getX(), (float)r.getY(),
            Colour(0xFF050C18), (float)r.getX(), (float)r.getBottom(), false
        );
        g.setGradientFill(panelGrad);
        g.fillRoundedRectangle(r.toFloat(), 10.f);

        // Hemisphere top highlight (dome reflection — light on top half)
        ColourGradient highlight(
            accent.withAlpha(0.10f), (float)r.getCentreX(), (float)r.getY(),
            accent.withAlpha(0.00f), (float)r.getCentreX(), r.getY() + r.getHeight() * 0.45f, false
        );
        g.setGradientFill(highlight);
        g.fillRoundedRectangle(r.toFloat().withHeight(r.getHeight() * 0.45f), 10.f);

        // Outer border (colored glow)
        g.setColour(accent.withAlpha(0.45f));
        g.drawRoundedRectangle(r.toFloat().reduced(0.5f), 10.f, 1.5f);

        // Inner rim (darker, subtle 3D depth)
        g.setColour(Colour(0x18FFFFFF));
        g.drawRoundedRectangle(r.toFloat().reduced(2.f), 8.f, 0.8f);

        // Thin accent line under title
        float lineY = (float)r.getY() + 20.f;
        ColourGradient lineGrad(accent.withAlpha(0.5f), (float)r.getX() + 12, lineY,
                                 accent.withAlpha(0.0f), (float)r.getRight() - 12, lineY, false);
        g.setGradientFill(lineGrad);
        g.drawHorizontalLine((int)lineY, (float)r.getX() + 12, (float)r.getRight() - 12);

        // Title text
        g.setColour(accent);
        g.setFont(Font("Arial", 9.f, Font::bold));
        g.drawText(title, r.withHeight(22), Justification::centredTop, false);
    };

    drawHemiPanel({8,   65, 218, 428}, "SPRING REVERB", C::cyan);
    drawHemiPanel({233, 65, 162, 428}, "TREMOLO",       C::teal);
    drawHemiPanel({402, 65, 234, 428}, "EFFETS",        C::cobalt);
    drawHemiPanel({643, 65, 110, 206}, "GUITARE",       C::seafoam);
    drawHemiPanel({643, 277, 110, 216}, "NIVEAU",       C::cyan.withAlpha(0.7f));
}

// ── Resized ───────────────────────────────────────────────────────────────────
void GhostSurfEditor::resized()
{
    // Header
    titleLabel.setBounds(10, 12, 210, 38);
    presetBox.setBounds(228, 17, 210, 28);

    // Mode buttons
    modeNormal.setBounds(450, 17, 90, 28);
    modeSwell.setBounds (545, 17, 90, 28);
    modeArpege.setBounds(640, 17, 90, 28);

    const int KS = 52;
    const int KY = 130;

    // REVERB section (x=8, w=218) — 3 knobs at top
    placeKnob(reverbMix,   58,  KY, KS);
    placeKnob(reverbDecay, 117, KY, KS);
    placeKnob(reverbTone,  176, KY, KS);

    // Waveform display fills lower 2/3 of reverb panel
    waveDisplay.setBounds(14, 200, 206, 282);

    // TREMOLO section (x=233, w=162)
    placeKnob(tremSpeed, 284, KY, KS);
    placeKnob(tremDepth, 354, KY, KS);
    tremSyncBtn.setBounds(240, 202, 82, 24);
    tremDivBox.setBounds(328, 202, 60, 24);

    // EFFETS section (x=402, w=234) — 4 knobs: drive, lofi, bass, treble
    placeKnob(drive,  449, KY, KS);
    placeKnob(lofi,   508, KY, KS);
    placeKnob(bass,   567, KY, KS);
    placeKnob(treble, 626, KY, KS);

    // GUITARE section (x=643, w=110) — shows controls by mode
    bool isSwell  = (currentMode == 1);
    bool isArpege = (currentMode == 2);
    bool isNormal = (currentMode == 0);

    swellAttack.slider.setVisible(isSwell);
    swellAttack.label.setVisible(isSwell);
    swellAmount.slider.setVisible(isSwell);
    swellAmount.label.setVisible(isSwell);

    slideAmount.slider.setVisible(isNormal);
    slideAmount.label.setVisible(isNormal);
    slideSpeed.slider.setVisible(isNormal);
    slideSpeed.label.setVisible(isNormal);

    arpPatternBox.setVisible(isArpege);
    arpDisplay.setVisible(isArpege);

    const int GCX = 698; // centre X of guitare section

    placeKnob(swellAttack, GCX, 110, KS);
    placeKnob(swellAmount, GCX, 195, KS);
    placeKnob(slideAmount, GCX, 110, KS);
    placeKnob(slideSpeed,  GCX, 195, KS);

    // Arpeggiator controls in guitare panel
    arpPatternBox.setBounds(647, 86, 102, 24);
    arpDisplay.setBounds(647, 116, 102, 32);  // 8 step boxes

    // VU meter in niveau panel
    vuMeter.setBounds(649, 285, 98, 198);
}
