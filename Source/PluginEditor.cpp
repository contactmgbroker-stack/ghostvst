#include "PluginEditor.h"
using namespace juce;

// ── Palette ───────────────────────────────────────────────────────────────────
namespace C {
    const Colour bg    { 0xFF04090F };
    const Colour sky   { 0xFF4DB8FF };
    const Colour aqua  { 0xFF00D4C0 };
    const Colour cobalt{ 0xFF5A9FFF };
    const Colour mint  { 0xFF3ADEA0 };
    const Colour violet{ 0xFF9966FF };
    const Colour freeze{ 0xFF77CCFF };
    const Colour ice   { 0xFFCCEEFF };
    const Colour dim   { 0xFF4070A0 };
    const Colour ledOn { 0xFF00FF88 };
    const Colour ledOff{ 0xFF883333 };
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
    setColour(ComboBox::backgroundColourId,  Colour(0xFF0A1825));
    setColour(ComboBox::textColourId,        C::ice);
    setColour(ComboBox::arrowColourId,       C::sky);
    setColour(ComboBox::outlineColourId,     C::sky.withAlpha(0.30f));
    setColour(PopupMenu::backgroundColourId, Colour(0xFF0A1825));
    setColour(PopupMenu::textColourId,       C::ice);
    setColour(PopupMenu::highlightedBackgroundColourId, C::sky.withAlpha(0.18f));
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

    // Live highlight ring — random color stored per slider
    int rank=(int)sl.getProperties()["live"];
    if(rank>0){
        var lv=sl.getProperties()["liveColor"];
        Colour lc=lv.isVoid() ? C::sky : Colour((uint32)(int64)lv);
        float pulse=0.75f+0.25f*std::sin((float)Time::getMillisecondCounter()*0.004f);
        g.setColour(lc.withAlpha(0.14f*pulse));
        g.fillEllipse(cc.x-r-10,cc.y-r-10,(r+10)*2,(r+10)*2);
        g.setColour(lc.withAlpha(0.92f));
        g.drawEllipse(cc.x-r-2,cc.y-r-2,(r+2)*2,(r+2)*2,rank==1?2.5f:1.8f);
    }

    // Overdrive glow >88%
    if(pos>0.88f){
        float gp=(pos-0.88f)*8.33f;
        g.setColour(Colour(0xFFFF5500).withAlpha(0.15f*gp));
        g.fillEllipse(cc.x-r-12,cc.y-r-12,(r+12)*2,(r+12)*2);
        g.setColour(Colour(0xFFFF7700).withAlpha(0.50f*gp));
        g.drawEllipse(cc.x-r-4,cc.y-r-4,(r+4)*2,(r+4)*2,2.f);
    }

    // Track arc
    float ar=r-4.f;
    { Path p; p.addArc(cc.x-ar,cc.y-ar,ar*2,ar*2,start,end,true);
      g.setColour(Colour(0xFF081520)); g.strokePath(p,PathStrokeType(4.f)); }

    // Value arc with neon glow
    float va=start+pos*(end-start);
    if(pos>0.004f){
        Path p; p.addArc(cc.x-ar,cc.y-ar,ar*2,ar*2,start,va,true);
        g.setColour(ac.withAlpha(0.22f));
        g.strokePath(p,PathStrokeType(7.f,PathStrokeType::curved,PathStrokeType::rounded));
        g.setColour(ac);
        g.strokePath(p,PathStrokeType(2.5f,PathStrokeType::curved,PathStrokeType::rounded));
        auto ep=cc.getPointOnCircumference(ar,va);
        g.setColour(ac.withAlpha(0.45f)); g.fillEllipse(ep.x-5,ep.y-5,10,10);
        g.setColour(C::ice);             g.fillEllipse(ep.x-2.5f,ep.y-2.5f,5,5);
    }

    // Hemisphere body
    float ir=r*0.68f;
    bool isHover=sl.isMouseOverOrDragging();
    bool isDrag =sl.isMouseButtonDown();
    g.setColour(Colour(0xFF020810)); g.fillEllipse(cc.x-r,cc.y-r,r*2,r*2);
    Colour bodyTop=isDrag?Colour(0xFF243C60):Colour(0xFF1A3855);
    ColourGradient body(bodyTop,cc.x-ir*0.3f,cc.y-ir,Colour(0xFF040C18),cc.x,cc.y+ir,false);
    g.setGradientFill(body); g.fillEllipse(cc.x-ir,cc.y-ir,ir*2,ir*2);
    float hlA=isDrag?0.25f:isHover?0.22f:0.16f;
    ColourGradient hl(Colour(0xFFFFFFFF).withAlpha(hlA),cc.x-ir*0.2f,cc.y-ir*0.85f,
                      Colour(0x00FFFFFF),cc.x,cc.y,true);
    g.setGradientFill(hl); g.fillEllipse(cc.x-ir*0.6f,cc.y-ir*0.95f,ir*1.2f,ir*0.9f);
    g.setColour(ac.withAlpha(isHover?0.40f:0.20f));
    g.drawEllipse(cc.x-ir,cc.y-ir,ir*2,ir*2,isHover?1.6f:1.f);

    // Indicator dot
    auto dp=cc.getPointOnCircumference(ir*0.60f,va);
    g.setColour(C::ice); g.fillEllipse(dp.x-2.5f,dp.y-2.5f,5,5);

    // Value text
    g.setColour(C::dim); g.setFont(Font("Arial",8.f,Font::plain));
    double v=sl.getValue();
    g.drawText(v==(int)v?String((int)v):String(v,1),
               (int)(cc.x-20),(int)(cc.y+ir+3),40,12,Justification::centred);
}

