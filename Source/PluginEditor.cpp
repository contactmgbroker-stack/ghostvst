#include "PluginEditor.h"

using namespace juce;

// ── Palette océan ─────────────────────────────────────────────────────────────
namespace C {
    const Colour bg       { 0xFF02060E };
    const Colour panel    { 0xCC080F1E };
    const Colour cyan     { 0xFF00CCFF };
    const Colour teal     { 0xFF00E5CC };
    const Colour cobalt   { 0xFF5599FF };
    const Colour seafoam  { 0xFF00DD88 };
    const Colour white    { 0xFFDDF0FF };
    const Colour dimWhite { 0xFF4A6888 };
    const Colour border   { 0x4400AACC };
    const Colour btnOff   { 0xFF04101E };
    // Live highlight : rouge → orange → jaune
    const Colour live1    { 0xFFFF2200 };  // 1er = rouge
    const Colour live2    { 0xFFFF8800 };  // 2e  = orange
    const Colour live3    { 0xFFFFCC00 };  // 3e  = jaune
    // Compat aliases
    const Colour amber  = teal;
    const Colour purple = cobalt;
    const Colour green  = seafoam;
}

// ── Pattern préréglés (miroir du processor) ──────────────────────────────────
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
    setColour(ComboBox::backgroundColourId, C::panel);
    setColour(ComboBox::textColourId,       C::white);
    setColour(ComboBox::arrowColourId,      C::cyan);
    setColour(ComboBox::outlineColourId,    C::border);
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
    auto bounds = Rectangle<float>((float)x,(float)y,(float)w,(float)h);
    auto centre = bounds.getCentre();
    float r = jmin(bounds.getWidth(), bounds.getHeight()) * 0.42f;

    Colour accent = C::cyan;
    auto tag = slider.getComponentID();
    if (tag == "teal"   || tag == "amber")  accent = C::teal;
    if (tag == "cobalt" || tag == "purple") accent = C::cobalt;
    if (tag == "seafoam"|| tag == "green")  accent = C::seafoam;

    // ── Live highlight (rang 1=rouge, 2=orange, 3=jaune) ────────────────────
    int liveRank = (int)slider.getProperties()["live"];
    if (liveRank > 0) {
        Colour lc = liveRank == 1 ? C::live1 : liveRank == 2 ? C::live2 : C::live3;
        g.setColour(lc.withAlpha(0.25f));
        g.drawEllipse(centre.x-r-7, centre.y-r-7, (r+7)*2.f, (r+7)*2.f, 6.f);
        g.setColour(lc.withAlpha(0.85f));
        g.drawEllipse(centre.x-r-2.5f, centre.y-r-2.5f, (r+2.5f)*2.f, (r+2.5f)*2.f, 2.f);
        g.setColour(lc.brighter(0.5f).withAlpha(0.5f));
        g.drawEllipse(centre.x-r-0.5f, centre.y-r-0.5f, (r+0.5f)*2.f, (r+0.5f)*2.f, 1.f);
    }

    // Fond knob
    g.setColour(Colour(0xFF04101E));
    g.fillEllipse(centre.x-r, centre.y-r, r*2, r*2);
    g.setColour(accent.withAlpha(0.2f));
    g.drawEllipse(centre.x-r, centre.y-r, r*2, r*2, 1.5f);

    // Arc bg
    Path arcBg;
    arcBg.addArc(centre.x-r+4, centre.y-r+4, (r-4)*2, (r-4)*2, startAngle, endAngle, true);
    g.setColour(Colour(0xFF0A1828));
    g.strokePath(arcBg, PathStrokeType(3.5f));

    // Arc valeur
    float angle = startAngle + sliderPos*(endAngle-startAngle);
    Path arcVal;
    arcVal.addArc(centre.x-r+4, centre.y-r+4, (r-4)*2, (r-4)*2, startAngle, angle, true);
    g.setColour(accent);
    g.strokePath(arcVal, PathStrokeType(3.5f, PathStrokeType::curved, PathStrokeType::rounded));

    // Dot lumineux au bout de l'arc
    auto arcEnd = centre.getPointOnCircumference(r-4, angle);
    g.setColour(accent.withAlpha(0.55f));
    g.fillEllipse(arcEnd.x-5, arcEnd.y-5, 10, 10);
    g.setColour(accent.brighter(0.4f));
    g.fillEllipse(arcEnd.x-2.5f, arcEnd.y-2.5f, 5, 5);

    // Corps du knob (hémisphère 3D)
    float ir = r*0.70f;
    ColourGradient kg(Colour(0xFF1A2E44), centre.x, centre.y-ir,
                      Colour(0xFF060E1C), centre.x, centre.y+ir, false);
    g.setGradientFill(kg);
    g.fillEllipse(centre.x-ir, centre.y-ir, ir*2, ir*2);
    g.setColour(accent.withAlpha(0.15f));
    g.drawEllipse(centre.x-ir, centre.y-ir, ir*2, ir*2, 1.f);

    // Indicator
    auto dot = centre.getPointOnCircumference(ir*0.65f, angle);
    g.setColour(accent.brighter(0.2f));
    g.fillEllipse(dot.x-3, dot.y-3, 6, 6);

    // Valeur texte
    g.setColour(C::white.withAlpha(0.85f));
    g.setFont(Font("Arial", 8.f, Font::plain));
    String vs; double val = slider.getValue();
    if (val == (int)val) vs = String((int)val); else vs = String(val,1);
    g.drawText(vs, (int)(centre.x-20), (int)(centre.y+ir+3), 40, 12, Justification::centred);
}

