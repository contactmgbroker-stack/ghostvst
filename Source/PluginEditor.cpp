#include "PluginEditor.h"
using namespace juce;

// ── Palette : nuances de bleu clair ──────────────────────────────────────────
namespace C {
    const Colour bg    { 0xFF06101C };   // fond nuit profonde
    const Colour sky   { 0xFF5BC8FF };   // bleu ciel   — reverb
    const Colour aqua  { 0xFF00D8C8 };   // aquamarine  — tremolo
    const Colour cobalt{ 0xFF6AABFF };   // cobalt clair— effets
    const Colour mint  { 0xFF44E8A8 };   // menthe      — guitare
    const Colour ice   { 0xFFCCEEFF };   // blanc glacé
    const Colour dim   { 0xFF5080A0 };   // gris-bleu discret
    const Colour live1 { 0xFFFF3322 };   // rouge — potard #1
    const Colour live2 { 0xFFFF8800 };   // orange— potard #2
    const Colour live3 { 0xFFFFCC00 };   // jaune — potard #3
}

// Retourne la couleur accent d'un slider à partir de son tag
static Colour accentOf(const Slider& s) {
    auto id = s.getComponentID();
    if (id == "aqua")   return C::aqua;
    if (id == "cobalt") return C::cobalt;
    if (id == "mint")   return C::mint;
    return C::sky;
}

// ── OceanLookAndFeel ──────────────────────────────────────────────────────────
OceanLookAndFeel::OceanLookAndFeel()
{
    setColour(ComboBox::backgroundColourId,  Colour(0xFF0C1E30));
    setColour(ComboBox::textColourId,        C::ice);
    setColour(ComboBox::arrowColourId,       C::sky);
    setColour(ComboBox::outlineColourId,     C::sky.withAlpha(0.35f));
    setColour(PopupMenu::backgroundColourId, Colour(0xFF0C1E30));
    setColour(PopupMenu::textColourId,       C::ice);
    setColour(PopupMenu::highlightedBackgroundColourId, C::sky.withAlpha(0.20f));
    setColour(Label::textColourId,           C::dim);
    setColour(ToggleButton::textColourId,    C::ice);
    setColour(ToggleButton::tickColourId,    C::sky);
}

// ── Knob ─────────────────────────────────────────────────────────────────────
void OceanLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int w, int h,
                                         float pos, float start, float end, Slider& sl)
{
    auto b  = Rectangle<float>((float)x,(float)y,(float)w,(float)h);
    auto cc = b.getCentre();
    float r = jmin(b.getWidth(), b.getHeight()) * 0.42f;
    Colour ac = accentOf(sl);

    // ── Anneau live (rouge/orange/jaune) ────────────────────────────────────
    int rank = (int)sl.getProperties()["live"];
    if (rank > 0) {
        Colour lc = rank==1 ? C::live1 : rank==2 ? C::live2 : C::live3;
        g.setColour(lc.withAlpha(0.20f));
        g.drawEllipse(cc.x-r-7, cc.y-r-7, (r+7)*2, (r+7)*2, 6.f);
        g.setColour(lc.withAlpha(0.90f));
        g.drawEllipse(cc.x-r-2, cc.y-r-2, (r+2)*2, (r+2)*2, 1.8f);
    }

    // ── Arc piste (fond gris-bleu) ───────────────────────────────────────────
    float ar = r - 4.f;
    {
        Path p;
        p.addArc(cc.x-ar, cc.y-ar, ar*2, ar*2, start, end, true);
        g.setColour(Colour(0xFF112030));
        g.strokePath(p, PathStrokeType(3.5f));
    }

    // ── Arc valeur (bleu lumineux) ───────────────────────────────────────────
    float va = start + pos*(end-start);
    if (pos > 0.004f) {
        Path p;
        p.addArc(cc.x-ar, cc.y-ar, ar*2, ar*2, start, va, true);
        g.setColour(ac);
        g.strokePath(p, PathStrokeType(3.5f, PathStrokeType::curved, PathStrokeType::rounded));
        // Dot lumineux à l'extrémité
        auto ep = cc.getPointOnCircumference(ar, va);
        g.setColour(ac.withAlpha(0.45f));
        g.fillEllipse(ep.x-5, ep.y-5, 10, 10);
        g.setColour(C::ice);
        g.fillEllipse(ep.x-2.5f, ep.y-2.5f, 5, 5);
    }

    // ── Corps en verre (hémisphère) ──────────────────────────────────────────
    float ir = r * 0.68f;
    // Socle foncé
    g.setColour(Colour(0xFF040C16));
    g.fillEllipse(cc.x-r, cc.y-r, r*2, r*2);
    // Dégradé hémisphérique (lumière en haut-gauche)
    ColourGradient body(Colour(0xFF1E4060), cc.x-ir*0.3f, cc.y-ir,
                        Colour(0xFF060F1E), cc.x, cc.y+ir, false);
    g.setGradientFill(body);
    g.fillEllipse(cc.x-ir, cc.y-ir, ir*2, ir*2);
    // Reflet spéculaire (highlight blanc en haut)
    ColourGradient hl(Colour(0x30FFFFFF), cc.x-ir*0.2f, cc.y-ir*0.85f,
                      Colour(0x00FFFFFF), cc.x, cc.y, true);
    g.setGradientFill(hl);
    g.fillEllipse(cc.x-ir*0.6f, cc.y-ir*0.95f, ir*1.2f, ir*0.9f);
    // Liseré d'accent doux
    g.setColour(ac.withAlpha(0.18f));
    g.drawEllipse(cc.x-ir, cc.y-ir, ir*2, ir*2, 1.f);

    // ── Indicateur ──────────────────────────────────────────────────────────
    auto dp = cc.getPointOnCircumference(ir*0.60f, va);
    g.setColour(C::ice);
    g.fillEllipse(dp.x-2.5f, dp.y-2.5f, 5, 5);

    // ── Valeur texte ─────────────────────────────────────────────────────────
    g.setColour(C::dim);
    g.setFont(Font("Arial", 8.f, Font::plain));
    double v = sl.getValue();
    String vs = (v==(int)v) ? String((int)v) : String(v,1);
    g.drawText(vs,(int)(cc.x-20),(int)(cc.y+ir+3),40,12,Justification::centred);
}

