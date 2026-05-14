#include "PluginEditor.h"
using namespace juce;

// ── Palette ───────────────────────────────────────────────────────────────────
namespace C {
    const Colour bg    { 0xFF05101A };
    const Colour sky   { 0xFF5BC8FF };
    const Colour aqua  { 0xFF00D8C8 };
    const Colour cobalt{ 0xFF6AABFF };
    const Colour mint  { 0xFF44E8A8 };
    const Colour violet{ 0xFFAA77FF };  // Uni-Vibe
    const Colour freeze{ 0xFF88DDFF };  // Freeze
    const Colour ice   { 0xFFCCEEFF };
    const Colour dim   { 0xFF5080A0 };
    const Colour live1 { 0xFFFF3322 };
    const Colour live2 { 0xFFFF8800 };
    const Colour live3 { 0xFFFFCC00 };
}

static Colour accentOf(const Slider& s) {
    auto id=s.getComponentID();
    if(id=="aqua")   return C::aqua;
    if(id=="cobalt") return C::cobalt;
    if(id=="mint")   return C::mint;
    if(id=="violet") return C::violet;
    if(id=="freeze") return C::freeze;
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

void OceanLookAndFeel::drawRotarySlider(Graphics& g,int x,int y,int w,int h,
                                         float pos,float start,float end,Slider& sl)
{
    auto b=Rectangle<float>((float)x,(float)y,(float)w,(float)h);
    auto cc=b.getCentre();
    float r=jmin(b.getWidth(),b.getHeight())*0.42f;
    Colour ac=accentOf(sl);

    // ── Live highlight ring ─────────────────────────────────────────────────
    int rank=(int)sl.getProperties()["live"];
    if(rank>0){
        Colour lc=rank==1?C::live1:rank==2?C::live2:C::live3;
        g.setColour(lc.withAlpha(0.18f));
        g.drawEllipse(cc.x-r-8,cc.y-r-8,(r+8)*2,(r+8)*2,7.f);
        g.setColour(lc.withAlpha(0.88f));
        g.drawEllipse(cc.x-r-2,cc.y-r-2,(r+2)*2,(r+2)*2,1.8f);
    }

    // ── Overdrive glow (>90%) ───────────────────────────────────────────────
    if(pos>0.90f){
        float gp=(pos-0.90f)*10.f;
        g.setColour(Colour(0xFFFF6B00).withAlpha(0.12f*gp));
        g.fillEllipse(cc.x-r-10,cc.y-r-10,(r+10)*2,(r+10)*2);
        g.setColour(Colour(0xFFFF6B00).withAlpha(0.35f*gp));
        g.drawEllipse(cc.x-r-3,cc.y-r-3,(r+3)*2,(r+3)*2,2.5f);
    }

    // ── Arc piste ──────────────────────────────────────────────────────────
    float ar=r-4.f;
    { Path p; p.addArc(cc.x-ar,cc.y-ar,ar*2,ar*2,start,end,true);
      g.setColour(Colour(0xFF0D1E30)); g.strokePath(p,PathStrokeType(3.5f)); }

    // ── Arc valeur ─────────────────────────────────────────────────────────
    float va=start+pos*(end-start);
    if(pos>0.004f){
        Path p; p.addArc(cc.x-ar,cc.y-ar,ar*2,ar*2,start,va,true);
        g.setColour(ac);
        g.strokePath(p,PathStrokeType(3.5f,PathStrokeType::curved,PathStrokeType::rounded));
        auto ep=cc.getPointOnCircumference(ar,va);
        g.setColour(ac.withAlpha(0.40f)); g.fillEllipse(ep.x-5,ep.y-5,10,10);
        g.setColour(C::ice); g.fillEllipse(ep.x-2.5f,ep.y-2.5f,5,5);
    }

    // ── Corps verre hémisphère ──────────────────────────────────────────────
    float ir=r*0.68f;
    bool isHover=sl.isMouseOverOrDragging();
    bool isDrag =sl.isMouseButtonDown();
    // Légère élévation au survol / drag
    float lift = isDrag?0.f : isHover?0.5f : 0.f;
    g.setColour(Colour(0xFF030C16)); g.fillEllipse(cc.x-r,cc.y-r,r*2,r*2);
    Colour bodyTop = isDrag ? Colour(0xFF284870) : Colour(0xFF1E4060);
    ColourGradient body(bodyTop.brighter(lift*0.15f),cc.x-ir*0.3f,cc.y-ir,
                        Colour(0xFF050E1C),cc.x,cc.y+ir,false);
    g.setGradientFill(body); g.fillEllipse(cc.x-ir,cc.y-ir,ir*2,ir*2);
    // Highlight spéculaire — plus intense au survol
    uint8 hlAlpha = isDrag ? 0x48 : isHover ? 0x40 : 0x32;
    ColourGradient hl(Colour(hlAlpha,0xFF,0xFF,0xFF),cc.x-ir*0.2f,cc.y-ir*0.85f,
                      Colour(0x00,0xFF,0xFF,0xFF),cc.x,cc.y,true);
    g.setGradientFill(hl); g.fillEllipse(cc.x-ir*0.6f,cc.y-ir*0.95f,ir*1.2f,ir*0.9f);
    // Liseré accent : plus brillant si actif
    g.setColour(ac.withAlpha(isHover?0.38f:0.18f));
    g.drawEllipse(cc.x-ir,cc.y-ir,ir*2,ir*2,isHover?1.4f:1.f);

    // ── Indicateur ─────────────────────────────────────────────────────────
    auto dp=cc.getPointOnCircumference(ir*0.60f,va);
    g.setColour(C::ice); g.fillEllipse(dp.x-2.5f,dp.y-2.5f,5,5);

    // ── Valeur ─────────────────────────────────────────────────────────────
    g.setColour(C::dim); g.setFont(Font("Arial",8.f,Font::plain));
    double v=sl.getValue();
    g.drawText(v==(int)v?String((int)v):String(v,1),
               (int)(cc.x-20),(int)(cc.y+ir+3),40,12,Justification::centred);
}

void OceanLookAndFeel::drawComboBox(Graphics& g,int w,int h,bool,int bx,int by,int bw,int bh,ComboBox&)
{
    ColourGradient bg(Colour(0xFF0E2035),0,0,Colour(0xFF081520),0,(float)h,false);
    g.setGradientFill(bg); g.fillRoundedRectangle(0,0,w,h,5.f);
    g.setColour(C::sky.withAlpha(0.40f)); g.drawRoundedRectangle(0.5f,0.5f,w-1.f,h-1.f,5.f,1.f);
    Path arrow;
    auto a=Rectangle<int>(bx,by,bw,bh).toFloat();
    arrow.startNewSubPath(a.getX()+4,a.getCentreY()-2);
    arrow.lineTo(a.getCentreX(),a.getCentreY()+3);
    arrow.lineTo(a.getRight()-4,a.getCentreY()-2);
    g.setColour(C::sky); g.strokePath(arrow,PathStrokeType(1.5f));
}

void OceanLookAndFeel::drawButtonBackground(Graphics& g,Button& btn,const Colour&,bool hi,bool)
{
    auto b=btn.getLocalBounds().toFloat();
    bool on=btn.getToggleState();
    if(on){
        ColourGradient gr(C::sky.withAlpha(0.28f),b.getX(),b.getY(),
                          C::sky.withAlpha(0.08f),b.getX(),b.getBottom(),false);
        g.setGradientFill(gr); g.fillRoundedRectangle(b,7.f);
        g.setColour(C::sky.withAlpha(0.85f)); g.drawRoundedRectangle(b.reduced(0.5f),7.f,1.5f);
    } else {
        ColourGradient gr(Colour(0xFF0E2035),b.getX(),b.getY(),
                          Colour(0xFF081520),b.getX(),b.getBottom(),false);
        g.setGradientFill(gr); g.fillRoundedRectangle(b,7.f);
        g.setColour(hi?C::sky.withAlpha(0.35f):C::sky.withAlpha(0.20f));
        g.drawRoundedRectangle(b.reduced(0.5f),7.f,1.f);
    }
}

Font OceanLookAndFeel::getLabelFont(Label&) { return Font("Arial",10.f,Font::bold); }
void OceanLookAndFeel::drawLabel(Graphics& g,Label& l) {
    g.setColour(l.findColour(Label::textColourId));
    g.setFont(getLabelFont(l));
    g.drawText(l.getText(),l.getLocalBounds(),Justification::centred,false);
}

// ── Titre griffé ──────────────────────────────────────────────────────────────
static void drawScratchedTitle(Graphics& g,const String& text,float x,float y)
{
    Font f("Arial",28.f,Font::bold|Font::italic);
    GlyphArrangement ga; ga.addLineOfText(f,text,x,y+28.f);
    Path p; ga.createPath(p);
    g.setColour(Colour(0xFF000810)); g.fillPath(p,AffineTransform::translation(2.5f,3.f));
    g.setColour(C::sky.withAlpha(0.22f)); g.fillPath(p,AffineTransform::translation(0.f,1.f));
    ColourGradient grad(C::ice,x,y,C::sky,x,y+32.f,false);
    g.setGradientFill(grad); g.fillPath(p);
    g.setColour(Colour(0xFFFFFFFF).withAlpha(0.55f)); g.strokePath(p,PathStrokeType(0.55f));
    auto pb=p.getBounds();
    g.saveState(); g.reduceClipRegion(p);
    g.setColour(Colour(0xFFFFFFFF).withAlpha(0.07f));
    for(float sx=pb.getX()-8.f;sx<pb.getRight()+8.f;sx+=13.f)
        g.drawLine(sx+9.f,pb.getY()-2.f,sx,pb.getBottom()+2.f,1.5f);
    g.restoreState();
}

// ── Panel helper ──────────────────────────────────────────────────────────────
static void drawPanel(Graphics& g,Rectangle<int> r,const char* t,Colour ac)
{
    ColourGradient fill(Colour(0xFF0E1E2E),(float)r.getX(),(float)r.getY(),
                        Colour(0xFF060E1C),(float)r.getX(),(float)r.getBottom(),false);
    g.setGradientFill(fill); g.fillRoundedRectangle(r.toFloat(),10.f);
    ColourGradient hi(ac.withAlpha(0.09f),(float)r.getCentreX(),(float)r.getY(),
                      ac.withAlpha(0.00f),(float)r.getCentreX(),r.getY()+r.getHeight()*0.38f,false);
    g.setGradientFill(hi); g.fillRoundedRectangle(r.toFloat().withHeight(r.getHeight()*0.38f),10.f);
    g.setColour(ac.withAlpha(0.42f)); g.drawRoundedRectangle(r.toFloat().reduced(0.5f),10.f,1.2f);
    g.setColour(Colour(0x12FFFFFF)); g.drawRoundedRectangle(r.toFloat().reduced(2.f),8.f,0.8f);
    g.setColour(ac.brighter(0.1f)); g.setFont(Font("Arial",9.f,Font::bold));
    g.drawText(t,r.withHeight(22),Justification::centredTop,false);
}

// ── WaveformDisplay ───────────────────────────────────────────────────────────
WaveformDisplay::WaveformDisplay(GhostSurfProcessor& p):proc(p){startTimerHz(30);}

void WaveformDisplay::paint(Graphics& g)
{
    auto b=getLocalBounds().toFloat();
    const float BW=b.getWidth(),BH=b.getHeight(),x0=b.getX(),y0=b.getY();
    const float wAmp=15.f,wOff=wAmp+10.f;
    const int NP=200;

    auto wY=[&](float t){
        return y0+wOff-std::sin(t*MathConstants<float>::pi*4.8f)*wAmp*0.88f
                      -std::sin(t*MathConstants<float>::pi*2.0f)*wAmp*0.42f
                      -std::sin(t*MathConstants<float>::pi*9.2f)*wAmp*0.10f;
    };

    // Forme vague + fond
    Path shape;
    shape.startNewSubPath(x0,y0+BH); shape.lineTo(x0+BW,y0+BH);
    shape.lineTo(x0+BW,wY(1.f));
    for(int i=NP;i>=0;--i) shape.lineTo(x0+(float)i/NP*BW,wY((float)i/NP));
    shape.closeSubPath();
    ColourGradient bg(Colour(0xFF071525),x0,y0+wOff,Colour(0xFF050E1C),x0,y0+BH,false);
    g.setGradientFill(bg); g.fillPath(shape);

    // Oscilloscope clippé
    { Graphics::ScopedSaveState ss(g); g.reduceClipRegion(shape);
      float iT=y0+wOff,iH=BH-wOff,cy=iT+iH*0.5f;
      g.setColour(Colour(0x0C00AACC));
      for(int i=1;i<5;++i) g.drawHorizontalLine((int)(iT+iH*i/5.f),x0+4,x0+BW-4);
      g.setColour(Colour(0x2000BBDD)); g.drawHorizontalLine((int)cy,x0+4,x0+BW-4);
      const float* d=proc.getScopePtr(); int wp=proc.getScopeWritePos();
      const int N=GhostSurfProcessor::SCOPE_SIZE,PW=(int)BW-6;
      Path osc; bool st=false;
      for(int i=0;i<PW;++i){
          float s=jlimit(-1.f,1.f,d[(wp+i*N/PW)%N]);
          float px=x0+3.f+i,py=cy-s*iH*0.43f;
          if(!st){osc.startNewSubPath(px,py);st=true;} else osc.lineTo(px,py);
      }
      g.setColour(C::sky.withAlpha(0.12f)); g.strokePath(osc,PathStrokeType(6.f,PathStrokeType::curved));
      g.setColour(C::sky.withAlpha(0.42f)); g.strokePath(osc,PathStrokeType(2.5f,PathStrokeType::curved));
      g.setColour(C::ice.withAlpha(0.88f)); g.strokePath(osc,PathStrokeType(1.f,PathStrokeType::curved));
    }

    // Crête de vague
    Path crest;
    for(int i=0;i<=NP;++i){float t=(float)i/NP,wx=x0+t*BW,wy=wY(t);
        if(i==0)crest.startNewSubPath(wx,wy); else crest.lineTo(wx,wy);}
    g.setColour(C::sky.withAlpha(0.14f)); g.strokePath(crest,PathStrokeType(9.f,PathStrokeType::curved));
    g.setColour(C::sky.withAlpha(0.45f)); g.strokePath(crest,PathStrokeType(3.5f,PathStrokeType::curved));
    g.setColour(C::ice.withAlpha(0.82f)); g.strokePath(crest,PathStrokeType(1.2f,PathStrokeType::curved));
}

// ── SpecterPad ────────────────────────────────────────────────────────────────
SpecterPad::SpecterPad(GhostSurfProcessor& p):proc(p){
    startTimerHz(60);
    setMouseCursor(MouseCursor::CrosshairCursor);
}

void SpecterPad::spawnParticles(float x,float y,Colour c,int n)
{
    for(int i=0;i<n;++i){
        float angle=juce::Random::getSystemRandom().nextFloat()*MathConstants<float>::twoPi;
        float speed=1.f+juce::Random::getSystemRandom().nextFloat()*2.5f;
        float life=0.4f+juce::Random::getSystemRandom().nextFloat()*0.5f;
        particles.push_back({x,y,std::cos(angle)*speed,std::sin(angle)*speed,life,life,
            1.5f+juce::Random::getSystemRandom().nextFloat()*2.5f,c});
    }
    // Garder max 120 particules
    while((int)particles.size()>120) particles.erase(particles.begin());
}

void SpecterPad::timerCallback()
{
    bool dirty=false;
    for(auto& p:particles){
        p.x+=p.vx; p.y+=p.vy;
        p.vy+=0.08f; // gravité légère
        p.life-=1.f/60.f;
        dirty=true;
    }
    particles.erase(std::remove_if(particles.begin(),particles.end(),
        [](const Particle& p){return p.life<=0.f;}),particles.end());
    if(dirty||dragging) repaint();
}

void SpecterPad::drawFilterCurve(Graphics& g,Rectangle<float> area,int shape)
{
    // Dessiner la courbe de réponse du filtre (approximation visuelle)
    float W=area.getWidth(),H=area.getHeight();
    float x0=area.getX(),y0=area.getY();
    float fc=curX; // 0-1 = fréquence cutoff normalisée
    float q =1.f-curY; // 0-1 = résonance normalisée

    Path curve; bool started=false;
    const int NPTS=200;
    for(int i=0;i<NPTS;++i){
        float t=(float)i/NPTS; // fréquence normalisée log
        float logF=t; // 0=20Hz 1=20kHz (linéaire ici pour simplifier)
        float resp=0.f;

        switch(shape){
        case 0: { // SINE - lowpass
            float dist=logF-fc;
            float peak=q*0.8f*std::exp(-dist*dist*8.f);
            resp=(logF<fc ? 1.f : std::exp(-(logF-fc)*8.f))+peak;
            break; }
        case 1: { // SQUARE - bandpass
            float bw=0.05f+q*0.12f;
            float dist=std::abs(logF-fc);
            resp=std::exp(-dist*dist/(bw*bw))*( 0.5f+q*0.8f);
            break; }
        case 2: { // SAW - highpass
            float dist=logF-fc;
            float peak=q*0.8f*std::exp(-dist*dist*8.f);
            resp=(logF>fc ? 1.f : std::exp((logF-fc)*8.f))+peak;
            break; }
        case 3: { // CHAOS - lowpass + random notches
            float dist=logF-fc;
            resp=(logF<fc?1.f:std::exp(-(logF-fc)*8.f));
            float notch=0.4f*std::sin(logF*42.f+fc*10.f)*std::sin(logF*17.f);
            resp=jmax(0.f,resp+notch*q);
            break; }
        }
        resp=jlimit(0.f,1.4f,resp);
        float px=x0+t*W, py=y0+H-resp*H*0.75f;
        if(!started){curve.startNewSubPath(px,py);started=true;} else curve.lineTo(px,py);
    }
    g.setColour(C::sky.withAlpha(0.08f)); g.strokePath(curve,PathStrokeType(8.f,PathStrokeType::curved));
    g.setColour(C::aqua.withAlpha(0.55f)); g.strokePath(curve,PathStrokeType(2.f,PathStrokeType::curved));
    g.setColour(C::ice.withAlpha(0.85f)); g.strokePath(curve,PathStrokeType(0.8f,PathStrokeType::curved));
}

void SpecterPad::paint(Graphics& g)
{
    auto b=getLocalBounds().toFloat();
    float W=b.getWidth(),H=b.getHeight();

    // Fond
    ColourGradient bg(Colour(0xFF08182A),b.getX(),b.getY(),
                      Colour(0xFF04101C),b.getX(),b.getBottom(),false);
    g.setGradientFill(bg); g.fillRoundedRectangle(b,8.f);

    // Grille fréquentielle
    g.setColour(Colour(0x0800BBDD));
    for(int i=1;i<8;++i) g.drawVerticalLine((int)(b.getX()+W*i/8.f),(float)b.getY()+5,(float)b.getBottom()-5);
    for(int i=1;i<4;++i) g.drawHorizontalLine((int)(b.getY()+H*i/4.f),(float)b.getX()+5,(float)b.getRight()-5);

    // Étiquettes fréquences
    static const char* freqLabels[]={"100","500","1k","2k","5k","10k","16k"};
    g.setColour(C::dim.withAlpha(0.5f)); g.setFont(Font("Arial",7.f,Font::plain));
    for(int i=0;i<7;++i)
        g.drawText(freqLabels[i],(int)(b.getX()+W*(i+1)/8.f)-12,(int)(b.getBottom()-14),24,11,Justification::centred);

    // Courbe de filtre
    int shape=proc.getSpecterShape();
    drawFilterCurve(g,b.reduced(4,20),shape);

    // Particules
    for(auto& p:particles){
        float alpha=jmax(0.f,p.life/p.maxLife);
        g.setColour(p.col.withAlpha(alpha*0.85f));
        g.fillEllipse(p.x-p.size*0.5f,p.y-p.size*0.5f,p.size,p.size);
    }

    // Curseur (croix + halo)
    float cx=b.getX()+curX*W, cy=b.getY()+curY*H;
    g.setColour(C::sky.withAlpha(0.12f));
    g.drawVerticalLine((int)cx,(float)b.getY(),(float)b.getBottom());
    g.drawHorizontalLine((int)cy,(float)b.getX(),(float)b.getRight());
    // Halo du curseur
    ColourGradient glow(C::aqua.withAlpha(0.50f),cx,cy,C::aqua.withAlpha(0.f),cx+18,cy,true);
    g.setGradientFill(glow); g.fillEllipse(cx-18,cy-18,36,36);
    g.setColour(C::ice); g.fillEllipse(cx-4,cy-4,8,8);
    g.setColour(C::aqua.withAlpha(0.85f)); g.drawEllipse(cx-5,cy-5,10,10,1.5f);

    // Boutons de forme (SINE / SQ / SAW / CHAOS)
    static const char* shapes[]={"SINE","SQ","SAW","CHAOS"};
    static const Colour shapeCols[]={C::sky,C::cobalt,C::mint,C::violet};
    for(int i=0;i<4;++i){
        float bx=b.getX()+4.f+i*58.f, by=b.getY()+4.f;
        bool active=(i==shape);
        g.setColour(active?shapeCols[i].withAlpha(0.25f):Colour(0x18FFFFFF));
        g.fillRoundedRectangle(bx,by,52,18,4.f);
        g.setColour(active?shapeCols[i]:C::dim);
        g.drawRoundedRectangle(bx,by,52,18,4.f,1.f);
        g.setFont(Font("Arial",8.f,Font::bold));
        g.drawText(shapes[i],(int)bx,(int)by,52,18,Justification::centred);
    }

    // Étiquette cutoff + résonance
    g.setColour(C::dim); g.setFont(Font("Arial",8.f,Font::plain));
    float fc=proc.getSpecterCutoff();
    String fcStr=fc<1000.f?String((int)fc)+"Hz":String(fc/1000.f,1)+"kHz";
    g.drawText("fc="+fcStr,(int)(b.getRight()-80),(int)b.getY()+4,76,12,Justification::centredRight);
    g.drawText("Q="+String(proc.getSpecterReso(),2),(int)(b.getRight()-80),(int)b.getY()+14,76,12,Justification::centredRight);

    // Bordure
    g.setColour(C::aqua.withAlpha(0.35f)); g.drawRoundedRectangle(b.reduced(0.5f),8.f,1.2f);
}

void SpecterPad::mouseDown(const MouseEvent& e)
{
    auto b=getLocalBounds().toFloat();
    // Boutons forme en haut (zone y < 26px depuis le bord du component)
    if(e.y < 26){
        dragging=false;
        int shp=jlimit(0,3,(int)((e.x-4.f)/58.f));
        proc.setSpecterShape(shp);
        static const Colour sc[]={C::sky,C::cobalt,C::mint,C::violet};
        spawnParticles((float)e.x,(float)e.y,sc[shp],10);
        return;
    }
    dragging=true;
    curX=jlimit(0.f,1.f,(e.x-b.getX())/b.getWidth());
    curY=jlimit(0.f,1.f,(e.y-b.getY())/b.getHeight());
    float fc=20.f*std::pow(1000.f,curX);   // log scale 20Hz→20kHz
    proc.setSpecterCutoff(fc);
    proc.setSpecterReso(1.f-curY);
    spawnParticles((float)e.x,(float)e.y,C::aqua,8);
}

void SpecterPad::mouseDrag(const MouseEvent& e)
{
    auto b=getLocalBounds().toFloat();
    // Ignorer la zone des boutons en drag
    if(!dragging) return;
    curX=jlimit(0.f,1.f,(e.x-b.getX())/b.getWidth());
    curY=jlimit(0.f,1.f,(e.y-b.getY())/b.getHeight());
    float fc=20.f*std::pow(1000.f,curX);
    proc.setSpecterCutoff(fc);
    proc.setSpecterReso(1.f-curY);
    // Particules seulement toutes les 3 frames pour ne pas peser
    if(juce::Random::getSystemRandom().nextInt(3)==0)
        spawnParticles((float)e.x,(float)e.y,C::aqua.withAlpha(0.55f),2);
    // Pas de repaint() ici — le timer 60Hz s'en charge
}

void SpecterPad::mouseMove(const MouseEvent& e)
{
    // Mise à jour de la position du curseur (preview hover)
    // Le timer 60Hz gère le repaint — pas besoin d'appeler repaint() ici
    auto b=getLocalBounds().toFloat();
    curX=jlimit(0.f,1.f,(e.x-b.getX())/b.getWidth());
    curY=jlimit(0.f,1.f,(e.y-b.getY())/b.getHeight());
}

void SpecterPad::mouseUp(const MouseEvent&) { dragging=false; }

// ── FreezePanel ───────────────────────────────────────────────────────────────
FreezePanel::FreezePanel(GhostSurfProcessor& p):proc(p){startTimerHz(30);}

void FreezePanel::timerCallback()
{
    bool active=proc.isFreezing();
    if(active){
        pulse+=0.08f;
        if(pulse>MathConstants<float>::twoPi) pulse-=MathConstants<float>::twoPi;
        glowAnim=jmin(1.f,glowAnim+0.06f);
    } else {
        glowAnim=jmax(0.f,glowAnim-0.06f);
    }
    if(active!=wasActive){wasActive=active;repaint();}
    if(active||glowAnim>0.01f) repaint();
}

void FreezePanel::paint(Graphics& g)
{
    auto b=getLocalBounds().toFloat();
    bool active=proc.isFreezing();

    // Fond
    ColourGradient bg(Colour(0xFF081828),b.getX(),b.getY(),
                      Colour(0xFF04101C),b.getX(),b.getBottom(),false);
    g.setGradientFill(bg); g.fillRoundedRectangle(b,8.f);

    // Bouton hexagonal centré
    float cx=b.getCentreX(), cy=b.getY()+b.getHeight()*0.42f;
    float rad=jmin(b.getWidth(),b.getHeight())*0.28f;

    // Glow externe (quand actif)
    if(glowAnim>0.01f){
        float pulseR=rad+6.f+std::sin(pulse)*4.f;
        ColourGradient gl(C::freeze.withAlpha(0.20f*glowAnim),cx,cy,
                          C::freeze.withAlpha(0.f),cx+pulseR,cy,true);
        g.setGradientFill(gl); g.fillEllipse(cx-pulseR,cy-pulseR,pulseR*2,pulseR*2);
    }

    // Hexagone
    Path hex;
    for(int i=0;i<6;++i){
        float a=MathConstants<float>::pi/6.f+i*MathConstants<float>::pi/3.f;
        float hx=cx+std::cos(a)*rad, hy=cy+std::sin(a)*rad;
        if(i==0)hex.startNewSubPath(hx,hy); else hex.lineTo(hx,hy);
    }
    hex.closeSubPath();

    if(active){
        ColourGradient fill(C::freeze.withAlpha(0.35f),cx,cy-rad,
                            C::freeze.withAlpha(0.10f),cx,cy+rad,false);
        g.setGradientFill(fill); g.fillPath(hex);
        g.setColour(C::freeze.withAlpha(0.9f)); g.strokePath(hex,PathStrokeType(2.5f));
        // Cristaux (lignes de glace)
        g.setColour(C::ice.withAlpha(0.25f));
        for(int i=0;i<6;++i){
            float a=i*MathConstants<float>::pi/3.f+pulse*0.1f;
            g.drawLine(cx,cy,cx+std::cos(a)*rad*0.8f,cy+std::sin(a)*rad*0.8f,1.f);
        }
    } else {
        ColourGradient fill(Colour(0xFF152535),cx,cy-rad,Colour(0xFF0A1820),cx,cy+rad,false);
        g.setGradientFill(fill); g.fillPath(hex);
        g.setColour(C::freeze.withAlpha(0.35f+glowAnim*0.4f));
        g.strokePath(hex,PathStrokeType(1.5f));
    }

    // Icône ❄ ou HOLD
    g.setFont(Font("Arial",active?9.f:10.f,Font::bold));
    g.setColour(active?C::ice:C::freeze.withAlpha(0.70f));
    g.drawText(active?"HOLD":"FREEZE",(int)(cx-30),(int)(cy-8),60,16,Justification::centred);

    // État shimmer / grain si actif
    if(active){
        float sh=proc.getAPVTS().getRawParameterValue("freezeShimmer")->load();
        if(sh>0.05f){
            g.setColour(C::violet.withAlpha(0.6f));
            g.setFont(Font("Arial",7.5f,Font::plain));
            g.drawText("✦ shimmer",(int)b.getX(),(int)(cy+rad+6),(int)b.getWidth(),12,Justification::centred);
        }
    }

    // Label
    g.setColour(C::freeze.withAlpha(0.5f));
    g.setFont(Font("Arial",8.f,Font::bold));
    g.drawText("FREEZE",b.removeFromBottom(18.f),Justification::centred);
    g.setColour(C::freeze.withAlpha(0.30f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f),8.f,1.2f);
}

void FreezePanel::mouseDown(const MouseEvent& e)
{
    auto b=getLocalBounds().toFloat();
    float cx=b.getCentreX(), cy=b.getY()+b.getHeight()*0.42f;
    float rad=jmin(b.getWidth(),b.getHeight())*0.28f;
    // Zone cliquable généreuse : hexagone + 14px de marge
    if(e.getPosition().toFloat().getDistanceFrom({cx,cy}) < rad+14.f)
        proc.setFreezeActive(!proc.isFreezing());
}
void FreezePanel::mouseEnter(const MouseEvent&) { setMouseCursor(MouseCursor::PointingHandCursor); }
void FreezePanel::mouseExit (const MouseEvent&) { setMouseCursor(MouseCursor::NormalCursor); }

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
    for(int i=0;i<S;++i){
        auto seg=Rectangle<float>(b.getX()+5,b.getBottom()-14.f-(i+1)*sH,b.getWidth()-10,sH-1.5f);
        Colour sc=i<lit?(i>=S-2?Colour(0xFFFF2244):i>=S-5?Colour(0xFFFFAA00):C::aqua):Colour(0xFF0A1A28);
        g.setColour(sc); g.fillRoundedRectangle(seg,1.5f);
    }
    g.setColour(C::dim); g.setFont(8.f);
    g.drawText("dB",b.removeFromBottom(14.f),Justification::centred);
}