void OceanLookAndFeel::drawComboBox(Graphics& g,int w,int h,bool,int bx,int by,int bw,int bh,ComboBox&)
{
    ColourGradient bg(Colour(0xFF0C1E32),0,0,Colour(0xFF070F1E),0,(float)h,false);
    g.setGradientFill(bg); g.fillRoundedRectangle(0,0,w,h,5.f);
    g.setColour(C::sky.withAlpha(0.35f)); g.drawRoundedRectangle(0.5f,0.5f,w-1.f,h-1.f,5.f,1.f);
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
    // Small LED (<=20px) — drawn as glowing circle
    if(b.getHeight()<=20.f && b.getWidth()<=20.f){
        float r=jmin(b.getWidth(),b.getHeight())*0.38f;
        auto cc=b.getCentre();
        Colour ledCol=on?C::ledOn:C::ledOff;
        if(on){ g.setColour(ledCol.withAlpha(0.30f)); g.fillEllipse(cc.x-r-3,cc.y-r-3,(r+3)*2,(r+3)*2); }
        g.setColour(Colour(0xFF030810)); g.fillEllipse(cc.x-r,cc.y-r,r*2,r*2);
        ColourGradient fill(ledCol.brighter(0.4f),cc.x-r*0.3f,cc.y-r*0.7f,ledCol.darker(0.3f),cc.x,cc.y+r,false);
        g.setGradientFill(fill); g.fillEllipse(cc.x-r,cc.y-r,r*2,r*2);
        g.setColour(ledCol.withAlpha(0.70f)); g.drawEllipse(cc.x-r,cc.y-r,r*2,r*2,1.f);
        return;
    }
    // Normal toggle button
    if(on){
        ColourGradient gr(C::sky.withAlpha(0.25f),b.getX(),b.getY(),C::sky.withAlpha(0.07f),b.getX(),b.getBottom(),false);
        g.setGradientFill(gr); g.fillRoundedRectangle(b,7.f);
        g.setColour(C::sky.withAlpha(0.80f)); g.drawRoundedRectangle(b.reduced(0.5f),7.f,1.5f);
    } else {
        ColourGradient gr(Colour(0xFF0C1E32),b.getX(),b.getY(),Colour(0xFF070F1E),b.getX(),b.getBottom(),false);
        g.setGradientFill(gr); g.fillRoundedRectangle(b,7.f);
        g.setColour(hi?C::sky.withAlpha(0.35f):C::sky.withAlpha(0.18f));
        g.drawRoundedRectangle(b.reduced(0.5f),7.f,1.f);
    }
}

void OceanLookAndFeel::drawTickBox(Graphics& g,Component& comp,
    float x,float y,float w,float h,bool ticked,bool,bool,bool)
{
    // For LED-sized buttons suppress the checkbox entirely — LED drawn in drawButtonBackground
    if(w<=20.f && h<=20.f) return;
    // For larger toggles (SYNC, VIBRATO) draw a minimal rounded rect tick
    Rectangle<float> r(x,y,w,h);
    g.setColour(ticked ? C::sky.withAlpha(0.9f) : Colour(0xFF1A3050));
    g.fillRoundedRectangle(r,3.f);
    g.setColour(C::sky.withAlpha(0.55f)); g.drawRoundedRectangle(r,3.f,1.f);
    if(ticked){ g.setColour(C::ice); g.setFont(10.f);
                g.drawText("✓",r.toNearestInt(),Justification::centred); }
}

Font OceanLookAndFeel::getLabelFont(Label&) { return Font("Arial",9.5f,Font::bold); }
void OceanLookAndFeel::drawLabel(Graphics& g,Label& l) {
    g.setColour(l.findColour(Label::textColourId));
    g.setFont(getLabelFont(l));
    g.drawText(l.getText(),l.getLocalBounds(),Justification::centred,false);
}

// ── Panel helper ──────────────────────────────────────────────────────────────
static void drawPanel(Graphics& g,Rectangle<int> r,const char* title,Colour ac)
{
    auto rf=r.toFloat();
    ColourGradient fill(Colour(0xFF0C1A2A),(float)r.getX(),(float)r.getY(),
                        Colour(0xFF050D18),(float)r.getX(),(float)r.getBottom(),false);
    g.setGradientFill(fill); g.fillRoundedRectangle(rf,10.f);
    ColourGradient topGlow(ac.withAlpha(0.12f),(float)r.getCentreX(),(float)r.getY(),
                           ac.withAlpha(0.00f),(float)r.getCentreX(),r.getY()+r.getHeight()*0.35f,false);
    g.setGradientFill(topGlow); g.fillRoundedRectangle(rf.withHeight(r.getHeight()*0.35f),10.f);
    g.setColour(ac.withAlpha(0.50f)); g.drawRoundedRectangle(rf.reduced(0.5f),10.f,1.5f);
    g.setColour(Colour(0x10FFFFFF)); g.drawRoundedRectangle(rf.reduced(2.f),8.f,0.7f);
    g.setColour(ac.brighter(0.15f)); g.setFont(Font("Arial",8.5f,Font::bold));
    g.drawText(title,r.withHeight(20),Justification::centredTop,false);
}

// ── Scratched neon title ──────────────────────────────────────────────────────
static void drawScratchedTitle(Graphics& g,const String& text,float x,float y)
{
    Font f("Arial",30.f,Font::bold|Font::italic);
    GlyphArrangement ga; ga.addLineOfText(f,text,x,y+30.f);
    Path p; ga.createPath(p);
    g.setColour(Colour(0xFF000810)); g.fillPath(p,AffineTransform::translation(2.f,3.f));
    g.setColour(C::sky.withAlpha(0.18f)); g.fillPath(p,AffineTransform::translation(0.f,1.f));
    ColourGradient grad(C::ice,x,y,C::sky,x,y+34.f,false);
    g.setGradientFill(grad); g.fillPath(p);
    g.setColour(Colour(0xFFFFFFFF).withAlpha(0.50f)); g.strokePath(p,PathStrokeType(0.5f));
    auto pb=p.getBounds();
    g.saveState(); g.reduceClipRegion(p);
    g.setColour(Colour(0xFFFFFFFF).withAlpha(0.06f));
    for(float sx=pb.getX()-8.f;sx<pb.getRight()+8.f;sx+=12.f)
        g.drawLine(sx+10.f,pb.getY()-2.f,sx,pb.getBottom()+2.f,1.3f);
    g.restoreState();
}

// ── WaveformDisplay ───────────────────────────────────────────────────────────
WaveformDisplay::WaveformDisplay(GhostSurfProcessor& p):proc(p){ startTimerHz(30); }

