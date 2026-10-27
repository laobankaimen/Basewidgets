// 所有信号、文件名和存储数字均为演示数据，不接入本机采集卡或真实录音。
// 展示约定：ADC 数据按满幅 1 归一化，频谱使用周期 Hann 的单边 PSD，不伪称校准 SPL。
#include "AcousticPage.h"
#include <basewidgets/Acoustics/AcousticAnalysis.h>
#include <basewidgets/Charts/AcousticWaveformPlot.h>
#include <basewidgets/Charts/AcousticSpectrumPlot.h>
#include <basewidgets/Charts/AcousticSpectrogramPlot.h>
#include <basewidgets/Charts/RecordingTimelinePlot.h>
#include <basewidgets/Gauges/SignalQualityMeter.h>
#include <basewidgets/Gauges/AcquisitionStatusPanel.h>
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Inputs/Label.h>
#include <QTimer>
#include <QSignalBlocker>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QShowEvent>
#include <QHideEvent>
#include <cmath>
using namespace basewidgets;
namespace {
constexpr double rate=24000,pi=3.14159265358979323846;
constexpr int fftSize=1024,hop=256;
double primary(double t)
{
    // 750 Hz 稳定线谱 + 1.5 秒周期线性扫频 + 两个弱背景分量。
    const double u=t-std::floor(t/1.5)*1.5;
    return .42*std::sin(2*pi*750*t)+.18*std::sin(2*pi*(1800*u+1200*u*u))
        +.013*std::sin(2*pi*9173*t)+.008*std::sin(2*pi*5381*t);
}
}
AcousticPage::AcousticPage(QWidget* parent) : StyledWidget(parent)
{
    auto* outer=new QVBoxLayout(this); outer->setContentsMargins(10,10,10,10);
    auto* description=new Label("模拟水声数据 · 24 kHz / 2 通道 · Hann 1024 点 · hop 256 · 未校准，单位 dBFS/Hz",this);
    description->setWordWrap(true); outer->addWidget(description);
    auto* controls=new QHBoxLayout;
    m_run=new CheckBox("继续模拟采集",this); m_run->setObjectName("AcousticDemoRun"); controls->addWidget(m_run);
    auto* follow=new CheckBox("跟随最新波形",this); follow->setChecked(true); controls->addWidget(follow);
    auto* cursors=new CheckBox("双游标测量",this); controls->addWidget(cursors);
    auto* hold=new CheckBox("频谱峰值保持",this); controls->addWidget(hold);
    auto* reset=new PushButton("重置峰值",this); reset->setAppearance(ButtonAppearance::Subtle); controls->addWidget(reset);
    controls->addStretch(); outer->addLayout(controls);
    auto* measurement=new Label("先打开双游标，再在波形上点击 A/B：显示 Δt。滚轮/拖动会停止跟随。",this);
    measurement->setWordWrap(true); outer->addWidget(measurement);
    auto* scroll=new ScrollArea(this); scroll->setFrameShape(QFrame::NoFrame); outer->addWidget(scroll,1);
    auto* content=new StyledWidget; auto* layout=new QVBoxLayout(content); layout->setContentsMargins(0,0,0,0); layout->setSpacing(14);
    auto* metrics=new QWidget; auto* metricLayout=new ResponsiveGridLayout(metrics,280,2); metricLayout->setContentsMargins(0,0,0,0);
    m_quality=new SignalQualityMeter; m_quality->setChannelName("CH1 · ADC 信号质量"); m_quality->setFixedHeight(220);
    m_status=new AcquisitionStatusPanel; m_status->setFixedHeight(220); metricLayout->addWidget(m_quality); metricLayout->addWidget(m_status); layout->addWidget(metrics);
    m_wave=new AcousticWaveformPlot; m_wave->setObjectName("AcousticDemoWaveform"); m_wave->setMinimumHeight(290);
    m_wave->configure(rate,32768); m_wave->addChannel("CH1 · 线谱+扫频"); m_wave->addChannel("CH2 · 含削波示例");
    m_wave->setVisibleDuration(.02); layout->addWidget(m_wave);
    m_spectrum=new AcousticSpectrumPlot; m_spectrum->setObjectName("AcousticDemoSpectrum"); m_spectrum->setMinimumHeight(290); m_spectrum->setLevelRange(-120,0); layout->addWidget(m_spectrum);
    auto* spectrumHint=new Label("PSD 与每频点功率不是同一量纲。频带功率需在线性域积分；未提供水听器标定时不显示 dB re 1 μPa。",content);
    spectrumHint->setWordWrap(true); layout->addWidget(spectrumHint);
    m_spectrogram=new AcousticSpectrogramPlot; m_spectrogram->setObjectName("AcousticDemoSpectrogram"); m_spectrogram->setMinimumHeight(360); m_spectrogram->setLevelRange(-110,-20); layout->addWidget(m_spectrogram);
    auto* gapHint=new Label("每 23 帧跳过一个分析帧来演示空行（原始采集样点仍完整）。横轴 Hz，纵轴帧中心秒，色标固定。",content);
    gapHint->setWordWrap(true); layout->addWidget(gapHint);
    m_timeline=new RecordingTimelinePlot; m_timeline->setObjectName("AcousticDemoTimeline"); m_timeline->setMinimumHeight(270); layout->addWidget(m_timeline);
    m_timeline->setRecords({{"rec-01","CH1","CH1_001.wav",0,18,RecordingState::Ready},
        {"rec-02","CH1","CH1_002.wav",22,43,RecordingState::Ready},{"rec-03","CH1","CH1_003.wav",45,60,RecordingState::Processing},
        {"rec-04","CH2","CH2_001.wav",0,18,RecordingState::Ready},{"rec-05","CH2","CH2_002.wav",24,40,RecordingState::Flagged},
        {"rec-06","CH2","CH2_003.wav",45,60,RecordingState::Processing}});
    auto* selection=new Label("录音记录状态：蓝=就绪，黄=处理中，红=待复核。点击记录返回 ID，可接业务文件列表/播放器。",content);
    selection->setWordWrap(true); layout->addWidget(selection);
    connect(m_timeline,&RecordingTimelinePlot::recordSelected,selection,[this,selection](const QString& id) {
        for(const auto& record:m_timeline->records()) if(record.id==id)
            selection->setText(QString("%1 · %2 · %3–%4 s · ID=%5").arg(record.label,record.channel).arg(record.startSeconds).arg(record.endSeconds).arg(id));
    });
    scroll->setWidget(content);
    connect(follow,&QCheckBox::toggled,m_wave,&AcousticWaveformPlot::setFollowLatest);
    connect(m_wave,&AcousticWaveformPlot::followLatestChanged,follow,&QCheckBox::setChecked);
    connect(cursors,&QCheckBox::toggled,m_wave,&AcousticWaveformPlot::setCursorModeEnabled);
    connect(m_wave,&StyledPlot::interactionModeChanged,cursors,[cursors](StyledPlot::InteractionMode mode) {
        const QSignalBlocker block(cursors); cursors->setChecked(mode==StyledPlot::InteractionMode::Inspect);
    });
    connect(m_wave,&AcousticWaveformPlot::cursorsChanged,measurement,[measurement](double a,double b) {
        measurement->setText(QString("A=%1 s · B=%2 s · Δt=%3 ms").arg(a,0,'f',6).arg(b,0,'f',6).arg(std::abs(b-a)*1000,0,'f',3));
    });
    connect(hold,&QCheckBox::toggled,m_spectrum,&AcousticSpectrumPlot::setPeakHoldEnabled);
    connect(reset,&QPushButton::clicked,m_spectrum,&AcousticSpectrumPlot::resetPeakHold);
    // 初始快照让页面立即有数据；帧中心时间与样点时钟一致，不使用定时器墙钟替代采样时间。
    for(int i=0;i<100;++i) appendDemoBlock();
    m_timer=new QTimer(this); m_timer->setObjectName("AcousticDemoTimer"); m_timer->setInterval(80);
    connect(m_timer,&QTimer::timeout,this,&AcousticPage::appendDemoBlock);
    connect(m_run,&QCheckBox::toggled,this,[this](bool enabled) { if(enabled && isVisible()) m_timer->start(); else m_timer->stop(); updateStatus(); });
    updateStatus();
}
void AcousticPage::appendDemoBlock()
{
    const double start=double(m_frame)*hop/rate;
    QVector<double> first(hop),second(hop),window(fftSize);
    for(int i=0;i<hop;++i) {
        const double t=start+i/rate; first[i]=primary(t);
        second[i]=qBound(-1.,1.12*std::sin(2*pi*1500*t),1.);
    }
    for(int i=0;i<fftSize;++i) window[i]=primary(start+(i-(fftSize-hop))/rate);
    AcousticSpectrumFrame frame;
    if(!AcousticAnalysis::spectrum(window,rate,&frame)) return;
    if(m_frame==0) m_spectrogram->configure(frame,96,hop/rate);
    m_wave->appendSamples(0,start,first); m_wave->appendSamples(1,start,second);
    m_spectrum->setFrame(frame); m_quality->setSamples(window);
    if(m_frame%23!=12) m_spectrogram->appendFrame(start+(hop-1-(fftSize-1)/2.)/rate,frame);
    ++m_frame; updateStatus();
}
void AcousticPage::updateStatus()
{
    AcquisitionStatus status; status.state=m_run->isChecked()?AcquisitionState::Recording:AcquisitionState::Paused;
    status.sampleRate=rate; status.channels=2; status.receivedSamples=quint64(m_frame)*hop*2;
    // 示例快照：容量并非读取本机磁盘，速率是模拟采集的 PCM float32 名义速率。
    status.totalBytes=quint64(100)*1024*1024*1024; status.freeBytes=quint64(64)*1024*1024*1024;
    status.writeBytesPerSecond=m_run->isChecked()?rate*2*4:0;
    m_status->setStatus(status);
}
void AcousticPage::showEvent(QShowEvent* event) { StyledWidget::showEvent(event); if(m_run->isChecked()) m_timer->start(); }
void AcousticPage::hideEvent(QHideEvent* event) { m_timer->stop(); StyledWidget::hideEvent(event); }