// ── Knob builder ─────────────────────────────────────────────────────────────
void GhostSurfEditor::buildKnob(KnobWidget& kw,const char* id,const char* lbl,Colour ac)
{
    // Drag horizontal OU vertical — beaucoup plus naturel
    kw.slider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    kw.slider.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    // 250 pixels pour la plage complète = précision confortable
    kw.slider.setMouseDragSensitivity(250);
    // Double-clic = retour à la valeur par défaut
    if(auto* param=proc.getAPVTS().getParameter(id)){
        double def=param->convertFrom0to1(param->getDefaultValue());
        kw.slider.setDoubleClickReturnValue(true,def);
    }
    kw.slider.setScrollWheelEnabled(true);
    kw.slider.setLookAndFeel(&lf);
    if     (ac==C::aqua)   kw.slider.setComponentID("aqua");
    else if(ac==C::cobalt) kw.slider.setComponentID("cobalt");
    else if(ac==C::mint)   kw.slider.setComponentID("mint");
    else if(ac==C::violet) kw.slider.setComponentID("violet");
    else if(ac==C::freeze) kw.slider.setComponentID("freeze");
    else                   kw.slider.setComponentID("sky");
    kw.slider.setMouseCursor(MouseCursor::UpDownLeftRightResizeCursor);
    addAndMakeVisible(kw.slider);
    kw.label.setText(lbl,dontSendNotification);
    kw.label.setFont(Font("Arial",9.f,Font::bold));
    kw.label.setColour(Label::textColourId,ac);
    kw.label.setJustificationType(Justification::centred);
    addAndMakeVisible(kw.label);
    kw.attach=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(proc.getAPVTS(),id,kw.slider);
}
void GhostSurfEditor::placeKnob(KnobWidget& kw,int cx,int cy,int sz)
{
    kw.slider.setBounds(cx-sz/2,cy-sz/2,sz,sz);
    kw.label.setBounds(cx-28,cy+sz/2+2,56,13);
}

