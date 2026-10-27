#include "DirectionAnalysisPage.h"
#include <basewidgets/Charts/BeamPlots.h>
#include <basewidgets/Charts/LofarDemonPlots.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Inputs/PushButton.h>
#include <QDoubleSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <cmath>
using namespace basewidgets;
DirectionAnalysisPage::DirectionAnalysisPage(QWidget* parent) : StyledWidget(parent)
{
    auto* outer=new QVBoxLayout(this); outer->setContentsMargins(10,10,10,10);
    auto* hint=new Label("合成示例 · 三束响应 / 方位历程 / LOFAR / DEMON · Shift 拖拽放大，双击恢复，右键扩展",this);
    hint->setWordWrap(true); outer->addWidget(hint);
    auto* row=new QHBoxLayout; m_run=new CheckBox("继续模拟",this); m_run->setObjectName("DirectionDemoRun"); row->addWidget(m_run);
    auto* reset=new PushButton("恢复所有视图",this); row->addWidget(reset); row->addStretch(); outer->addLayout(row);
    auto* colors=new QHBoxLayout;
    auto spin=[&](double value) { auto* result=new QDoubleSpinBox(this); result->setRange(-1e12,1e12); result->setValue(value); result->setDecimals(1); result->setMinimumWidth(70); result->setMaximumWidth(130); return result; };
    auto* lower=spin(-55); auto* upper=spin(-5); auto* threshold=spin(-30); auto* enabled=new CheckBox("显示门限",this);
    colors->addWidget(new Label("测向色标",this)); colors->addWidget(lower); colors->addWidget(new Label("至",this)); colors->addWidget(upper);
    colors->addWidget(enabled); colors->addWidget(threshold); outer->addLayout(colors);
    auto* output=new Label("显示门限只隐藏低值，不修改原始响应；框选会发出可供业务处理的范围。",this); output->setWordWrap(true); outer->addWidget(output);
    auto* scroll=new ScrollArea(this); scroll->setFrameShape(QFrame::NoFrame); outer->addWidget(scroll,1);
    auto* content=new StyledWidget; auto* layout=new QVBoxLayout(content); layout->setContentsMargins(0,0,0,0); layout->setSpacing(14);
    m_beams=new BeamPatternPlot; m_beams->setObjectName("DirectionDemoBeams"); m_beams->setMinimumHeight(300); layout->addWidget(m_beams);
    m_history=new BearingHistoryPlot; m_history->setObjectName("DirectionDemoHistory"); m_history->setMinimumHeight(340);
    m_history->configure(0,360,361,64,.125); m_history->setLevelRange(-55,-5); layout->addWidget(m_history);
    m_lofar=new LofarPlot; m_lofar->setObjectName("DirectionDemoLofar"); m_lofar->setMinimumHeight(320); m_lofar->setLevelRange(-100,-20); layout->addWidget(m_lofar);
    m_demon=new DemonSpectrumPlot; m_demon->setObjectName("DirectionDemoDemonSpectrum"); m_demon->setMinimumHeight(300); m_demon->setLevelRange(-100,0); layout->addWidget(m_demon);
    m_demonHistory=new DemonHistoryPlot; m_demonHistory->setObjectName("DirectionDemoDemonHistory"); m_demonHistory->setMinimumHeight(320); m_demonHistory->setLevelRange(-100,-10); layout->addWidget(m_demonHistory);
    auto* explanation=new Label("DEMON 示例：fs=8192 Hz，4096 点，载频 2048 Hz，调制 32/64 Hz；带通 1800–2300 Hz，低通 100 Hz 后抽取 8。有限块处理有边缘效应。",content);
    explanation->setWordWrap(true); layout->addWidget(explanation); scroll->setWidget(content);
    auto recolor=[=] {
        if(lower->value()>=upper->value()) { output->setText("色标上限必须大于下限"); return; }
        m_history->setLevelRange(lower->value(),upper->value()); m_history->setDisplayThreshold(threshold->value(),enabled->isChecked());
    };
    connect(lower,QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,[=](double){ recolor(); });
    connect(upper,QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,[=](double){ recolor(); });
    connect(threshold,QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,[=](double){ recolor(); });
    connect(enabled,&QCheckBox::toggled,this,[=](bool){ recolor(); });
    connect(m_history,&HistoryHeatmapPlot::levelRangeChanged,this,[=](double lo,double hi) { const QSignalBlocker a(lower),b(upper); lower->setValue(lo); upper->setValue(hi); });
    connect(m_history,&HistoryHeatmapPlot::displayThresholdChanged,this,[=](double value,bool on) { const QSignalBlocker a(threshold),b(enabled); threshold->setValue(value); enabled->setChecked(on); });
    const QList<StyledPlot*> plots{m_beams,m_history,m_lofar,m_demon,m_demonHistory};
    connect(reset,&QPushButton::clicked,this,[plots] { for(auto* plot:plots) plot->resetView(); });
    for(auto* plot:plots) {
        // 业务可在此加入“导出片段 / 加入关注列表 / 调用算法”等动作，menu 只在当前弹出期间有效。
        connect(plot,&StyledPlot::prepareContextMenu,this,[plot,output](QMenu* menu,const QPointF& point) {
            menu->addSeparator(); auto* action=menu->addAction("业务扩展示例：标记此位置");
            connect(action,&QAction::triggered,output,[plot,output,point] { output->setText(QString("标记 %1：x=%2，y=%3").arg(plot->plotTitle()).arg(point.x(),0,'f',3).arg(point.y(),0,'f',3)); });
        });
        connect(plot,&StyledPlot::selectionMade,output,[plot,output](double a,double b,double c,double d) {
            output->setText(QString("%1 选区：x=[%2,%3] y=[%4,%5]").arg(plot->plotTitle()).arg(a,0,'f',2).arg(b,0,'f',2).arg(c,0,'f',2).arg(d,0,'f',2));
        });
    }
    connect(m_beams,&BeamPatternPlot::beamPicked,output,[output](const QString& id,double angle,double level) {
        output->setText(QString("%1 · 方位 %2° · 响应 %3 dB").arg(id).arg(angle,0,'f',1).arg(level,0,'f',1));
    });
    for(int i=0;i<64;++i) advance();
    m_timer=new QTimer(this); m_timer->setObjectName("DirectionDemoTimer"); m_timer->setInterval(150);
    connect(m_timer,&QTimer::timeout,this,&DirectionAnalysisPage::advance);
    connect(m_run,&QCheckBox::toggled,this,[this](bool on) { if(on&&isVisible())m_timer->start(); else m_timer->stop(); });
}
void DirectionAnalysisPage::advance()
{
    constexpr int n=4096,hop=1024; constexpr double rate=8192,pi=3.14159265358979323846;
    const double time=m_frame*.125;
    QVector<double> bearings(361),a(361),b(361),c(361),response(361);
    const double one=45+12*std::sin(time*.35),two=170+20*std::sin(time*.22),three=285-6*time;
    auto lobe=[](double angle,double center,double width,double height) { const double d=std::remainder(angle-center,360.); return -58+height*std::exp(-.5*d*d/(width*width)); };
    for(int i=0;i<=360;++i) { bearings[i]=i; a[i]=lobe(i,one,9,52); b[i]=lobe(i,two,14,46); c[i]=lobe(i,three,7,49); response[i]=std::max(a[i],std::max(b[i],c[i])); }
    m_beams->setBeam("beam-A","波束 A",bearings,a); m_beams->setBeam("beam-B","波束 B",bearings,b); m_beams->setBeam("beam-C","波束 C",bearings,c);
    if(m_frame%21!=8) m_history->appendFrame(time,response);
    QVector<double> samples(n);
    for(int i=0;i<n;++i) {
        const double t=(double(m_frame)*hop-(n-hop)+i)/rate;
        samples[i]=.5*(1+.45*std::cos(2*pi*32*t)+.15*std::cos(2*pi*64*t))*std::cos(2*pi*2048*t)
            +.1*std::sin(2*pi*96*t)+.04*std::cos(2*pi*(120*t+5*t*t));
    }
    AcousticSpectrumFrame raw; DemonResult envelope; DemonSettings settings;
    settings.carrierLowerHz=1800; settings.carrierUpperHz=2300; settings.maximumModulationHz=100; settings.decimation=8;
    if(AcousticAnalysis::spectrum(samples,rate,&raw)&&DemonAnalysis::analyze(samples,rate,settings,&envelope)) {
        if(m_frame==0) { m_lofar->configure(raw,64,.125,0,400); m_demonHistory->configure(envelope,64,.125); }
        const double center=(double(m_frame)*hop-(n-hop)+(n-1)/2.)/rate;
        if(m_frame%21!=8) { m_lofar->appendFrame(center,raw); m_demonHistory->appendResult(center,envelope); }
        m_demon->setResult(envelope);
    }
    ++m_frame;
}
void DirectionAnalysisPage::showEvent(QShowEvent* event) { StyledWidget::showEvent(event); if(m_run->isChecked())m_timer->start(); }
void DirectionAnalysisPage::hideEvent(QHideEvent* event) { m_timer->stop(); StyledWidget::hideEvent(event); }