void WaveformDisplay::paint(Graphics& g)
{
    auto b=getLocalBounds().toFloat();
    const float BW=b.getWidth(),BH=b.getHeight(),x0=b.getX(),y0=b.getY();
    const float wAmp=14.f,wOff=wAmp+8.f;
    const int NP=200;
    auto wY=[&](float t){
        return y0+wOff
            -std::sin(t*MathConstants<float>::pi*4.6f)*wAmp*0.88f
            -std::sin(t*MathConstants<float>::pi*2.1f)*wAmp*0.40f
            -std::sin(t*MathConstants<float>::pi*9.0f)*wAmp*0.10f;
    };
    Path shape;
    shape.startNewSubPath(x0,y0+BH); shape.lineTo(x0+BW,y0+BH); shape.lineTo(x0+BW,wY(1.f));
    for(int i=NP;i>=0;--i) shape.lineTo(x0+(float)i/NP*BW,wY((float)i/NP));
    shape.closeSubPath();
    ColourGradient bg(Colour(0xFF061222),x0,y0+wOff,Colour(0xFF040C18),x0,y0+BH,false);
    g.setGradientFill(bg); g.fillPath(shape);
    { Graphics::ScopedSaveState ss(g); g.reduceClipRegion(shape);
      float iT=y0+wOff,iH=BH-wOff,cy=iT+iH*0.5f;
      g.setColour(Colour(0x0800AACC));
      for(int i=1;i<5;++i) g.drawHorizontalLine((int)(iT+iH*i/5.f),x0+4,x0+BW-4);
      g.setColour(Colour(0x1800BBDD)); g.drawHorizontalLine((int)cy,x0+4,x0+BW-4);
      const float* d=proc.getScopePtr(); int wp=proc.getScopeWritePos();
      const int N=GhostSurfProcessor::SCOPE_SIZE,PW=(int)BW-6;
      Path osc; bool st=false;
      for(int i=0;i<PW;++i){
          float s=jlimit(-1.f,1.f,d[(wp+i*N/PW)%N]);
          float px=x0+3.f+i,py=cy-s*iH*0.42f;
          if(!st){osc.startNewSubPath(px,py);st=true;} else osc.lineTo(px,py);
      }
      g.setColour(C::sky.withAlpha(0.10f)); g.strokePath(osc,PathStrokeType(6.f,PathStrokeType::curved));
      g.setColour(C::sky.withAlpha(0.40f)); g.strokePath(osc,PathStrokeType(2.f,PathStrokeType::curved));
      g.setColour(C::ice.withAlpha(0.85f)); g.strokePath(osc,PathStrokeType(1.f,PathStrokeType::curved));
    }
    Path crest;
    for(int i=0;i<=NP;++i){float t=(float)i/NP,wx=x0+t*BW,wy=wY(t);
        if(i==0)crest.startNewSubPath(wx,wy); else crest.lineTo(wx,wy);}
    g.setColour(C::sky.withAlpha(0.12f)); g.strokePath(crest,PathStrokeType(8.f,PathStrokeType::curved));
    g.setColour(C::sky.withAlpha(0.42f)); g.strokePath(crest,PathStrokeType(3.f,PathStrokeType::curved));
    g.setColour(C::ice.withAlpha(0.80f)); g.strokePath(crest,PathStrokeType(1.f,PathStrokeType::curved));
}

// ── SpecterPad ────────────────────────────────────────────────────────────────
SpecterPad::SpecterPad(GhostSurfProcessor& p):proc(p){ startTimerHz(60); setMouseCursor(MouseCursor::CrosshairCursor); }

void SpecterPad::spawnParticles(float x,float y,Colour c,int n)
{
    for(int i=0;i<n;++i){
        float angle=Random::getSystemRandom().nextFloat()*MathConstants<float>::twoPi;
        float speed=1.f+Random::getSystemRandom().nextFloat()*2.5f;
        float life=0.4f+Random::getSystemRandom().nextFloat()*0.5f;
        particles.push_back({x,y,std::cos(angle)*speed,std::sin(angle)*speed,life,life,
                             1.5f+Random::getSystemRandom().nextFloat()*2.5f,c});
    }
    while((int)particles.size()>120) particles.erase(particles.begin());
}

void SpecterPad::timerCallback()
{
    bool dirty=!particles.empty();
    for(auto& p:particles){ p.x+=p.vx; p.y+=p.vy; p.vy+=0.07f; p.life-=1.f/60.f; }
    particles.erase(std::remove_if(particles.begin(),particles.end(),
        [](const Particle& p){return p.life<=0.f;}),particles.end());
    if(dirty||dragging) repaint();
}

void SpecterPad::drawFilterCurve(Graphics& g,Rectangle<float> area,int shape)
{
    float W=area.getWidth(),H=area.getHeight(),x0=area.getX(),y0=area.getY();
    float fc=filterX,q=1.f-filterY;
    Path curve; bool started=false;
    for(int i=0;i<200;++i){
        float t=(float)i/200.f,resp=0.f;
        switch(shape){
        case 0:{ float peak=q*0.9f*std::exp(-(t-fc)*(t-fc)*8.f);
                 resp=(t<fc?1.f:std::exp(-(t-fc)*8.f))+peak; break;}
        case 1:{ float bw=0.04f+q*0.10f,dist=std::abs(t-fc);
                 resp=std::exp(-dist*dist/(bw*bw))*(0.5f+q*0.9f); break;}
        case 2:{ float peak=q*0.9f*std::exp(-(t-fc)*(t-fc)*8.f);
                 resp=(t>fc?1.f:std::exp((t-fc)*8.f))+peak; break;}
        case 3:{ resp=(t<fc?1.f:std::exp(-(t-fc)*8.f));
                 resp=jmax(0.f,resp+0.38f*std::sin(t*42.f+fc*10.f)*std::sin(t*17.f)*q); break;}
        }
        resp=jlimit(0.f,1.4f,resp);
        float px=x0+t*W,py=y0+H-resp*H*0.73f;
        if(!started){curve.startNewSubPath(px,py);started=true;} else curve.lineTo(px,py);
    }
    g.setColour(C::sky.withAlpha(0.07f)); g.strokePath(curve,PathStrokeType(9.f,PathStrokeType::curved));
    g.setColour(C::aqua.withAlpha(0.55f)); g.strokePath(curve,PathStrokeType(2.f,PathStrokeType::curved));
    g.setColour(C::ice.withAlpha(0.88f)); g.strokePath(curve,PathStrokeType(0.8f,PathStrokeType::curved));
}