// ── Constructeur ─────────────────────────────────────────────────────────────
GhostSurfEditor::GhostSurfEditor(GhostSurfProcessor& p)
    : AudioProcessorEditor(&p),proc(p),vuMeter(p),waveDisplay(p),specterPad(p),freezePanel(p)
{
    setSize(760,650); setLookAndFeel(&lf);

    // Preset box
    for(int i=0;i<GhostSurfProcessor::NUM_PRESETS;++i)
        presetBox.addItem(p.getProgramName(i),i+1);
    presetBox.setSelectedId(p.getCurrentProgram()+1,dontSendNotification);
    presetBox.onChange=[&]{
        int i=presetBox.getSelectedId()-1;
        proc.setCurrentProgram(i);
        updateLiveHighlights(i);
    };
    addAndMakeVisible(presetBox);

    // Tremolo sync/div
    tremSyncBtn.setButtonText("SYNC"); tremSyncBtn.setLookAndFeel(&lf); addAndMakeVisible(tremSyncBtn);
    tremSyncAttach=std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(proc.getAPVTS(),"tremSync",tremSyncBtn);
    tremDivBox.addItem("1/4",1); tremDivBox.addItem("1/8",2); tremDivBox.addItem("1/16",3);
    addAndMakeVisible(tremDivBox);
    tremDivAttach=std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(proc.getAPVTS(),"tremDiv",tremDivBox);

    // Vibe mode
    vibeModeBtn.setButtonText("VIBRATO"); vibeModeBtn.setLookAndFeel(&lf); addAndMakeVisible(vibeModeBtn);
    vibeModeAttach=std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(proc.getAPVTS(),"vibeMode",vibeModeBtn);

    // Knobs
    buildKnob(reverbMix,   "reverbMix",   "MIX",     C::sky);
    buildKnob(reverbDecay, "reverbDecay", "DECAY",    C::sky);
    buildKnob(reverbTone,  "reverbTone",  "TONE",     C::sky);
    buildKnob(tremSpeed,   "tremSpeed",   "VITESSE",  C::aqua);
    buildKnob(tremDepth,   "tremDepth",   "PROFOND",  C::aqua);
    buildKnob(drive,       "drive",       "DRIVE",    C::cobalt);
    buildKnob(lofi,        "lofi",        "LO-FI",    C::cobalt);
    buildKnob(bass,        "bass",        "BASSES",   C::cobalt);
    buildKnob(treble,      "treble",      "AIGUS",    C::cobalt);
    buildKnob(slideAmount, "slideAmount", "GLISS",    C::mint);
    buildKnob(slideSpeed,  "slideSpeed",  "SPEED",    C::mint);
    buildKnob(vibeSpeed,   "vibeSpeed",   "SPEED",    C::violet);
    buildKnob(vibeDepth,   "vibeDepth",   "DEPTH",    C::violet);
    buildKnob(freezeGrain,   "freezeGrain",   "GRAIN",   C::freeze);
    buildKnob(freezeShimmer, "freezeShimmer", "SHIMMER", C::freeze);
    buildKnob(freezeDecay,   "freezeDecay",   "WET",     C::freeze);

    addAndMakeVisible(vuMeter); addAndMakeVisible(waveDisplay);
    addAndMakeVisible(specterPad); addAndMakeVisible(freezePanel);

    updateLiveHighlights(p.getCurrentProgram());
    startTimerHz(20);
}
GhostSurfEditor::~GhostSurfEditor(){setLookAndFeel(nullptr);}