// ── ComboBox ─────────────────────────────────────────────────────────────────
void OceanLookAndFeel::drawComboBox(Graphics& g, int w, int h, bool,
                                     int bx, int by, int bw, int bh, ComboBox&)
{
    ColourGradient bg(Colour(0xFF0E2035),0,0,Colour(0xFF081520),0,h,false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(0,0,w,h,5.f);
    g.setColour(C::sky.withAlpha(0.40f));
    g.drawRoundedRectangle(0.5f,0.5f,w-1.f,h-1.f,5.f,1.f);
    Path arrow;
    auto a=Rectangle<int>(bx,by,bw,bh).toFloat();
    arrow.startNewSubPath(a.getX()+4, a.getCentreY()-2);
    arrow.lineTo(a.getCentreX(), a.getCentreY()+3);
    arrow.lineTo(a.getRight()-4, a.getCentreY()-2);
    g.setColour(C::sky); g.strokePath(arrow,PathStrokeType(1.5f));
}

// ── Bouton mode ──────────────────────────────────────────────────────────────
void OceanLookAndFeel::drawButtonBackground(Graphics& g, Button& btn,
                                             const Colour&, bool hi, bool)
{
    auto b = btn.getLocalBounds().toFloat();
    bool on = btn.getToggleState();
    if (on) {
        ColourGradient gr(C::sky.withAlpha(0.28f),b.getX(),b.getY(),
                          C::sky.withAlpha(0.08f),b.getX(),b.getBottom(),false);
        g.setGradientFill(gr); g.fillRoundedRectangle(b,7.f);
        g.setColour(C::sky.withAlpha(0.85f));
        g.drawRoundedRectangle(b.reduced(0.5f),7.f,1.5f);
    } else {
        ColourGradient gr(Colour(0xFF0E2035),b.getX(),b.getY(),
                          Colour(0xFF081520),b.getX(),b.getBottom(),false);
        g.setGradientFill(gr); g.fillRoundedRectangle(b,7.f);
        g.setColour(hi ? C::sky.withAlpha(0.35f) : C::sky.withAlpha(0.20f));
        g.drawRoundedRectangle(b.reduced(0.5f),7.f,1.f);
    }
}

Font OceanLookAndFeel::getLabelFont(Label&) { return Font("Arial",10.f,Font::bold); }
void OceanLookAndFeel::drawLabel(Graphics& g, Label& l) {
    g.setColour(l.findColour(Label::textColourId));
    g.setFont(getLabelFont(l));
    g.drawText(l.getText(),l.getLocalBounds(),Justification::centred,false);
}

// ── Titre griffé ─────────────────────────────────────────────────────────────
static void drawScratchedTitle(Graphics& g, const String& text, float x, float y)
{
    Font f("Arial", 27.f, Font::bold | Font::italic);
    GlyphArrangement ga;
    ga.addLineOfText(f, text, x, y + 27.f);
    Path p; ga.createPath(p);

    // Ombre portée
    g.setColour(Colour(0xFF000810));
    g.fillPath(p, AffineTransform::translation(2.5f, 3.f));

    // Lueur diffuse
    g.setColour(C::sky.withAlpha(0.20f));
    g.fillPath(p, AffineTransform::translation(0.f, 1.f));

    // Remplissage gradient bleu clair → bleu ciel
    ColourGradient grad(C::ice, x, y, C::sky, x, y+30.f, false);
    g.setGradientFill(grad); g.fillPath(p);

    // Contour fin gravé (effet griffé)
    g.setColour(Colour(0xFFFFFFFF).withAlpha(0.55f));
    g.strokePath(p, PathStrokeType(0.55f));

    // Rayures diagonales (texture griffée)
    auto pb = p.getBounds();
    g.saveState();
    g.reduceClipRegion(p);
    g.setColour(Colour(0xFFFFFFFF).withAlpha(0.07f));
    for (float sx = pb.getX()-8.f; sx < pb.getRight()+8.f; sx += 13.f)
        g.drawLine(sx+9.f, pb.getY()-2.f, sx, pb.getBottom()+2.f, 1.5f);
    g.restoreState();
}

// ── WaveformDisplay ───────────────────────────────────────────────────────────
WaveformDisplay::WaveformDisplay(GhostSurfProcessor& p) : proc(p) { startTimerHz(30); }

void WaveformDisplay::paint(Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float BW=b.getWidth(), BH=b.getHeight(), x0=b.getX(), y0=b.getY();
    const float wAmp=16.f, wOff=wAmp+10.f;
    const int NP=220;

    auto wY = [&](float t) {
        return y0+wOff - std::sin(t*MathConstants<float>::pi*4.8f)*wAmp*0.88f
                       - std::sin(t*MathConstants<float>::pi*2.0f)*wAmp*0.42f
                       - std::sin(t*MathConstants<float>::pi*9.2f)*wAmp*0.10f;
    };

    // Forme de vague (clip + fond)
    Path shape;
    shape.startNewSubPath(x0,y0+BH); shape.lineTo(x0+BW,y0+BH);
    shape.lineTo(x0+BW, wY(1.f));
    for (int i=NP;i>=0;--i) shape.lineTo(x0+(float)i/NP*BW, wY((float)i/NP));
    shape.closeSubPath();

    ColourGradient bg(Colour(0xFF071525),x0,y0+wOff, Colour(0xFF050E1C),x0,y0+BH,false);
    g.setGradientFill(bg); g.fillPath(shape);

    // Oscilloscope (clippé)
    {
        Graphics::ScopedSaveState ss(g);
        g.reduceClipRegion(shape);
        float iTop=y0+wOff, iH=BH-wOff, cy=iTop+iH*0.5f;
        // Grille légère
        g.setColour(Colour(0x0C00AACC));
        for (int i=1;i<5;++i) g.drawHorizontalLine((int)(iTop+iH*i/5.f), x0+5, x0+BW-5);
        g.setColour(Colour(0x2000BBDD));
        g.drawHorizontalLine((int)cy, x0+5, x0+BW-5);
        // Forme d'onde
        const float* d=proc.getScopePtr(); int wp=proc.getScopeWritePos();
        const int N=GhostSurfProcessor::SCOPE_SIZE, PW=(int)BW-6;
        Path osc; bool st=false;
        for (int i=0;i<PW;++i) {
            float s=jlimit(-1.f,1.f,d[(wp+i*N/PW)%N]);
            float px=x0+3.f+i, py=cy-s*iH*0.43f;
            if (!st){osc.startNewSubPath(px,py);st=true;} else osc.lineTo(px,py);
        }
        g.setColour(C::sky.withAlpha(0.12f)); g.strokePath(osc,PathStrokeType(6.f,PathStrokeType::curved));
        g.setColour(C::sky.withAlpha(0.40f)); g.strokePath(osc,PathStrokeType(2.5f,PathStrokeType::curved));
        g.setColour(C::ice.withAlpha(0.88f)); g.strokePath(osc,PathStrokeType(1.f,PathStrokeType::curved));
    }

    // Crete de vague (bord lumineux)
    Path crest;
    for (int i=0;i<=NP;++i) {
        float t=(float)i/NP, wx=x0+t*BW, wy=wY(t);
        if (i==0) crest.startNewSubPath(wx,wy); else crest.lineTo(wx,wy);
    }
    g.setColour(C::sky.withAlpha(0.13f)); g.strokePath(crest,PathStrokeType(9.f,PathStrokeType::curved));
    g.setColour(C::sky.withAlpha(0.45f)); g.strokePath(crest,PathStrokeType(3.5f,PathStrokeType::curved));
    g.setColour(C::ice.withAlpha(0.80f)); g.strokePath(crest,PathStrokeType(1.2f,PathStrokeType::curved));

    // Ecume aux cretes
    float prevDy=wY(1.f/NP)-wY(0.f);
    for (int i=2;i<NP-1;i+=2) {
        float t=(float)i/NP, dy=wY((float)(i+1)/NP)-wY(t);
        if (prevDy<=0.f && dy>0.f) {
            float cx=x0+t*BW, ty=wY(t);
            for (int d=-2;d<=2;++d) {
                float fr=jmax(0.5f,3.f-std::abs(d)*0.6f), fa=jmax(0.f,0.75f-std::abs(d)*0.13f);
                g.setColour(Colour(0xFFE8F8FF).withAlpha(fa));
                g.fillEllipse(cx+d*5.f-fr, ty-fr-std::abs(d)*1.f-fr, fr*2, fr*2);
            }
        }
        prevDy=dy;
    }
    // Bordures latérales et bas
    g.setColour(C::sky.withAlpha(0.28f));
    g.drawLine(x0,wY(0.f),x0,y0+BH,1.2f);
    g.drawLine(x0+BW,wY(1.f),x0+BW,y0+BH,1.2f);
    g.drawLine(x0,y0+BH,x0+BW,y0+BH,1.2f);
}

// ── ArpEditor ─────────────────────────────────────────────────────────────────
ArpEditor::ArpEditor(GhostSurfProcessor& p) : proc(p) { startTimerHz(30); }

int   ArpEditor::getStepAt(float x)  const { return jlimit(0,7,(int)((x-10.f)/((getWidth()-20.f)/8.f))); }
float ArpEditor::getValForY(float y)  const {
    float v=1.f-jlimit(0.f,1.f,(y-8.f)/(getHeight()-28.f));
    if (v<0.06f) return 0.f; if (v>0.94f) return 1.f;
    if (std::abs(v-0.5f)<0.07f) return 0.5f;
    return v;
}

void ArpEditor::mouseDown(const MouseEvent& e) {
    dragStep=getStepAt((float)e.x); dragStartY=(float)e.y; dragStartVal=proc.getArpStepValue(dragStep);
    if (e.mods.isRightButtonDown()) { proc.setArpStepValue(dragStep,0.f); dragStep=-1; return; }
    float cur=proc.getArpStepValue(dragStep);
    proc.setArpStepValue(dragStep, cur>0.5f?0.f:1.f);
}
void ArpEditor::mouseDrag(const MouseEvent& e) {
    if (dragStep<0) return;
    float dy=dragStartY-(float)e.y;
    float v=jlimit(0.f,1.f,dragStartVal+dy/(getHeight()-28.f));
    if (v<0.05f)v=0.f; else if(v>0.95f)v=1.f; else if(std::abs(v-0.5f)<0.06f)v=0.5f;
    proc.setArpStepValue(dragStep,v);
}
void ArpEditor::mouseUp(const MouseEvent&) { dragStep=-1; }
void ArpEditor::mouseDoubleClick(const MouseEvent& e) { proc.setArpStepValue(getStepAt((float)e.x),0.f); }

void ArpEditor::paint(Graphics& g)
{
    auto b=getLocalBounds().toFloat();
    int cur=proc.getCurrentArpStep();

    // Fond verre sombre
    ColourGradient bg(Colour(0xFF081A28),b.getX(),b.getY(),
                      Colour(0xFF050E1C),b.getX(),b.getBottom(),false);
    g.setGradientFill(bg); g.fillRoundedRectangle(b,8.f);

    const float PX=10.f, PY=8.f;
    float cW=(b.getWidth()-PX*2.f)/8.f, cH=b.getHeight()-PY-20.f;

    for (int s=0;s<8;++s) {
        float val=proc.getArpStepValue(s);
        bool active=(s==cur);
        float cx=b.getX()+PX+s*cW;

        // Fond cellule
        auto cell=Rectangle<float>(cx+2,b.getY()+PY,cW-4,cH);
        g.setColour(active?Colour(0xFF0D2840):Colour(0xFF06121E));
        g.fillRoundedRectangle(cell,4.f);
        g.setColour((active?C::sky:C::sky.withAlpha(0.15f)).withAlpha(active?0.35f:0.15f));
        g.drawRoundedRectangle(cell.reduced(0.5f),4.f,1.f);

        // Barre de valeur
        if (val>0.005f) {
            float bH=cH*val, bY=b.getY()+PY+cH-bH;
            auto bar=Rectangle<float>(cx+4,bY,cW-8,bH);
            Colour bc = val>0.7f ? C::sky : val>0.35f ? C::aqua : C::mint;
            ColourGradient barG(bc.withAlpha(active?0.95f:0.72f),cx,bY,
                                bc.withAlpha(active?0.50f:0.30f),cx,bY+bH,false);
            g.setGradientFill(barG); g.fillRoundedRectangle(bar,3.f);
            // Top highlight
            g.setColour(C::ice.withAlpha(0.85f));
            g.fillRoundedRectangle(bar.withHeight(2.5f),2.f);
            // Glow step actif
            if (active) { g.setColour(C::sky.withAlpha(0.20f)); g.fillRoundedRectangle(bar.expanded(3.f),5.f); }
        }

        // Ligne guide à 50%
        float midY=b.getY()+PY+cH*0.5f;
        g.setColour(Colour(0x1000BBDD)); g.drawHorizontalLine((int)midY,cx+4,cx+cW-4);

        // Numéro
        g.setColour(active?C::sky:C::dim);
        g.setFont(Font("Arial",9.f,Font::bold));
        g.drawText(String(s+1),(int)cx,(int)(b.getBottom()-18),(int)cW,14,Justification::centred);

        // % valeur
        if (val>0.01f) {
            String vs=val>0.95f?"●":val>0.45f&&val<0.55f?"½":String((int)(val*100))+"%";
            g.setColour(C::ice.withAlpha(0.65f));
            g.setFont(Font("Arial",7.5f,Font::plain));
            float topY=b.getY()+PY+cH*(1.f-val);
            g.drawText(vs,(int)cx,(int)topY-11,(int)cW,10,Justification::centred);
        }
    }
    // Bordure externe bleutée
    g.setColour(C::sky.withAlpha(0.38f));
    g.drawRoundedRectangle(b.reduced(0.5f),8.f,1.5f);
}

// ── VUMeter ───────────────────────────────────────────────────────────────────
VUMeter::VUMeter(GhostSurfProcessor& p):proc(p){startTimerHz(30);}
void VUMeter::timerCallback(){displayLevel+=(proc.getOutputLevel()-displayLevel)*0.25f;repaint();}
void VUMeter::paint(Graphics& g)
{
    auto b=getLocalBounds().toFloat();
    ColourGradient bg(Colour(0xFF071525),b.getX(),b.getY(),Colour(0xFF050E1C),b.getX(),b.getBottom(),false);
    g.setGradientFill(bg); g.fillRoundedRectangle(b,4.f);
    g.setColour(C::sky.withAlpha(0.30f)); g.drawRoundedRectangle(b.reduced(0.5f),4.f,1.f);
    const int S=20; float sH=(b.getHeight()-20.f)/S;
    float db=Decibels::gainToDecibels(displayLevel,-60.f);
    int lit=jlimit(0,S,(int)(jmap(db,-48.f,0.f,0.f,1.f)*S));
    for (int i=0;i<S;++i) {
        auto seg=Rectangle<float>(b.getX()+5, b.getBottom()-14.f-(i+1)*sH, b.getWidth()-10, sH-1.5f);
        Colour sc = i<lit ? (i>=S-2?Colour(0xFFFF2244):i>=S-5?Colour(0xFFFFAA00):C::aqua) : Colour(0xFF0A1A28);
        g.setColour(sc); g.fillRoundedRectangle(seg,1.5f);
    }
    g.setColour(C::dim); g.setFont(8.f);
    g.drawText("dB",b.removeFromBottom(14.f),Justification::centred);
}

// ── Knob builder ─────────────────────────────────────────────────────────────
void GhostSurfEditor::buildKnob(KnobWidget& kw, const char* id, const char* lbl, Colour ac)
{
    kw.slider.setSliderStyle(Slider::RotaryVerticalDrag);
    kw.slider.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    kw.slider.setLookAndFeel(&lf);
    if      (ac==C::aqua)   kw.slider.setComponentID("aqua");
    else if (ac==C::cobalt) kw.slider.setComponentID("cobalt");
    else if (ac==C::mint)   kw.slider.setComponentID("mint");
    else                    kw.slider.setComponentID("sky");
    addAndMakeVisible(kw.slider);
    kw.label.setText(lbl,dontSendNotification);
    kw.label.setFont(Font("Arial",9.f,Font::bold));
    kw.label.setColour(Label::textColourId,ac);
    kw.label.setJustificationType(Justification::centred);
    addAndMakeVisible(kw.label);
    kw.attach=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(proc.getAPVTS(),id,kw.slider);
}
void GhostSurfEditor::placeKnob(KnobWidget& kw, int cx, int cy, int sz)
{
    kw.slider.setBounds(cx-sz/2,cy-sz/2,sz,sz);
    kw.label.setBounds(cx-28,cy+sz/2+2,56,13);
}

// ── Constructeur ─────────────────────────────────────────────────────────────
GhostSurfEditor::GhostSurfEditor(GhostSurfProcessor& p)
    : AudioProcessorEditor(&p), proc(p), vuMeter(p), waveDisplay(p), arpEditor(p)
{
    setSize(760,610); setLookAndFeel(&lf);

    titleLabel.setText("",dontSendNotification); addAndMakeVisible(titleLabel);

    for (int i=0;i<GhostSurfProcessor::NUM_PRESETS;++i) presetBox.addItem(p.getProgramName(i),i+1);
    presetBox.setSelectedId(p.getCurrentProgram()+1,dontSendNotification);
    presetBox.onChange=[&]{ int i=presetBox.getSelectedId()-1; proc.setCurrentProgram(i); updateLiveHighlights(i); };
    addAndMakeVisible(presetBox);

    auto addMode=[&](TextButton& btn, const String& t){
        btn.setButtonText(t); btn.setClickingTogglesState(true); btn.setRadioGroupId(1);
        btn.setLookAndFeel(&lf);
        btn.setColour(TextButton::textColourOffId,C::ice);
        btn.setColour(TextButton::textColourOnId,C::sky);
        addAndMakeVisible(btn);
    };
    addMode(modeNormal,"NORMAL"); addMode(modeSwell,"AUTO-SWELL"); addMode(modeArpege,"ARPEGE");
    modeNormal.setToggleState(true,dontSendNotification);
    modeNormal.onClick=[&]{setGuitarMode(0);}; modeSwell.onClick=[&]{setGuitarMode(1);}; modeArpege.onClick=[&]{setGuitarMode(2);};

    tremSyncBtn.setButtonText("SYNC BPM"); tremSyncBtn.setLookAndFeel(&lf); addAndMakeVisible(tremSyncBtn);
    tremSyncAttach=std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(proc.getAPVTS(),"tremSync",tremSyncBtn);

    tremDivBox.addItem("1/4",1); tremDivBox.addItem("1/8",2); tremDivBox.addItem("1/16",3);
    addAndMakeVisible(tremDivBox);
    tremDivAttach=std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(proc.getAPVTS(),"tremDiv",tremDivBox);

    arpPatternBox.addItem("1/8 Gate",1); arpPatternBox.addItem("Triolets",2);
    arpPatternBox.addItem("A Forest",3); arpPatternBox.addItem("Syncope",4);
    arpPatternBox.addItem("Gallop",5);   arpPatternBox.addItem("Off-Beat",6);
    arpPatternBox.addItem("Surf Beat",7);arpPatternBox.addItem("Waltz",8);
    addAndMakeVisible(arpPatternBox);
    arpPatternAttach=std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(proc.getAPVTS(),"arpPattern",arpPatternBox);
    arpPatternBox.onChange=[&]{ proc.loadArpPattern(arpPatternBox.getSelectedId()-1); };

    buildKnob(reverbMix,  "reverbMix",  "MIX",    C::sky);
    buildKnob(reverbDecay,"reverbDecay","DECAY",   C::sky);
    buildKnob(reverbTone, "reverbTone", "TONE",    C::sky);
    buildKnob(tremSpeed,  "tremSpeed",  "VITESSE", C::aqua);
    buildKnob(tremDepth,  "tremDepth",  "PROFOND", C::aqua);
    buildKnob(drive,      "drive",      "DRIVE",   C::cobalt);
    buildKnob(lofi,       "lofi",       "LO-FI",   C::cobalt);
    buildKnob(bass,       "bass",       "BASSES",  C::cobalt);
    buildKnob(treble,     "treble",     "AIGUS",   C::cobalt);
    buildKnob(swellAttack,"swellAttack","ATTAQUE",  C::mint);
    buildKnob(swellAmount,"swellAmount","AMOUNT",   C::mint);
    buildKnob(slideAmount,"slideAmount","GLISS",    C::mint);
    buildKnob(slideSpeed, "slideSpeed", "VITESSE",  C::mint);

    addAndMakeVisible(vuMeter); addAndMakeVisible(waveDisplay); addAndMakeVisible(arpEditor);
    updateLiveHighlights(p.getCurrentProgram());
    startTimerHz(10);
}
GhostSurfEditor::~GhostSurfEditor(){setLookAndFeel(nullptr);}

void GhostSurfEditor::timerCallback()
{
    int mode=(int)proc.getAPVTS().getRawParameterValue("guitarMode")->load();
    if (mode!=currentMode){currentMode=mode;
        modeNormal.setToggleState(mode==0,dontSendNotification);
        modeSwell.setToggleState(mode==1,dontSendNotification);
        modeArpege.setToggleState(mode==2,dontSendNotification);
        resized();}
}
void GhostSurfEditor::setGuitarMode(int m){
    currentMode=m;
    if(auto* p=proc.getAPVTS().getParameter("guitarMode")) p->setValueNotifyingHost(p->convertTo0to1((float)m));
    resized();
}

// ── updateLiveHighlights ─────────────────────────────────────────────────────
void GhostSurfEditor::updateLiveHighlights(int idx)
{
    struct Def { const char* p1,*p2,*p3; bool arp; };
    static const Def L[12]={
        {"reverbMix",  "reverbDecay","reverbTone",  true },  // The Cure
        {"lofi",       "swellAttack","reverbMix",   false},  // Lil Peep
        {"drive",      "tremSpeed",  "tremDepth",   false},  // Iggy Pop
        {"tremSpeed",  "reverbTone", "tremDepth",   false},  // Surf Clean
        {"swellAttack","reverbDecay","swellAmount",  false},  // Night Waves
        {"tremSpeed",  "tremDepth",  "drive",        false},  // Dick Dale
        {"swellAmount","swellAttack","reverbDecay",  false},  // The Pixies
        {"reverbDecay","reverbTone", "reverbMix",    true },  // Joy Division
        {"slideAmount","drive",      "bass",         false},  // Jack White
        {"reverbDecay","tremDepth",  "reverbMix",    true },  // Haunted Motel
        {"drive",      "slideAmount","treble",       false},  // Jimi Hendrix
        {"treble",     "bass",       "lofi",         true },  // Nile Rodgers
    };
    struct KM{const char* id; Slider* s;};
    KM km[]={{"reverbMix",&reverbMix.slider},{"reverbDecay",&reverbDecay.slider},
             {"reverbTone",&reverbTone.slider},{"tremSpeed",&tremSpeed.slider},
             {"tremDepth",&tremDepth.slider},{"drive",&drive.slider},
             {"lofi",&lofi.slider},{"bass",&bass.slider},{"treble",&treble.slider},
             {"swellAttack",&swellAttack.slider},{"swellAmount",&swellAmount.slider},
             {"slideAmount",&slideAmount.slider},{"slideSpeed",&slideSpeed.slider}};
    for (auto& k:km){k.s->getProperties().set("live",0);k.s->repaint();}
    arpBoxLive=false;
    if (idx<0||idx>=12) return;
    const char* ranked[]={L[idx].p1,L[idx].p2,L[idx].p3};
    for (int r=0;r<3;++r)
        for (auto& k:km)
            if (std::strcmp(k.id,ranked[r])==0){k.s->getProperties().set("live",r+1);k.s->repaint();}
    arpBoxLive=L[idx].arp; repaint();
}

// ── paint ─────────────────────────────────────────────────────────────────────
void GhostSurfEditor::paint(Graphics& g)
{
    const int W=getWidth(), H=getHeight();

    // Fond dégradé nuit bleue
    ColourGradient bg(Colour(0xFF060F1A),0,0, Colour(0xFF081522),0,H,false);
    g.setGradientFill(bg); g.fillAll();

    // Vagues sous-marines subtiles
    for (int i=0;i<3;++i){
        Path w; float wy=H*(0.18f+i*0.26f), amp=14.f+i*5.f;
        w.startNewSubPath(0,wy);
        for (int x=0;x<=W;x+=4)
            w.lineTo((float)x, wy+std::sin(x*0.016f+i*1.2f)*amp+std::sin(x*0.006f+i*0.5f)*amp*0.4f);
        w.lineTo((float)W,(float)H); w.lineTo(0,(float)H); w.closeSubPath();
        g.setColour(Colour(0xFF0A2040).withAlpha(0.05f+i*0.02f)); g.fillPath(w);
    }

    // Header
    g.setColour(Colour(0xCC04101C)); g.fillRect(0,0,W,62);
    g.setColour(C::sky.withAlpha(0.40f)); g.drawHorizontalLine(62,0,(float)W);

    // Titre griffé
    drawScratchedTitle(g,"GHOST SURF", 12.f, 15.f);

    // Panneaux hémisphériques
    auto panel=[&](Rectangle<int> r, const char* t, Colour ac){
        // Fond verre bleu
        ColourGradient fill(Colour(0xFF102030),(float)r.getX(),(float)r.getY(),
                            Colour(0xFF060F1C),(float)r.getX(),(float)r.getBottom(),false);
        g.setGradientFill(fill); g.fillRoundedRectangle(r.toFloat(),10.f);
        // Dome highlight (reflet hémisphère)
        ColourGradient hi(ac.withAlpha(0.10f),(float)r.getCentreX(),(float)r.getY(),
                          ac.withAlpha(0.00f),(float)r.getCentreX(),r.getY()+r.getHeight()*0.4f,false);
        g.setGradientFill(hi);
        g.fillRoundedRectangle(r.toFloat().withHeight(r.getHeight()*0.4f),10.f);
        // Bordure fine colorée
        g.setColour(ac.withAlpha(0.45f));
        g.drawRoundedRectangle(r.toFloat().reduced(0.5f),10.f,1.2f);
        // Liseré interne (profondeur)
        g.setColour(Colour(0x14FFFFFF));
        g.drawRoundedRectangle(r.toFloat().reduced(2.f),8.f,0.8f);
        // Titre section
        g.setColour(ac.brighter(0.15f));
        g.setFont(Font("Arial",9.f,Font::bold));
        g.drawText(t, r.withHeight(22), Justification::centredTop, false);
    };

    panel({8,  65,218,420}, "SPRING REVERB", C::sky);
    panel({233,65,162,420}, "TREMOLO",        C::aqua);
    panel({402,65,234,420}, "EFFETS",         C::cobalt);
    panel({643,65,110,205}, "GUITARE",        C::mint);
    panel({643,277,110,208},"NIVEAU",         C::sky.withAlpha(0.7f));
    panel({8, 492,744,108}, "ARPÉGIATEUR",   C::aqua);

    // Label pattern
    g.setColour(C::aqua.withAlpha(0.65f));
    g.setFont(Font("Arial",8.f,Font::bold));
    g.drawText("PATTERN :", 576,498,68,14,Justification::centredRight);

    // Encadrement arpPatternBox si live
    if (arpBoxLive && currentMode==2){
        auto ab=arpPatternBox.getBounds().toFloat();
        g.setColour(C::live1.withAlpha(0.20f)); g.drawRoundedRectangle(ab.expanded(5.f),7.f,5.f);
        g.setColour(C::live1.withAlpha(0.88f)); g.drawRoundedRectangle(ab.expanded(2.f),6.f,1.8f);
    }

    // Légende live (●●● dégradé)
    for (int r=1;r<=3;++r){
        Colour lc=r==1?C::live1:r==2?C::live2:C::live3;
        g.setColour(lc.withAlpha(0.85f));
        g.fillEllipse((float)(W-155+(r-1)*42),22.f,8.f,8.f);
    }
    g.setColour(C::dim); g.setFont(Font("Arial",8.f,Font::plain));
    g.drawText("live",W-120,20,42,12,Justification::centredLeft);
}

// ── resized ───────────────────────────────────────────────────────────────────
void GhostSurfEditor::resized()
{
    presetBox.setBounds(228,17,210,28);
    modeNormal.setBounds(450,17,90,28); modeSwell.setBounds(545,17,90,28); modeArpege.setBounds(640,17,90,28);

    const int KS=52,KY=130;
    placeKnob(reverbMix,  58, KY,KS); placeKnob(reverbDecay,117,KY,KS); placeKnob(reverbTone,176,KY,KS);
    waveDisplay.setBounds(14,198,206,278);

    placeKnob(tremSpeed,284,KY,KS); placeKnob(tremDepth,354,KY,KS);
    tremSyncBtn.setBounds(240,202,82,24); tremDivBox.setBounds(328,202,60,24);

    placeKnob(drive,449,KY,KS); placeKnob(lofi,508,KY,KS);
    placeKnob(bass,567,KY,KS); placeKnob(treble,626,KY,KS);

    bool sw=(currentMode==1), nm=(currentMode==0);
    swellAttack.slider.setVisible(sw); swellAttack.label.setVisible(sw);
    swellAmount.slider.setVisible(sw); swellAmount.label.setVisible(sw);
    slideAmount.slider.setVisible(nm); slideAmount.label.setVisible(nm);
    slideSpeed.slider.setVisible(nm);  slideSpeed.label.setVisible(nm);

    const int GX=698;
    placeKnob(swellAttack,GX,110,KS); placeKnob(swellAmount,GX,195,KS);
    placeKnob(slideAmount,GX,110,KS); placeKnob(slideSpeed, GX,195,KS);

    vuMeter.setBounds(649,285,98,195);
    arpPatternBox.setBounds(646,497,102,22); arpPatternBox.setVisible(true);
    arpEditor.setBounds(14,500,628,92);
}