void SpecterPad::paint(Graphics& g)
{
    auto b=getLocalBounds().toFloat();
    float W=b.getWidth(),H=b.getHeight();
    ColourGradient bg(Colour(0xFF071826),b.getX(),b.getY(),Colour(0xFF030E1C),b.getX(),b.getBottom(),false);
    g.setGradientFill(bg); g.fillRoundedRectangle(b,8.f);
    g.setColour(Colour(0x0800BBDD));
    for(int i=1;i<8;++i) g.drawVerticalLine((int)(b.getX()+W*i/8.f),(float)b.getY()+5,(float)b.getBottom()-5);
    for(int i=1;i<4;++i) g.drawHorizontalLine((int)(b.getY()+H*i/4.f),(float)b.getX()+5,(float)b.getRight()-5);
    static const char* freqLabels[]={"100","500","1k","2k","5k","10k","16k"};
    g.setColour(C::dim.withAlpha(0.45f)); g.setFont(Font("Arial",7.f,Font::plain));
    for(int i=0;i<7;++i)
        g.drawText(freqLabels[i],(int)(b.getX()+W*(i+1)/8.f)-12,(int)(b.getBottom()-14),24,11,Justification::centred);
    drawFilterCurve(g,b.reduced(4,22),proc.getSpecterShape());
    for(auto& p:particles){
        float alpha=jmax(0.f,p.life/p.maxLife);
        g.setColour(p.col.withAlpha(alpha*0.85f));
        g.fillEllipse(p.x-p.size*0.5f,p.y-p.size*0.5f,p.size,p.size);
    }
    // Visual cursor (hover — curX/curY)
    float cx=b.getX()+curX*W,cy=b.getY()+curY*H;
    g.setColour(C::sky.withAlpha(0.10f));
    g.drawVerticalLine((int)cx,(float)b.getY(),(float)b.getBottom());
    g.drawHorizontalLine((int)cy,(float)b.getX(),(float)b.getRight());
    ColourGradient glow(C::aqua.withAlpha(0.50f),cx,cy,C::aqua.withAlpha(0.f),cx+18,cy,true);
    g.setGradientFill(glow); g.fillEllipse(cx-18,cy-18,36,36);
    g.setColour(C::ice); g.fillEllipse(cx-4,cy-4,8,8);
    g.setColour(C::aqua.withAlpha(0.85f)); g.drawEllipse(cx-5,cy-5,10,10,1.5f);
    // Committed filter marker (filterX/filterY)
    if(!dragging){
        float fx=b.getX()+filterX*W,fy=b.getY()+filterY*H;
        g.setColour(C::mint.withAlpha(0.55f));
        g.drawLine(fx-6,fy,fx+6,fy,1.2f); g.drawLine(fx,fy-6,fx,fy+6,1.2f);
    }
    // Shape buttons
    static const char* shapes[]={"SINE","SQ","SAW","CHAOS"};
    static const Colour shapeCols[]={C::sky,C::cobalt,C::mint,C::violet};
    int shape=proc.getSpecterShape();
    for(int i=0;i<4;++i){
        float bx=b.getX()+4.f+i*56.f,by=b.getY()+4.f;
        bool active=(i==shape);
        g.setColour(active?shapeCols[i].withAlpha(0.28f):Colour(0x14FFFFFF));
        g.fillRoundedRectangle(bx,by,50,17,4.f);
        g.setColour(active?shapeCols[i]:C::dim);
        g.drawRoundedRectangle(bx,by,50,17,4.f,1.f);
        g.setFont(Font("Arial",7.5f,Font::bold));
        g.drawText(shapes[i],(int)bx,(int)by,50,17,Justification::centred);
    }
    g.setColour(C::dim); g.setFont(Font("Arial",7.5f,Font::plain));
    float fc=proc.getSpecterCutoff();
    String fcStr=fc<1000.f?String((int)fc)+"Hz":String(fc/1000.f,1)+"kHz";
    g.drawText("fc="+fcStr,(int)(b.getRight()-82),(int)b.getY()+4,78,11,Justification::centredRight);
    g.drawText("Q="+String(proc.getSpecterReso(),2),(int)(b.getRight()-82),(int)b.getY()+14,78,11,Justification::centredRight);
    g.setColour(C::aqua.withAlpha(0.32f)); g.drawRoundedRectangle(b.reduced(0.5f),8.f,1.2f);
}

void SpecterPad::mouseDown(const MouseEvent& e)
{
    auto b=getLocalBounds().toFloat();
    if(e.y<24){
        dragging=false;
        int shp=jlimit(0,3,(int)((e.x-4.f)/56.f));
        proc.setSpecterShape(shp);
        static const Colour sc[]={C::sky,C::cobalt,C::mint,C::violet};
        spawnParticles((float)e.x,(float)e.y,sc[shp],10);
        repaint(); return;
    }
    dragging=true;
    curX=filterX=jlimit(0.f,1.f,(e.x-b.getX())/b.getWidth());
    curY=filterY=jlimit(0.f,1.f,(e.y-b.getY())/b.getHeight());
    proc.setSpecterCutoff(20.f*std::pow(1000.f,filterX));
    proc.setSpecterReso(1.f-filterY);
    spawnParticles((float)e.x,(float)e.y,C::aqua,8);
}

void SpecterPad::mouseDrag(const MouseEvent& e)
{
    if(!dragging) return;
    auto b=getLocalBounds().toFloat();
    curX=filterX=jlimit(0.f,1.f,(e.x-b.getX())/b.getWidth());
    curY=filterY=jlimit(0.f,1.f,(e.y-b.getY())/b.getHeight());
    proc.setSpecterCutoff(20.f*std::pow(1000.f,filterX));
    proc.setSpecterReso(1.f-filterY);
    if(Random::getSystemRandom().nextInt(3)==0)
        spawnParticles((float)e.x,(float)e.y,C::aqua.withAlpha(0.55f),2);
}

void SpecterPad::mouseMove(const MouseEvent& e)
{
    // Only update visual cursor — no audio change on hover
    auto b=getLocalBounds().toFloat();
    curX=jlimit(0.f,1.f,(e.x-b.getX())/b.getWidth());
    curY=jlimit(0.f,1.f,(e.y-b.getY())/b.getHeight());
    repaint();
}

void SpecterPad::mouseUp(const MouseEvent&) { dragging=false; }

// ── FreezePanel ───────────────────────────────────────────────────────────────
FreezePanel::FreezePanel(GhostSurfProcessor& p):proc(p){ startTimerHz(30); }

void FreezePanel::timerCallback()
{
    bool active=proc.isFreezing();
    if(active){ pulse+=0.07f; if(pulse>MathConstants<float>::twoPi) pulse-=MathConstants<float>::twoPi;
                glowAnim=jmin(1.f,glowAnim+0.06f); }
    else { glowAnim=jmax(0.f,glowAnim-0.06f); }
    if(active!=wasActive){wasActive=active;repaint();}
    if(active||glowAnim>0.01f) repaint();
}