void GhostSurfEditor::timerCallback()
{
    // Surf score animation
    float target=proc.getSurfScore();
    scoreAnim+=(target-scoreAnim)*0.18f;
    int combo=proc.getSurfCombo();
    if(combo!=comboFlash){comboFlash=combo;}
    repaint();
}

// ── updateLiveHighlights ─────────────────────────────────────────────────────
void GhostSurfEditor::updateLiveHighlights(int idx)
{
    struct Def { const char* p1,*p2,*p3; };
    static const Def L[12]={
        {"reverbMix",   "reverbDecay",  "reverbTone"},   // The Cure
        {"lofi",        "freezeDecay",  "reverbMix"},    // Lil Peep
        {"drive",       "tremSpeed",    "tremDepth"},    // Iggy Pop
        {"tremSpeed",   "reverbTone",   "reverbMix"},    // Surf Clean
        {"freezeDecay", "freezeShimmer","reverbDecay"},  // Night Waves
        {"tremSpeed",   "tremDepth",    "drive"},         // Dick Dale
        {"vibeDepth",   "vibeSpeed",    "reverbDecay"},  // The Pixies
        {"reverbDecay", "reverbTone",   "reverbMix"},    // Joy Division
        {"slideAmount", "drive",        "bass"},          // Jack White
        {"freezeShimmer","reverbDecay", "vibeDepth"},    // Haunted Motel
        {"drive",       "vibeDepth",    "treble"},        // Jimi Hendrix
        {"treble",      "bass",         "lofi"},          // Nile Rodgers
    };
    struct KM{const char* id;Slider* s;};
    KM km[]={
        {"reverbMix",&reverbMix.slider},{"reverbDecay",&reverbDecay.slider},
        {"reverbTone",&reverbTone.slider},{"tremSpeed",&tremSpeed.slider},
        {"tremDepth",&tremDepth.slider},{"drive",&drive.slider},
        {"lofi",&lofi.slider},{"bass",&bass.slider},{"treble",&treble.slider},
        {"slideAmount",&slideAmount.slider},{"slideSpeed",&slideSpeed.slider},
        {"vibeSpeed",&vibeSpeed.slider},{"vibeDepth",&vibeDepth.slider},
        {"freezeGrain",&freezeGrain.slider},{"freezeShimmer",&freezeShimmer.slider},
        {"freezeDecay",&freezeDecay.slider}
    };
    for(auto& k:km){k.s->getProperties().set("live",0);k.s->repaint();}
    if(idx<0||idx>=12)return;
    const char* ranked[]={L[idx].p1,L[idx].p2,L[idx].p3};
    for(int r=0;r<3;++r)
        for(auto& k:km)
            if(std::strcmp(k.id,ranked[r])==0){k.s->getProperties().set("live",r+1);k.s->repaint();}
}

