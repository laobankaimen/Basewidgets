#pragma once
#include <basewidgets/Charts/StyledPlot.h>
#include <basewidgets/Acoustics/AcousticAnalysis.h>
#include <cmath>
namespace basewidgets {
/** 有界多通道时域图。时间输入单位秒，必须由调用方选定同一时间基准。
 * configure 清空历史；appendSamples 拒绝重叠/倒序块，跨块缺口以 NaN 断线，不伪造连续数据。
 * 显示按像素保首尾/峰谷抽样，保留容量内的原始样点；测量/拟合使用原始数据。
 * 基类默认合并至 30 FPS；数据采集频率与展示帧率独立，不隐式重采样原始信号。
 */
class ACSS_UI_EXPORT AcousticWaveformPlot : public StyledPlot {
    Q_OBJECT
public:
    explicit AcousticWaveformPlot(QWidget* parent=nullptr);
    bool configure(double sampleRate,int samplesPerChannel=65536,AcousticUnit unit=AcousticUnit::FullScale);
    int addChannel(const QString& name,const QColor& color=QColor()); ///< 最多 16 通道；失败 -1。
    bool appendSamples(int channel,double startSeconds,const QVector<double>& samples);
    void clearSamples();
    int channelCount() const { return m_channels.size(); }
    int storedPointCount(int channel) const; ///< 包含用于表示缺口的断线点，始终 <= 配置容量。
    double sampleRate() const { return m_sampleRate; }
    void setChannelVisible(int channel,bool visible);
    void setFollowLatest(bool enabled); ///< 拖动/缩放时自动停止跟随，避免数据刷新抢回视口。
    bool followsLatest() const { return m_follow; }
    bool setVisibleDuration(double seconds);
    bool setCursors(double aSeconds,double bSeconds);
    double cursorDeltaSeconds() const { return std::abs(m_cursorB-m_cursorA); }
    void setCursorModeEnabled(bool enabled); ///< 开启后左键依次放置 A/B，禁用左键平移，滚轮仍缩放。
    bool fitAmplitude();
signals:
    void cursorsChanged(double aSeconds,double bSeconds);
    void followLatestChanged(bool enabled);
private:
    struct Channel { QCPGraph* graph=nullptr; bool hasData=false; double lastTime=0; };
    QVector<Channel> m_channels;
    QCPItemStraightLine *m_lineA=nullptr,*m_lineB=nullptr;
    double m_sampleRate=48000,m_duration=1,m_latest=0,m_cursorA=0,m_cursorB=0;
    int m_capacity=65536;
    bool m_follow=true,m_cursorMode=false,m_nextCursorB=false,m_hasLatest=false;
};
}