void FreezePanel::paint(Graphics& g)
{
    auto b=getLocalBounds().toFloat();
    bool active=proc.isFreezing();
    ColourGradient bg(Colour(0xFF071826),b.getX(),b.getY(),Colour(0xFF030E1C),b.getX(),b.getBottom(),false);
    g.setGradientFill(bg); g.fillRoundedRectangle(b,8.f);
    float cx=b.getCentreX(),cy=b.getY()+b.getHeight()*0.42f;
    float rad=jmin(b.getWidth(),b.getHeight())*0.28f;
    if(glowAnim>0.01f){
        float pr=rad+5.f+std::sin(pulse)*4.f;
        ColourGradient gl(C::freeze.withAlpha(0.22f*glowAnim),cx,cy,C::freeze.withAlpha(0.f),cx+pr,cy,true);
        g.setGradientFill(gl); g.fillEllipse(cx-pr,cy-pr,pr*2,pr*2);
    }
    Path hex;
    for(int i=0;i<6;++i){
        float a=MathConstants<float>::pi/6.f+i*MathConstants<float>::pi/3.f;
        float hx=cx+std::cos(a)*rad,hy=cy+std::sin(a)*rad;
        if(i==0)hex.startNewSubPath(hx,hy); else hex.lineTo(hx,hy);
    }
    hex.closeSubPath();
    if(active){
        ColourGradient fill(C::freeze.withAlpha(0.38f),cx,cy-rad,C::freeze.withAlpha(0.12f),cx,cy+rad,false);
        g.setGradientFill(fill); g.fillPath(hex);
        g.setColour(C::freeze.withAlpha(0.92f)); g.strokePath(hex,PathStrokeType(2.f));
        g.setColour(C::ice.withAlpha(0.22f));
        for(int i=0;i<6;++i){ float a=i*MathConstants<float>::pi/3.f+pulse*0.08f;
            g.drawLine(cx,cy,cx+std::cos(a)*rad*0.8f,cy+std::sin(a)*rad*0.8f,1.f); }
    } else {
        ColourGradient fill(Colour(0xFF122030),cx,cy-rad,Colour(0xFF081520),cx,cy+rad,false);
        g.setGradientFill(fill); g.fillPath(hex);
        g.setColour(C::freeze.withAlpha(0.32f+glowAnim*0.38f)); g.strokePath(hex,PathStrokeType(1.5f));
    }
    g.setFont(Font("Arial",active?8.5f:9.5f,Font::bold));
    g.setColour(active?C::ice:C::freeze.withAlpha(0.70f));
    g.drawText(active?"HOLD":"FREEZE",(int)(cx-30),(int)(cy-8),60,16,Justification::centred);
    if(active){
        float sh=proc.getAPVTS().getRawParameterValue("freezeShimmer")->load();
        if(sh>0.05f){ g.setColour(C::violet.withAlpha(0.6f)); g.setFont(Font("Arial",7.f,Font::plain));
            g.drawText("shimmer",(int)b.getX(),(int)(cy+rad+6),(int)b.getWidth(),11,Justification::centred); }
    }
    g.setColour(C::freeze.withAlpha(0.45f)); g.setFont(Font("Arial",7.5f,Font::bold));
    g.drawText("FREEZE",b.removeFromBottom(16.f),Justification::centred);
    g.setColour(C::freeze.withAlpha(0.28f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f),8.f,1.2f);
}

void FreezePanel::mouseDown(const MouseEvent& e)
{
    auto b=getLocalBounds().toFloat();
    float cx=b.getCentreX(),cy=b.getY()+b.getHeight()*0.42f;
    float rad=jmin(b.getWidth(),b.getHeight())*0.28f;
    if(e.getPosition().toFloat().getDistanceFrom({cx,cy})<rad+16.f)
        proc.setFreezeActive(!proc.isFreezing());
}
void FreezePanel::mouseEnter(const MouseEvent&){ setMouseCursor(MouseCursor::PointingHandCursor); }
void FreezePanel::mouseExit (const MouseEvent&){ setMouseCursor(MouseCursor::NormalCursor); }

// ── VUMeter ───────────────────────────────────────────────────────────────────
VUMeter::VUMeter(GhostSurfProcessor& p):proc(p){ startTimerHz(30); }

void VUMeter::timerCallback()
{
    float target=proc.getOutputLevel();
    float coeff=(target>displayLevel)?0.40f:0.06f; // fast attack, slow release
    displayLevel+=(target-displayLevel)*coeff;
    repaint();
}

void VUMeter::paint(Graphics& g)
{
    auto b=getLocalBounds().toFloat();
    ColourGradient bg(Colour(0xFF071222),b.getX(),b.getY(),Colour(0xFF040C18),b.getX(),b.getBottom(),false);
    g.setGradientFill(bg); g.fillRoundedRectangle(b,5.f);
    g.setColour(C::sky.withAlpha(0.28f)); g.drawRoundedRectangle(b.reduced(0.5f),5.f,1.f);
    const int S=20; float sH=(b.getHeight()-22.f)/S;
    float db=Decibels::gainToDecibels(displayLevel,-60.f);
    int lit=jlimit(0,S,(int)(jmap(db,-36.f,0.f,0.f,1.f)*S)); // range -36 to 0 dBFS
    for(int i=0;i<S;++i){
        auto seg=Rectangle<float>(b.getX()+5,b.getBottom()-15.f-(i+1)*sH,b.getWidth()-10,sH-1.5f);
        Colour sc=i<lit?(i>=S-2?Colour(0xFFFF1133):i>=S-5?Colour(0xFFFF9900):C::aqua):Colour(0xFF091A28);
        g.setColour(sc); g.fillRoundedRectangle(seg,1.5f);
    }
    g.setColour(C::dim); g.setFont(7.5f);
    g.drawText("dB",b.removeFromBottom(14.f),Justification::centred);
}

// ── Knob builder ─────────────────────────────────────────────────────────────
void GhostSurfEditor::buildKnob(KnobWidget& kw,const char* id,const char* lbl,Colour ac)
{
    kw.slider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    kw.slider.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    kw.slider.setVelocityBasedMode(false); // direct drag — feels natural
    kw.slider.setScrollWheelEnabled(true);
    if(auto* param=proc.getAPVTS().getParameter(id)){
        double def=(double)param->convertFrom0to1(param->getDefaultValue());
        kw.slider.setDoubleClickReturnValue(true,def);
    }
    kw.slider.setLookAndFeel(&lf);
    if(ac==C::aqua)        kw.slider.setComponentID("aqua");
    else if(ac==C::cobalt) kw.slider.setComponentID("cobalt");
    else if(ac==C::mint)   kw.slider.setComponentID("mint");
    else if(ac==C::violet) kw.slider.setComponentID("violet");
    else if(ac==C::freeze) kw.slider.setComponentID("freeze");
    else                   kw.slider.setComponentID("sky");
    kw.slider.setMouseCursor(MouseCursor::UpDownLeftRightResizeCursor);
    addAndMakeVisible(kw.slider);
    kw.label.setText(lbl,dontSendNotification);
    kw.label.setFont(Font("Arial",8.5f,Font::bold));
    kw.label.setColour(Label::textColourId,ac.withAlpha(0.80f));
    kw.label.setJustificationType(Justification::centred);
    addAndMakeVisible(kw.label);
    kw.attach=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(proc.getAPVTS(),id,kw.slider);
}