// ── paint ─────────────────────────────────────────────────────────────────────
void GhostSurfEditor::paint(Graphics& g)
{
    const int W=getWidth(),H=getHeight();

    // Fond dégradé
    ColourGradient bg(Colour(0xFF050F19),0,0,Colour(0xFF07141F),0,H,false);
    g.setGradientFill(bg); g.fillAll();

    // Vagues de fond
    for(int i=0;i<3;++i){
        Path w; float wy=H*(0.18f+i*0.26f),amp=14.f+i*5.f;
        w.startNewSubPath(0,wy);
        for(int x=0;x<=W;x+=4)
            w.lineTo((float)x,wy+std::sin(x*0.016f+i*1.2f)*amp+std::sin(x*0.006f+i*0.5f)*amp*0.4f);
        w.lineTo((float)W,(float)H); w.lineTo(0,(float)H); w.closeSubPath();
        g.setColour(Colour(0xFF0A2040).withAlpha(0.05f+i*0.02f)); g.fillPath(w);
    }

    // ── Header ────────────────────────────────────────────────────────────
    g.setColour(Colour(0xCC04101C)); g.fillRect(0,0,W,62);
    g.setColour(C::sky.withAlpha(0.38f)); g.drawHorizontalLine(62,0,(float)W);
    drawScratchedTitle(g,"GHOST SURF",12.f,14.f);

    // ── SURF SCORE bar ────────────────────────────────────────────────────
    {
        float barX=228.f,barY=18.f,barW=180.f,barH=12.f;
        // Fond barre
        g.setColour(Colour(0xFF0A1E30)); g.fillRoundedRectangle(barX,barY,barW,barH,5.f);
        // Remplissage score
        float fill=jlimit(0.f,1.f,scoreAnim/100.f);
        Colour fillCol=fill>0.8f?Colour(0xFFFFDD00):fill>0.5f?C::aqua:C::sky;
        ColourGradient barG(fillCol.withAlpha(0.9f),barX,barY,
                            fillCol.withAlpha(0.5f),barX+barW*fill,barY,false);
        g.setGradientFill(barG); g.fillRoundedRectangle(barX,barY,barW*fill,barH,5.f);
        g.setColour(C::sky.withAlpha(0.30f)); g.drawRoundedRectangle(barX,barY,barW,barH,5.f,1.f);
        // Score texte
        g.setColour(C::ice); g.setFont(Font("Arial",8.f,Font::bold));
        g.drawText("SURF SCORE",(int)barX,(int)(barY+barH+2),(int)barW,10,Justification::centredLeft);
        // Combo badge
        if(comboFlash>1){
            float bx=barX+barW+6; float by=barY-2;
            Colour cc=comboFlash>=6?Colour(0xFFFFDD00):comboFlash>=3?C::aqua:C::sky;
            g.setColour(cc.withAlpha(0.25f)); g.fillRoundedRectangle(bx,by,40,18,6.f);
            g.setColour(cc); g.drawRoundedRectangle(bx,by,40,18,6.f,1.2f);
            g.setFont(Font("Arial",8.f,Font::bold));
            g.drawText("x"+String(comboFlash),(int)bx,(int)by,40,18,Justification::centred);
        }
    }

    // ── Légende live ──────────────────────────────────────────────────────
    for(int r=1;r<=3;++r){
        Colour lc=r==1?C::live1:r==2?C::live2:C::live3;
        g.setColour(lc.withAlpha(0.85f)); g.fillEllipse((float)(W-155+(r-1)*42),22.f,8.f,8.f);
    }
    g.setColour(C::dim); g.setFont(Font("Arial",8.f,Font::plain));
    g.drawText("live",W-120,20,42,12,Justification::centredLeft);

    // ── Panneaux ──────────────────────────────────────────────────────────
    drawPanel(g,{8,  65,215,310},"SPRING REVERB", C::sky);
    drawPanel(g,{231,65,162,310},"TREMOLO",        C::aqua);
    drawPanel(g,{401,65,234,310},"EFFETS",         C::cobalt);
    drawPanel(g,{643,65, 110,155},"SLIDE",   C::mint);
    drawPanel(g,{643,228,110,147},"UNI-VIBE",C::violet);
    drawPanel(g,{643,383,110,257},"NIVEAU",  C::sky.withAlpha(0.7f));
    // Panneau bas : Specter + Freeze
    drawPanel(g,{8,383,627,257},"SPECTER  /  ❄ FREEZE",C::aqua);

    // ── Sync vibe wheel (dessiné sur le panneau VIBE) ─────────────────────
    {
        float spd=proc.getAPVTS().getRawParameterValue("vibeSpeed")->load();
        float dep=proc.getAPVTS().getRawParameterValue("vibeDepth")->load();
        static float wheelAngle=0.f;
        wheelAngle+=0.04f*spd*dep;
        if(dep>0.01f){
            float wCx=698.f, wCy=295.f, wR=20.f;
            g.setColour(C::violet.withAlpha(0.10f+dep*0.15f));
            g.drawEllipse(wCx-wR,wCy-wR,wR*2,wR*2,dep>0.5f?3.f:2.f);
            // Spoke tournant
            g.setColour(C::violet.withAlpha(0.6f));
            g.drawLine(wCx,wCy,wCx+std::cos(wheelAngle)*wR*0.8f,
                       wCy+std::sin(wheelAngle)*wR*0.8f,2.f);
            g.drawLine(wCx,wCy,wCx+std::cos(wheelAngle+MathConstants<float>::pi)*wR*0.55f,
                       wCy+std::sin(wheelAngle+MathConstants<float>::pi)*wR*0.55f,1.f);
        }
    }
}