void OceanLookAndFeel::drawComboBox(Graphics& g, int w, int h, bool,
                                     int bx, int by, int bw, int bh, ComboBox&)
{
    ColourGradient bg(Colour(0xFF0C1828), 0,0, Colour(0xFF060E1C), 0,h, false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(0.f,0.f,(float)w,(float)h, 5.f);
    g.setColour(C::cyan.withAlpha(0.4f));
    g.drawRoundedRectangle(0.5f,0.5f,w-1.f,h-1.f, 5.f, 1.f);
    Path arrow;
    auto a = Rectangle<int>(bx,by,bw,bh).toFloat();
    arrow.startNewSubPath(a.getX()+4, a.getCentreY()-2);
    arrow.lineTo(a.getCentreX(), a.getCentreY()+3);
    arrow.lineTo(a.getRight()-4, a.getCentreY()-2);
    g.setColour(C::cyan);
    g.strokePath(arrow, PathStrokeType(1.5f));
}

void OceanLookAndFeel::drawButtonBackground(Graphics& g, Button& btn,
                                             const Colour&, bool highlighted, bool)
{
    bool active = btn.getToggleState();
    auto b = btn.getLocalBounds().toFloat();
    if (active) {
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
void OceanLookAndFeel::drawLabel(Graphics& g, Label& label) {
    g.setColour(label.findColour(Label::textColourId));
    g.setFont(getLabelFont(label));
    g.drawText(label.getText(), label.getLocalBounds(), Justification::centred, false);
}

// ── WaveformDisplay (cadre en forme de vague) ─────────────────────────────────
WaveformDisplay::WaveformDisplay(GhostSurfProcessor& p) : proc(p) { startTimerHz(30); }

void WaveformDisplay::paint(Graphics& g)
{
    auto b   = getLocalBounds().toFloat();
    const float BW = b.getWidth(), BH = b.getHeight();
    const float x0 = b.getX(),    y0 = b.getY();

    const float waveAmp    = 18.f;
    const float waveOffset = waveAmp + 10.f;
    const int   numPts     = 240;

    auto topWaveY = [&](float t) -> float {
        return y0 + waveOffset
             - std::sin(t * MathConstants<float>::pi * 4.8f) * waveAmp * 0.90f
             - std::sin(t * MathConstants<float>::pi * 2.1f) * waveAmp * 0.45f
             - std::sin(t * MathConstants<float>::pi * 9.0f) * waveAmp * 0.12f;
    };

    Path waveShape;
    waveShape.startNewSubPath(x0, y0+BH);
    waveShape.lineTo(x0+BW, y0+BH);
    waveShape.lineTo(x0+BW, topWaveY(1.f));
    for (int i = numPts; i >= 0; --i)
        waveShape.lineTo(x0+(float)i/numPts*BW, topWaveY((float)i/numPts));
    waveShape.closeSubPath();

    ColourGradient bg(Colour(0xFF020A16), x0, y0+waveOffset,
                      Colour(0xFF050E1E), x0, y0+BH, false);
    g.setGradientFill(bg);
    g.fillPath(waveShape);

    {
        Graphics::ScopedSaveState ss(g);
        g.reduceClipRegion(waveShape);

        float innerTop = y0+waveOffset, innerH = BH-waveOffset;
        float cy = innerTop + innerH*0.5f;

        g.setColour(Colour(0x0D00AACC));
        for (int gi=1; gi<5; ++gi)
            g.drawHorizontalLine((int)(innerTop+innerH*gi/5.f), x0+6, x0+BW-6);
        for (int gi=1; gi<5; ++gi)
            g.drawVerticalLine((int)(x0+BW*gi/5.f), (int)innerTop+4, (int)(y0+BH)-4);
        g.setColour(Colour(0x2800CCFF));
        g.drawHorizontalLine((int)cy, x0+6, x0+BW-6);

        const float* data = proc.getScopePtr();
        int wp = proc.getScopeWritePos();
        const int N = GhostSurfProcessor::SCOPE_SIZE, PW = (int)BW-8;
        Path oscPath; bool started = false;
        for (int xi=0; xi<PW; ++xi) {
            int   idx = (wp+(int)((float)xi*N/PW)) % N;
            float py  = cy - jlimit(-1.f,1.f,data[idx])*innerH*0.44f;
            float px  = x0+4.f+xi;
            if (!started) { oscPath.startNewSubPath(px,py); started=true; }
            else oscPath.lineTo(px,py);
        }
        g.setColour(Colour(0xFF00CCFF).withAlpha(0.12f));
        g.strokePath(oscPath, PathStrokeType(7.f, PathStrokeType::curved));
        g.setColour(Colour(0xFF00CCFF).withAlpha(0.38f));
        g.strokePath(oscPath, PathStrokeType(3.f, PathStrokeType::curved));
        g.setColour(Colour(0xFF00EEFF).withAlpha(0.88f));
        g.strokePath(oscPath, PathStrokeType(1.2f, PathStrokeType::curved));
    }

    // Crete de vague
    Path waveCrest;
    for (int i=0; i<=numPts; ++i) {
        float t=(float)i/numPts, wx=x0+t*BW, wy=topWaveY(t);
        if (i==0) waveCrest.startNewSubPath(wx,wy); else waveCrest.lineTo(wx,wy);
    }
    g.setColour(C::cyan.withAlpha(0.15f));
    g.strokePath(waveCrest, PathStrokeType(10.f, PathStrokeType::curved));
    g.setColour(C::cyan.withAlpha(0.40f));
    g.strokePath(waveCrest, PathStrokeType(4.f,  PathStrokeType::curved));
    g.setColour(Colour(0xFFCCF5FF).withAlpha(0.85f));
    g.strokePath(waveCrest, PathStrokeType(1.3f, PathStrokeType::curved));

    // Ecume aux cretes
    float prevDy = topWaveY(1.f/numPts)-topWaveY(0.f);
    for (int i=2; i<numPts-1; i+=2) {
        float t=(float)i/numPts, dy=topWaveY((float)(i+1)/numPts)-topWaveY(t);
        if (prevDy<=0.f && dy>0.f) {
            float cx=x0+t*BW, top=topWaveY(t);
            for (int d=-3; d<=3; ++d) {
                float fr=jmax(0.5f, 3.5f-std::abs(d)*0.7f);
                float fa=jmax(0.f,  0.80f-std::abs(d)*0.12f);
                g.setColour(Colour(0xFFDDF8FF).withAlpha(fa));
                g.fillEllipse(cx+d*5.5f-fr, top-fr-std::abs(d)*1.2f-fr, fr*2.f, fr*2.f);
            }
        }
        prevDy = dy;
    }
    g.setColour(C::cyan.withAlpha(0.30f));
    g.drawLine(x0, topWaveY(0.f), x0, y0+BH, 1.5f);
    g.drawLine(x0+BW, topWaveY(1.f), x0+BW, y0+BH, 1.5f);
    g.drawLine(x0, y0+BH, x0+BW, y0+BH, 1.5f);
}

// ── ArpEditor — séquenceur interactif 8 steps ─────────────────────────────────
ArpEditor::ArpEditor(GhostSurfProcessor& p) : proc(p) { startTimerHz(30); }

int ArpEditor::getStepAt(float x) const
{
    float cellW = (float)(getWidth() - 20) / 8.f;
    int s = (int)((x - 10.f) / cellW);
    return jlimit(0, 7, s);
}

float ArpEditor::getValForY(float y) const
{
    float usable = (float)(getHeight() - 30);
    float v = 1.f - jlimit(0.f, 1.f, (y - 10.f) / usable);
    // Snap à 0 et 0.5 si proche
    if (v < 0.07f) return 0.f;
    if (v > 0.93f) return 1.f;
    if (std::abs(v - 0.5f) < 0.08f) return 0.5f;
    return v;
}

void ArpEditor::mouseDown(const MouseEvent& e)
{
    dragStep = getStepAt((float)e.x);
    dragStartY   = (float)e.y;
    dragStartVal = proc.getArpStepValue(dragStep);

    if (e.mods.isRightButtonDown()) {
        proc.setArpStepValue(dragStep, 0.f);
        dragStep = -1;
    } else if (e.mods.isMiddleButtonDown()) {
        proc.setArpStepValue(dragStep, 0.5f);
        dragStep = -1;
    } else {
        // clic gauche : toggle entre 1 et 0, ou commencer drag
        float cur = proc.getArpStepValue(dragStep);
        proc.setArpStepValue(dragStep, cur > 0.5f ? 0.f : 1.f);
    }
}

void ArpEditor::mouseDrag(const MouseEvent& e)
{
    if (dragStep < 0) return;
    float dy = dragStartY - (float)e.y;   // positif = vers le haut = plus fort
    float newVal = jlimit(0.f, 1.f, dragStartVal + dy / (float)(getHeight()-30));
    if (newVal < 0.05f) newVal = 0.f;
    if (newVal > 0.95f) newVal = 1.f;
    if (std::abs(newVal - 0.5f) < 0.06f) newVal = 0.5f;
    proc.setArpStepValue(dragStep, newVal);
}

void ArpEditor::mouseUp(const MouseEvent&) { dragStep = -1; }

void ArpEditor::mouseDoubleClick(const MouseEvent& e)
{
    int s = getStepAt((float)e.x);
    proc.setArpStepValue(s, 0.f);
}

void ArpEditor::paint(Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    int currentStep = proc.getCurrentArpStep();

    // Fond
    ColourGradient bg(Colour(0xFF020A16), b.getX(), b.getY(),
                      Colour(0xFF040D1C), b.getX(), b.getBottom(), false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(b, 8.f);

    const int STEPS = 8;
    float padX  = 10.f, padY = 10.f;
    float usableW = b.getWidth() - padX*2.f;
    float usableH = b.getHeight() - padY - 20.f;  // 20px pour label bas
    float cellW   = usableW / STEPS;

    for (int s = 0; s < STEPS; ++s) {
        float val  = proc.getArpStepValue(s);
        bool  active = (s == currentStep);
        float cx   = b.getX() + padX + s*cellW;
        float cy   = b.getY() + padY;

        // Zone cliquable (fond cellule)
        auto cellBg = Rectangle<float>(cx+2, cy, cellW-4, usableH);
        g.setColour(active ? Colour(0xFF0A1E30) : Colour(0xFF060E1C));
        g.fillRoundedRectangle(cellBg, 4.f);
        g.setColour(active ? C::cyan.withAlpha(0.3f) : Colour(0x1800AACC));
        g.drawRoundedRectangle(cellBg.reduced(0.5f), 4.f, 1.f);

        // Barre de valeur (depuis le bas)
        if (val > 0.005f) {
            float barH  = usableH * val;
            float barY  = cy + usableH - barH;
            auto barRect = Rectangle<float>(cx+4, barY, cellW-8, barH);

            // Couleur dégradée selon intensité
            Colour barTop = val > 0.75f ? C::cyan : val > 0.35f ? C::teal : C::seafoam;
            ColourGradient barGrad(barTop.withAlpha(active ? 0.95f : 0.70f), cx, barY,
                                   barTop.withAlpha(active ? 0.55f : 0.35f), cx, barY+barH, false);
            g.setGradientFill(barGrad);
            g.fillRoundedRectangle(barRect, 3.f);

            // Top highlight de la barre
            g.setColour(barTop.brighter(0.3f).withAlpha(0.9f));
            g.fillRoundedRectangle(barRect.withHeight(3.f), 2.f);

            // Glow si step actif
            if (active) {
                g.setColour(C::cyan.withAlpha(0.25f));
                g.fillRoundedRectangle(barRect.expanded(3.f), 5.f);
            }
        }

        // Ligne de mi-step (guide visuel à 0.5)
        float midY = cy + usableH*0.5f;
        g.setColour(Colour(0x1500AACC));
        g.drawHorizontalLine((int)midY, cx+4, cx+cellW-4);

        // Numéro du step en bas
        g.setColour(active ? C::cyan : C::dimWhite);
        g.setFont(Font("Arial", 9.f, Font::bold));
        g.drawText(String(s+1), (int)(cx), (int)(b.getBottom()-18), (int)cellW, 14,
                   Justification::centred);

        // Label de valeur au-dessus de la barre
        if (val > 0.01f) {
            String valStr = val > 0.95f ? "•" : val > 0.45f && val < 0.55f ? "½" : String((int)(val*100)) + "%";
            g.setColour(C::white.withAlpha(0.7f));
            g.setFont(Font("Arial", 8.f, Font::plain));
            float barTopY = b.getY() + padY + usableH*(1.f-val);
            g.drawText(valStr, (int)(cx), (int)(barTopY)-12, (int)cellW, 11, Justification::centred);
        }
    }

    // Bordure externe
    g.setColour(C::cyan.withAlpha(0.40f));
    g.drawRoundedRectangle(b.reduced(0.5f), 8.f, 1.5f);
}

// ── VUMeter ──────────────────────────────────────────────────────────────────
VUMeter::VUMeter(GhostSurfProcessor& p) : proc(p) { startTimerHz(30); }
void VUMeter::timerCallback() { displayLevel += (proc.getOutputLevel()-displayLevel)*0.25f; repaint(); }
void VUMeter::paint(Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    ColourGradient bg(Colour(0xFF030A14),b.getX(),b.getY(),Colour(0xFF050C18),b.getX(),b.getBottom(),false);
    g.setGradientFill(bg); g.fillRoundedRectangle(b,4.f);
    g.setColour(C::cyan.withAlpha(0.3f)); g.drawRoundedRectangle(b.reduced(0.5f),4.f,1.f);
    const int SEGS=20; float segH=(b.getHeight()-20.f)/SEGS;
    float dbVal=Decibels::gainToDecibels(displayLevel,-60.f);
    int litN=jlimit(0,SEGS,(int)(jmap(dbVal,-48.f,0.f,0.f,1.f)*SEGS));
    for (int i=0; i<SEGS; ++i) {
        float segY=b.getBottom()-14.f-(i+1)*segH;
        auto seg=Rectangle<float>(b.getX()+5,segY,b.getWidth()-10,segH-1.5f);
        if (i<litN) {
            if      (i>=SEGS-2) g.setColour(Colour(0xFFFF2244));
            else if (i>=SEGS-5) g.setColour(Colour(0xFFFFAA00));
            else                g.setColour(C::seafoam);
        } else g.setColour(Colour(0xFF080F1E));
        g.fillRoundedRectangle(seg,1.5f);
    }
    g.setColour(C::dimWhite); g.setFont(8.f);
    g.drawText("dB", b.removeFromBottom(14.f), Justification::centred);
}

// ── Editor ────────────────────────────────────────────────────────────────────
void GhostSurfEditor::buildKnob(KnobWidget& kw, const char* paramID,
                                  const char* labelText, Colour accent)
{
    kw.slider.setSliderStyle(Slider::RotaryVerticalDrag);
    kw.slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    kw.slider.setLookAndFeel(&lf);
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
    kw.slider.setBounds(cx-size/2, cy-size/2, size, size);
    kw.label.setBounds(cx-28, cy+size/2+2, 56, 13);
}

GhostSurfEditor::GhostSurfEditor(GhostSurfProcessor& p)
    : AudioProcessorEditor(&p), proc(p), vuMeter(p), waveDisplay(p), arpEditor(p)
{
    setSize(760, 610);
    setLookAndFeel(&lf);

    titleLabel.setText("GHOST SURF", dontSendNotification);
    titleLabel.setFont(Font("Arial", 22.f, Font::bold));
    titleLabel.setColour(Label::textColourId, C::cyan);
    addAndMakeVisible(titleLabel);

    for (int i=0; i<GhostSurfProcessor::NUM_PRESETS; ++i)
        presetBox.addItem(p.getProgramName(i), i+1);
    presetBox.setSelectedId(p.getCurrentProgram()+1, dontSendNotification);
    presetBox.onChange = [&] {
        int idx = presetBox.getSelectedId()-1;
        proc.setCurrentProgram(idx);
        updateLiveHighlights(idx);
    };
    addAndMakeVisible(presetBox);

    auto setupModeBtn = [&](TextButton& btn, const String& text) {
        btn.setButtonText(text); btn.setClickingTogglesState(true);
        btn.setRadioGroupId(1); btn.setLookAndFeel(&lf);
        btn.setColour(TextButton::textColourOffId, C::white);
        btn.setColour(TextButton::textColourOnId,  C::cyan);
        addAndMakeVisible(btn);
    };
    setupModeBtn(modeNormal, "NORMAL");
    setupModeBtn(modeSwell,  "AUTO-SWELL");
    setupModeBtn(modeArpege, "ARPEGE");
    modeNormal.setToggleState(true, dontSendNotification);
    modeNormal.onClick = [&]{ setGuitarMode(0); };
    modeSwell.onClick  = [&]{ setGuitarMode(1); };
    modeArpege.onClick = [&]{ setGuitarMode(2); };

    tremSyncBtn.setButtonText("SYNC BPM");
    tremSyncBtn.setLookAndFeel(&lf);
    addAndMakeVisible(tremSyncBtn);
    tremSyncAttach = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(
        proc.getAPVTS(), "tremSync", tremSyncBtn);

    tremDivBox.addItem("1/4",1); tremDivBox.addItem("1/8",2); tremDivBox.addItem("1/16",3);
    addAndMakeVisible(tremDivBox);
    tremDivAttach = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        proc.getAPVTS(), "tremDiv", tremDivBox);

    // Pattern loader pour l'arp
    arpPatternBox.addItem("1/8 Gate",1); arpPatternBox.addItem("Triolets",2);
    arpPatternBox.addItem("A Forest",3); arpPatternBox.addItem("Syncope",4);
    arpPatternBox.addItem("Gallop",5);   arpPatternBox.addItem("Off-Beat",6);
    arpPatternBox.addItem("Surf Beat",7);arpPatternBox.addItem("Waltz",8);
    addAndMakeVisible(arpPatternBox);
    arpPatternAttach = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(
        proc.getAPVTS(), "arpPattern", arpPatternBox);
    arpPatternBox.onChange = [&]{
        proc.loadArpPattern(arpPatternBox.getSelectedId()-1);
    };

    buildKnob(reverbMix,   "reverbMix",   "MIX",    C::cyan);
    buildKnob(reverbDecay, "reverbDecay", "DECAY",  C::cyan);
    buildKnob(reverbTone,  "reverbTone",  "TONE",   C::cyan);
    buildKnob(tremSpeed,   "tremSpeed",   "VITESSE", C::teal);
    buildKnob(tremDepth,   "tremDepth",   "INTENSITE",C::teal);
    buildKnob(drive,       "drive",       "DRIVE",  C::cobalt);
    buildKnob(lofi,        "lofi",        "LO-FI",  C::cobalt);
    buildKnob(bass,        "bass",        "BASSES", C::cobalt);
    buildKnob(treble,      "treble",      "AIGUS",  C::cobalt);
    buildKnob(swellAttack, "swellAttack", "ATTAQUE",C::seafoam);
    buildKnob(swellAmount, "swellAmount", "AMOUNT", C::seafoam);
    buildKnob(slideAmount, "slideAmount", "GLISS",  C::seafoam);
    buildKnob(slideSpeed,  "slideSpeed",  "VITESSE",C::seafoam);

    addAndMakeVisible(vuMeter);
    addAndMakeVisible(waveDisplay);
    addAndMakeVisible(arpEditor);

    updateLiveHighlights(p.getCurrentProgram());
    startTimerHz(10);
}

GhostSurfEditor::~GhostSurfEditor() { setLookAndFeel(nullptr); }

void GhostSurfEditor::timerCallback()
{
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

// ── updateLiveHighlights : dégradé rouge→orange→jaune par rang ──────────────
void GhostSurfEditor::updateLiveHighlights(int idx)
{
    // [presetIdx] = { param_rang1(rouge), param_rang2(orange), param_rang3(jaune), arpBox? }
    struct LiveDef { const char* p1; const char* p2; const char* p3; bool arp; };
    static const LiveDef L[12] = {
        // 0  The Cure - A Forest   : tout est dans la reverb spring
        { "reverbMix",   "reverbDecay", "reverbTone",   true  },
        // 1  Lil Peep - Ghost      : lo-fi texture + vitesse swell
        { "lofi",        "swellAttack", "reverbMix",    false },
        // 2  Iggy Pop - Dog        : fuzz en 1er, tremolo vitesse, puis profondeur
        { "drive",       "tremSpeed",   "tremDepth",    false },
        // 3  Surf Clean            : tremolo vitesse, puis tone clair, puis mix reverb
        { "tremSpeed",   "reverbTone",  "tremDepth",    false },
        // 4  Night Waves           : swell attack donne le mood, puis reverb longue
        { "swellAttack", "reverbDecay", "swellAmount",  false },
        // 5  Dick Dale - Misirlou  : vitesse tremolo = adrénaline, puis profondeur
        { "tremSpeed",   "tremDepth",   "drive",        false },
        // 6  The Pixies - Monkey   : quantité swell, puis decay long = dreamy
        { "swellAmount", "swellAttack", "reverbDecay",  false },
        // 7  Joy Division - Trans. : reverb froide = âme du son
        { "reverbDecay", "reverbTone",  "reverbMix",    true  },
        // 8  Jack White - Slide    : slide en 1er = signature, drive = punch
        { "slideAmount", "drive",       "bass",         false },
        // 9  Haunted Motel         : queue reverb fantôme + tremolo hanté
        { "reverbDecay", "tremDepth",   "reverbMix",    true  },
        // 10 Jimi Hendrix - Purple : drive = tout, slide = whammy, treble = mordant
        { "drive",       "slideAmount", "treble",       false },
        // 11 Nile Rodgers - LeFreak: aigus funk, graves, puis lofi pour le grain
        { "treble",      "bass",        "lofi",         true  },
    };

    struct KM { const char* id; Slider* s; };
    KM km[] = {
        {"reverbMix",   &reverbMix.slider},   {"reverbDecay", &reverbDecay.slider},
        {"reverbTone",  &reverbTone.slider},   {"tremSpeed",   &tremSpeed.slider},
        {"tremDepth",   &tremDepth.slider},    {"drive",       &drive.slider},
        {"lofi",        &lofi.slider},         {"bass",        &bass.slider},
        {"treble",      &treble.slider},       {"swellAttack", &swellAttack.slider},
        {"swellAmount", &swellAmount.slider},  {"slideAmount", &slideAmount.slider},
        {"slideSpeed",  &slideSpeed.slider},
    };

    // Effacer
    for (auto& k : km) { k.s->getProperties().set("live", 0); k.s->repaint(); }
    arpBoxLive = false;

    if (idx < 0 || idx >= 12) return;
    const LiveDef& d = L[idx];
    const char* ranked[3] = { d.p1, d.p2, d.p3 };
    for (int rank=0; rank<3; ++rank)
        for (auto& k : km)
            if (std::strcmp(k.id, ranked[rank]) == 0)
                { k.s->getProperties().set("live", rank+1); k.s->repaint(); }

    arpBoxLive = d.arp;
    repaint();
}

// ── Paint ─────────────────────────────────────────────────────────────────────
void GhostSurfEditor::paint(Graphics& g)
{
    const int W=getWidth(), H=getHeight();

    ColourGradient bgGrad(Colour(0xFF020609),0,0, Colour(0xFF040C1A),0,H, false);
    g.setGradientFill(bgGrad); g.fillAll();

    for (int wi=0; wi<4; ++wi) {
        Path wave;
        float waveY=H*(0.15f+wi*0.22f), amp=18.f+wi*4.f;
        wave.startNewSubPath(0, waveY);
        for (int x=0; x<=W; x+=3)
            wave.lineTo((float)x, waveY+std::sin(x*0.018f+wi*1.1f)*amp
                                      +std::sin(x*0.007f+wi*0.4f)*amp*0.5f);
        wave.lineTo((float)W,(float)H); wave.lineTo(0.f,(float)H); wave.closeSubPath();
        g.setColour(Colour(0x060A1E38));
        g.fillPath(wave);
    }
    g.setColour(Colour(0x0400BBDD));
    for (int y=0; y<H; y+=3) g.drawHorizontalLine(y,0,(float)W);

    // Header
    ColourGradient hdr(Colour(0xCC020810),0,0, Colour(0xAA040C1A),0,62, false);
    g.setGradientFill(hdr); g.fillRect(0,0,W,62);
    g.setColour(C::cyan.withAlpha(0.5f)); g.drawHorizontalLine(62,0,(float)W);
    g.setColour(C::cyan.withAlpha(0.1f)); g.drawHorizontalLine(63,0,(float)W);
    g.setColour(C::cyan.withAlpha(0.05f)); g.fillRoundedRectangle(6,6,200,50,8.f);

    auto drawHemiPanel = [&](Rectangle<int> r, const char* title, Colour accent) {
        g.setColour(Colour(0x50000010));
        g.fillRoundedRectangle(r.toFloat().translated(1,3).expanded(0.5f), 10.f);
        ColourGradient pg(Colour(0xFF0D1B2E),(float)r.getX(),(float)r.getY(),
                          Colour(0xFF050C18),(float)r.getX(),(float)r.getBottom(),false);
        g.setGradientFill(pg); g.fillRoundedRectangle(r.toFloat(),10.f);
        ColourGradient hi(accent.withAlpha(0.10f),(float)r.getCentreX(),(float)r.getY(),
                          accent.withAlpha(0.00f),(float)r.getCentreX(),r.getY()+r.getHeight()*0.45f,false);
        g.setGradientFill(hi);
        g.fillRoundedRectangle(r.toFloat().withHeight(r.getHeight()*0.45f),10.f);
        g.setColour(accent.withAlpha(0.45f));
        g.drawRoundedRectangle(r.toFloat().reduced(0.5f),10.f,1.5f);
        g.setColour(Colour(0x18FFFFFF));
        g.drawRoundedRectangle(r.toFloat().reduced(2.f),8.f,0.8f);
        float lineY=(float)r.getY()+20.f;
        ColourGradient lg(accent.withAlpha(0.5f),(float)r.getX()+12,lineY,
                          accent.withAlpha(0.0f),(float)r.getRight()-12,lineY,false);
        g.setGradientFill(lg); g.drawHorizontalLine((int)lineY,(float)r.getX()+12,(float)r.getRight()-12);
        g.setColour(accent); g.setFont(Font("Arial",9.f,Font::bold));
        g.drawText(title, r.withHeight(22), Justification::centredTop, false);
    };

    drawHemiPanel({8,   65, 218, 420}, "SPRING REVERB", C::cyan);
    drawHemiPanel({233, 65, 162, 420}, "TREMOLO",       C::teal);
    drawHemiPanel({402, 65, 234, 420}, "EFFETS",        C::cobalt);
    drawHemiPanel({643, 65, 110, 206}, "GUITARE",       C::seafoam);
    drawHemiPanel({643, 277, 110, 208}, "NIVEAU",       C::cyan.withAlpha(0.7f));

    // Panel arpégiateur (bas)
    drawHemiPanel({8, 492, 744, 108}, "ARPÉGIATEUR", C::teal);

    // Label pattern loader
    g.setColour(C::teal.withAlpha(0.7f));
    g.setFont(Font("Arial", 8.f, Font::bold));
    g.drawText("PATTERN:", 580, 498, 60, 14, Justification::centredRight);

    // Cadre rouge sur arpPatternBox (live)
    if (arpBoxLive && currentMode == 2) {
        auto ab = arpPatternBox.getBounds().toFloat();
        g.setColour(C::live1.withAlpha(0.25f));
        g.drawRoundedRectangle(ab.expanded(5.f), 7.f, 5.f);
        g.setColour(C::live1.withAlpha(0.85f));
        g.drawRoundedRectangle(ab.expanded(2.f), 6.f, 1.8f);
    }

    // Légende dégradée
    int lx = W-160, ly = 20;
    for (int r=1; r<=3; ++r) {
        Colour lc = r==1 ? C::live1 : r==2 ? C::live2 : C::live3;
        g.setColour(lc.withAlpha(0.85f));
        g.fillEllipse((float)(lx+(r-1)*44), (float)ly, 8.f, 8.f);
    }
    g.setColour(C::dimWhite);
    g.setFont(Font("Arial", 8.f, Font::plain));
    g.drawText("live", lx+134, ly-1, 28, 11, Justification::centredLeft);
}

// ── Resized ───────────────────────────────────────────────────────────────────
void GhostSurfEditor::resized()
{
    titleLabel.setBounds(10,12,210,38);
    presetBox.setBounds(228,17,210,28);
    modeNormal.setBounds(450,17,90,28);
    modeSwell.setBounds (545,17,90,28);
    modeArpege.setBounds(640,17,90,28);

    const int KS=52, KY=130;
    placeKnob(reverbMix,   58,  KY, KS);
    placeKnob(reverbDecay, 117, KY, KS);
    placeKnob(reverbTone,  176, KY, KS);
    waveDisplay.setBounds(14, 198, 206, 278);

    placeKnob(tremSpeed, 284, KY, KS);
    placeKnob(tremDepth, 354, KY, KS);
    tremSyncBtn.setBounds(240,202,82,24);
    tremDivBox.setBounds(328,202,60,24);

    placeKnob(drive,  449, KY, KS);
    placeKnob(lofi,   508, KY, KS);
    placeKnob(bass,   567, KY, KS);
    placeKnob(treble, 626, KY, KS);

    bool isSwell  = (currentMode==1);
    bool isNormal = (currentMode==0);
    swellAttack.slider.setVisible(isSwell); swellAttack.label.setVisible(isSwell);
    swellAmount.slider.setVisible(isSwell); swellAmount.label.setVisible(isSwell);
    slideAmount.slider.setVisible(isNormal); slideAmount.label.setVisible(isNormal);
    slideSpeed.slider.setVisible(isNormal);  slideSpeed.label.setVisible(isNormal);
    arpPatternBox.setVisible(false);  // affiché dans le panel bas

    const int GCX=698;
    placeKnob(swellAttack, GCX, 110, KS);
    placeKnob(swellAmount, GCX, 195, KS);
    placeKnob(slideAmount, GCX, 110, KS);
    placeKnob(slideSpeed,  GCX, 195, KS);

    vuMeter.setBounds(649,285,98,195);

    // Arpégiateur : panel bas complet
    // Pattern loader en haut à droite du panel
    arpPatternBox.setBounds(646, 497, 102, 22);
    arpPatternBox.setVisible(true);
    // Séquenceur de steps
    arpEditor.setBounds(14, 500, 628, 92);
}