void GhostSurfEditor::placeKnob(KnobWidget& kw,int cx,int cy,int sz)
{
    kw.slider.setBounds(cx-sz/2,cy-sz/2,sz,sz);
    kw.label.setBounds(cx-28,cy+sz/2+2,56,13);
}

juce::Slider* GhostSurfEditor::findRankedSlider(int rank)
{
    KnobWidget* all[]={&reverbMix,&reverbDecay,&reverbTone,&tremSpeed,&tremDepth,
                       &flangerRate,&flangerDepth,&flangerFeedback,
                       &drive,&lofi,&bass,&treble,&wahDepth,&wahRate,
                       &slideAmount,&slideSpeed,&vibeSpeed,&vibeDepth,
                       &freezeGrain,&freezeShimmer,&freezeDecay};
    for(auto* kw:all)
        if((int)kw->slider.getProperties()["live"]==rank) return &kw->slider;
    return nullptr;
}

// ── Constructor ───────────────────────────────────────────────────────────────
GhostSurfEditor::GhostSurfEditor(GhostSurfProcessor& p)
    : AudioProcessorEditor(&p),proc(p),vuMeter(p),waveDisplay(p),specterPad(p),freezePanel(p)
{
    setSize(760,660);
    setLookAndFeel(&lf);
    setWantsKeyboardFocus(true);

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

    // Tremolo controls
    tremSyncBtn.setButtonText("SYNC"); tremSyncBtn.setLookAndFeel(&lf); addAndMakeVisible(tremSyncBtn);
    tremSyncAttach=std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(proc.getAPVTS(),"tremSync",tremSyncBtn);
    tremDivBox.addItem("1/4",1); tremDivBox.addItem("1/8",2); tremDivBox.addItem("1/16",3);
    addAndMakeVisible(tremDivBox);
    tremDivAttach=std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(proc.getAPVTS(),"tremDiv",tremDivBox);

    // Vibe mode — positioned to not overlap DEPTH knob
    vibeModeBtn.setButtonText("VIBRATO"); vibeModeBtn.setLookAndFeel(&lf); addAndMakeVisible(vibeModeBtn);
    vibeModeAttach=std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(proc.getAPVTS(),"vibeMode",vibeModeBtn);

    // LED bypass buttons
    auto setupLed=[&](ToggleButton& btn,
                      std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment>& att,
                      const char* id){
        btn.setLookAndFeel(&lf);
        addAndMakeVisible(btn);
        att=std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(proc.getAPVTS(),id,btn);
    };
    setupLed(ledReverb,  ledReverbA,  "reverbOn");
    setupLed(ledTremolo, ledTremoloA, "tremoloOn");
    setupLed(ledFlanger, ledFlangerA, "flangerOn");
    setupLed(ledWah,     ledWahA,     "wahOn");
    setupLed(ledSlide,   ledSlideA,   "slideOn");
    setupLed(ledVibe,    ledVibeA,    "vibeOn");
    setupLed(ledSpecter, ledSpecterA, "specterOn");
    setupLed(ledAutoPan, ledAutoPanA, "autoPanOn");

    // Knobs
    buildKnob(reverbMix,      "reverbMix",      "MIX",    C::sky);
    buildKnob(reverbDecay,    "reverbDecay",    "DECAY",  C::sky);
    buildKnob(reverbTone,     "reverbTone",     "TONE",   C::sky);
    buildKnob(tremSpeed,      "tremSpeed",      "SPEED",  C::aqua);
    buildKnob(tremDepth,      "tremDepth",      "DEPTH",  C::aqua);
    buildKnob(flangerRate,    "flangerRate",    "RATE",   C::aqua);
    buildKnob(flangerDepth,   "flangerDepth",   "DEPTH",  C::aqua);
    buildKnob(flangerFeedback,"flangerFeedback","FBACK",  C::aqua);
    buildKnob(drive,          "drive",          "DRIVE",  C::cobalt);
    buildKnob(lofi,           "lofi",           "LO-FI",  C::cobalt);
    buildKnob(bass,           "bass",           "BASSES", C::cobalt);
    buildKnob(treble,         "treble",         "AIGUS",  C::cobalt);
    buildKnob(wahDepth,       "wahDepth",       "DEPTH",  C::cobalt);
    buildKnob(wahRate,        "wahRate",        "RATE",   C::cobalt);
    buildKnob(autoPanRate,    "autoPanRate",    "VITESSE",C::mint);
    buildKnob(slideAmount,    "slideAmount",    "GLISS",  C::mint);
    buildKnob(slideSpeed,     "slideSpeed",     "SPEED",  C::mint);
    buildKnob(vibeSpeed,      "vibeSpeed",      "SPEED",  C::violet);
    buildKnob(vibeDepth,      "vibeDepth",      "DEPTH",  C::violet);
    buildKnob(freezeGrain,    "freezeGrain",    "GRAIN",  C::freeze);
    buildKnob(freezeShimmer,  "freezeShimmer",  "SHIMMER",C::freeze);
    buildKnob(freezeDecay,    "freezeDecay",    "WET",    C::freeze);

    addAndMakeVisible(vuMeter);
    addAndMakeVisible(waveDisplay);
    addAndMakeVisible(specterPad);
    addAndMakeVisible(freezePanel);

    liveColours[0]=Colour(0xFFFF3322);
    liveColours[1]=Colour(0xFFFF8800);
    liveColours[2]=Colour(0xFFFFCC00);
    updateLiveHighlights(p.getCurrentProgram());
    startTimerHz(20);
}

GhostSurfEditor::~GhostSurfEditor() { setLookAndFeel(nullptr); }

void GhostSurfEditor::timerCallback()
{
    float target=proc.getSurfScore();
    scoreAnim+=(target-scoreAnim)*0.18f;
    int combo=proc.getSurfCombo();
    if(combo!=comboFlash) comboFlash=combo;
    repaint();
}