// ── resized ───────────────────────────────────────────────────────────────────
void GhostSurfEditor::resized()
{
    presetBox.setBounds(450,17,202,28);
    tremSyncBtn.setBounds(240,197,62,22); tremDivBox.setBounds(307,197,80,22);
    vibeModeBtn.setBounds(649,344,106,22);

    const int KS=52,KY=125;
    // Spring Reverb
    placeKnob(reverbMix,  57, KY,KS);
    placeKnob(reverbDecay,117,KY,KS);
    placeKnob(reverbTone, 177,KY,KS);
    waveDisplay.setBounds(14,193,206,175);

    // Tremolo
    placeKnob(tremSpeed,282,KY,KS);
    placeKnob(tremDepth,352,KY,KS);

    // Effets
    placeKnob(drive,  449,KY,KS);
    placeKnob(lofi,   509,KY,KS);
    placeKnob(bass,   569,KY,KS);
    placeKnob(treble, 629,KY,KS);

    // Slide
    placeKnob(slideAmount,698,100,KS);
    placeKnob(slideSpeed, 698,175,KS);

    // Uni-Vibe
    placeKnob(vibeSpeed,698,260,KS);
    placeKnob(vibeDepth,698,335,KS);

    // Specter Pad — occupe la gauche du panneau bas
    specterPad.setBounds(14,400,410,230);

    // Freeze Panel — bouton central
    freezePanel.setBounds(432,400,114,230);

    // Freeze knobs — colonne droite du panneau bas
    placeKnob(freezeGrain,  567,430,44);
    placeKnob(freezeShimmer,567,500,44);
    placeKnob(freezeDecay,  567,572,44);

    // VU Meter — colonne de droite
    vuMeter.setBounds(649,395,104,237);
}