// ── keyPressed ────────────────────────────────────────────────────────────────
bool GhostSurfEditor::keyPressed(const KeyPress& k)
{
    if(k==KeyPress::upKey){
        int p=(proc.getCurrentProgram()-1+GhostSurfProcessor::NUM_PRESETS)%GhostSurfProcessor::NUM_PRESETS;
        proc.setCurrentProgram(p);
        presetBox.setSelectedId(p+1,dontSendNotification);
        updateLiveHighlights(p);
        return true;
    }
    if(k==KeyPress::downKey){
        int p=(proc.getCurrentProgram()+1)%GhostSurfProcessor::NUM_PRESETS;
        proc.setCurrentProgram(p);
        presetBox.setSelectedId(p+1,dontSendNotification);
        updateLiveHighlights(p);
        return true;
    }
    if(k==KeyPress::leftKey||k==KeyPress::rightKey){
        if(auto* s=findRankedSlider(1)){
            double step=(s->getMaximum()-s->getMinimum())*0.01;
            s->setValue(jlimit(s->getMinimum(),s->getMaximum(),
                               s->getValue()+(k==KeyPress::rightKey?step:-step)),sendNotification);
        }
        return true;
    }
    return false;
}

// ── updateLiveHighlights ─────────────────────────────────────────────────────
void GhostSurfEditor::updateLiveHighlights(int idx)
{
    // Deterministic random hues — evenly spaced 120 degrees apart
    Random rnd((int64)(idx*77777+12345));
    float h0=rnd.nextFloat();
    liveColours[0]=Colour::fromHSV(h0,                    0.90f,1.00f,1.f);
    liveColours[1]=Colour::fromHSV(std::fmod(h0+0.33f,1.f),0.85f,0.95f,1.f);
    liveColours[2]=Colour::fromHSV(std::fmod(h0+0.66f,1.f),0.80f,0.90f,1.f);

    struct Def { const char* p1,*p2,*p3; };
    static const Def L[12]={
        {"reverbMix",    "reverbDecay",    "flangerDepth"},  // The Cure
        {"lofi",         "reverbMix",      "freezeDecay"},   // Lil Peep
        {"drive",        "wahDepth",       "bass"},          // Iggy Pop
        {"reverbMix",    "reverbTone",     "treble"},        // Surf Clean
        {"tremDepth",    "freezeShimmer",  "flangerDepth"},  // Night Waves
        {"tremSpeed",    "tremDepth",      "reverbTone"},    // Dick Dale
        {"drive",        "vibeDepth",      "reverbMix"},     // Pixies
        {"flangerDepth", "drive",          "treble"},        // Joy Division
        {"drive",        "slideAmount",    "bass"},          // Jack White
        {"freezeShimmer","vibeDepth",      "tremDepth"},     // Haunted Motel
        {"drive",        "wahDepth",       "vibeDepth"},     // Jimi Hendrix
        {"treble",       "bass",           "lofi"},          // Nile Rodgers
    };

    const char* kids[]={"reverbMix","reverbDecay","reverbTone","tremSpeed","tremDepth",
                        "flangerRate","flangerDepth","flangerFeedback",
                        "drive","lofi","bass","treble",
                        "wahDepth","wahRate","slideAmount","slideSpeed",
                        "vibeSpeed","vibeDepth","freezeGrain","freezeShimmer","freezeDecay"};
    KnobWidget* kmap[]={&reverbMix,&reverbDecay,&reverbTone,&tremSpeed,&tremDepth,
                        &flangerRate,&flangerDepth,&flangerFeedback,
                        &drive,&lofi,&bass,&treble,
                        &wahDepth,&wahRate,&slideAmount,&slideSpeed,
                        &vibeSpeed,&vibeDepth,&freezeGrain,&freezeShimmer,&freezeDecay};
    const int NK=21;
    for(int i=0;i<NK;++i){
        kmap[i]->slider.getProperties().set("live",0);
        kmap[i]->slider.getProperties().set("liveColor",(int64)0);
        kmap[i]->slider.repaint();
    }
    if(idx<0||idx>=12) return;
    const char* ranked[]={L[idx].p1,L[idx].p2,L[idx].p3};
    for(int r=0;r<3;++r)
        for(int i=0;i<NK;++i)
            if(std::strcmp(kids[i],ranked[r])==0){
                kmap[i]->slider.getProperties().set("live",r+1);
                kmap[i]->slider.getProperties().set("liveColor",(int64)liveColours[r].getARGB());
                kmap[i]->slider.repaint();
            }
}

// ── paint ─────────────────────────────────────────────────────────────────────
void GhostSurfEditor::paint(Graphics& g)
{
    const int W=getWidth(),H=getHeight();

    // Background
    ColourGradient bg(Colour(0xFF040810),0,0,Colour(0xFF060F18),0,H,false);
    g.setGradientFill(bg); g.fillAll();

    // Symmetric animated background waves
    float t=(float)(Time::getMillisecondCounter()%8000)/8000.f*MathConstants<float>::twoPi;
    for(int i=0;i<3;++i){
        float wy=H*(0.20f+i*0.25f),amp=11.f+i*4.f;
        Path w; w.startNewSubPath(0,wy);
        for(int xi=0;xi<=W;xi+=4){
            float fx=(float)xi/W-0.5f; // -0.5 to +0.5
            float sym=std::sin(std::abs(fx)*MathConstants<float>::pi*6.f+t+i*1.1f)*std::cos(fx*MathConstants<float>::pi);
            w.lineTo((float)xi,wy+sym*amp);
        }
        w.lineTo((float)W,(float)H); w.lineTo(0,(float)H); w.closeSubPath();
        g.setColour(Colour(0xFF0A2040).withAlpha(0.04f+i*0.015f)); g.fillPath(w);
    }

    // Header
    g.setColour(Colour(0xCC030C16)); g.fillRect(0,0,W,66);
    // Symmetric neon divider line
    ColourGradient hline(C::sky.withAlpha(0.0f),0,65,C::aqua.withAlpha(0.65f),(float)W/2,65,false);
    hline.addColour(1.0,C::sky.withAlpha(0.0f));
    g.setGradientFill(hline); g.fillRect(0,64,W,2);
    g.setColour(C::sky.withAlpha(0.20f)); g.drawHorizontalLine(66,0,(float)W);

    drawScratchedTitle(g,"GHOST SURF",14.f,12.f);

    // Surf Score
    {
        float barX=220.f,barY=20.f,barW=185.f,barH=11.f;
        g.setColour(Colour(0xFF081C2E)); g.fillRoundedRectangle(barX,barY,barW,barH,4.f);
        float fill=jlimit(0.f,1.f,scoreAnim/100.f);
        Colour fc=fill>0.80f?Colour(0xFFFFDD00):fill>0.50f?C::aqua:C::sky;
        ColourGradient barG(fc.withAlpha(0.92f),barX,barY,fc.withAlpha(0.50f),barX+barW*fill,barY,false);
        g.setGradientFill(barG); g.fillRoundedRectangle(barX,barY,barW*fill,barH,4.f);
        g.setColour(C::sky.withAlpha(0.28f)); g.drawRoundedRectangle(barX,barY,barW,barH,4.f,1.f);
        g.setColour(C::ice.withAlpha(0.55f)); g.setFont(Font("Arial",7.5f,Font::bold));
        g.drawText("SURF SCORE",(int)barX,(int)(barY+barH+2),(int)barW,10,Justification::centredLeft);
        if(comboFlash>1){
            float bx=barX+barW+6,by=barY-1;
            Colour cc=comboFlash>=6?Colour(0xFFFFDD00):comboFlash>=3?C::aqua:C::sky;
            g.setColour(cc.withAlpha(0.22f)); g.fillRoundedRectangle(bx,by,38,16,5.f);
            g.setColour(cc); g.drawRoundedRectangle(bx,by,38,16,5.f,1.f);
            g.setFont(Font("Arial",8.f,Font::bold));
            g.drawText("x"+String(comboFlash),(int)bx,(int)by,38,16,Justification::centred);
        }
    }

    // Live color legend
    for(int r=0;r<3;++r){
        g.setColour(liveColours[r].withAlpha(0.88f));
        g.fillEllipse((float)(W-148+(r)*40),22.f,8.f,8.f);
    }
    g.setColour(C::dim.withAlpha(0.55f)); g.setFont(Font("Arial",7.f,Font::plain));
    g.drawText("live",W-50,20,38,12,Justification::centredLeft);

    // Panels
    drawPanel(g,{8,  67,215,316},"SPRING REVERB",    C::sky);
    drawPanel(g,{231,67,162,316},"TREMOLO + FLANGER", C::aqua);
    drawPanel(g,{401,67,234,316},"EFFETS + WAH",      C::cobalt);
    drawPanel(g,{643,67, 110,155},"SLIDE",            C::mint);
    drawPanel(g,{643,230,110,160},"UNI-VIBE",         C::violet);
    drawPanel(g,{643,398,110,251},"NIVEAU",           C::sky.withAlpha(0.7f));
    drawPanel(g,{8,  398,627,251},"SPECTER / FREEZE", C::aqua);

    // Sub-section labels (ASCII only — no encoding problems)
    g.setFont(Font("Arial",7.5f,Font::bold));
    g.setColour(C::aqua.withAlpha(0.40f));
    g.drawText("-- FLANGER --",235,248,154,12,Justification::centred);
    g.setColour(C::cobalt.withAlpha(0.40f));
    g.drawText("-- WAH-WAH --",405,248,226,12,Justification::centred);
    g.setColour(C::mint.withAlpha(0.40f));
    g.drawText("-- AUTO-PAN --",405,315,226,12,Justification::centred);

    // Uni-Vibe spinning wheel
    {
        float spd=proc.getAPVTS().getRawParameterValue("vibeSpeed")->load();
        float dep=proc.getAPVTS().getRawParameterValue("vibeDepth")->load();
        static float wA=0.f; wA+=0.04f*spd*dep;
        if(dep>0.01f){
            float wCx=698.f,wCy=300.f,wR=18.f;
            g.setColour(C::violet.withAlpha(0.10f+dep*0.14f));
            g.drawEllipse(wCx-wR,wCy-wR,wR*2,wR*2,dep>0.5f?3.f:2.f);
            g.setColour(C::violet.withAlpha(0.55f));
            g.drawLine(wCx,wCy,wCx+std::cos(wA)*wR*0.8f,wCy+std::sin(wA)*wR*0.8f,2.f);
            g.drawLine(wCx,wCy,wCx+std::cos(wA+MathConstants<float>::pi)*wR*0.55f,
                       wCy+std::sin(wA+MathConstants<float>::pi)*wR*0.55f,1.f);
        }
    }
}

// ── resized ───────────────────────────────────────────────────────────────────
void GhostSurfEditor::resized()
{
    presetBox.setBounds(448,19,202,28);
    tremSyncBtn.setBounds(240,206,58,20);
    tremDivBox .setBounds(303,206,80,20);
    // VIBRATO: petit bouton compact dans le panel UNI-VIBE, sous le knob DEPTH
    vibeModeBtn.setBounds(653,370,104,18);

    // LED buttons — 16x16, top-right corner of each panel
    ledReverb .setBounds(213,71,16,16);
    ledTremolo.setBounds(384,71,16,16);
    ledFlanger.setBounds(384,251,16,16);
    ledWah    .setBounds(626,251,16,16);
    ledSlide  .setBounds(744,71,16,16);
    ledVibe   .setBounds(744,234,16,16);
    ledSpecter.setBounds(626,402,16,16);
    ledAutoPan.setBounds(626,318,16,16);

    const int KS=52,KY=128;

    // Spring Reverb knobs
    placeKnob(reverbMix,  57,KY,KS);
    placeKnob(reverbDecay,117,KY,KS);
    placeKnob(reverbTone, 177,KY,KS);
    waveDisplay.setBounds(14,200,206,175);

    // Tremolo knobs
    placeKnob(tremSpeed,272,KY,KS);
    placeKnob(tremDepth,352,KY,KS);

    // Flanger knobs (sub-section below tremolo)
    placeKnob(flangerRate,    258,272,44);
    placeKnob(flangerDepth,   312,272,44);
    placeKnob(flangerFeedback,366,272,44);

    // Effets knobs
    placeKnob(drive,  449,KY,KS);
    placeKnob(lofi,   509,KY,KS);
    placeKnob(bass,   565,KY,KS);
    placeKnob(treble, 619,KY,KS);

    // Wah-Wah knobs (sub-section below effets)
    placeKnob(wahDepth,449,274,44);
    placeKnob(wahRate, 509,274,44);

    // Auto-Pan knob (sub-section below wah)
    placeKnob(autoPanRate,518,340,44);

    // Slide
    placeKnob(slideAmount,698,105,KS);
    placeKnob(slideSpeed, 698,178,KS);

    // Uni-Vibe — speed + depth, VIBRATO button sous depth sans overlap
    placeKnob(vibeSpeed,698,258,KS);
    placeKnob(vibeDepth,698,325,KS);

    // Specter Pad
    specterPad.setBounds(14,416,415,225);

    // Freeze
    freezePanel.setBounds(436,416,112,225);

    // Freeze knobs
    placeKnob(freezeGrain,  565,435,44);
    placeKnob(freezeShimmer,565,502,44);
    placeKnob(freezeDecay,  565,569,44);

    // VU Meter — aligné avec le panel NIVEAU
    vuMeter.setBounds(649,412,104,230);
}
