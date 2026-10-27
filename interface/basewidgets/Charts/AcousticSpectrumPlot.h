#pragma once
#include <basewidgets/Charts/StyledPlot.h>
#include <basewidgets/Acoustics/AcousticAnalysis.h>
namespace basewidgets {
/** 单边频谱/PSD 图。setFrame 接收计算结果，不在 UI 线程自动 FFT。
 * 峰值保持为逐频点最大值；采样率、FFT 长度、单位、窗函数、去均值、floor 或 scaling 改变时自动重置。
 * PSD 频带积分使用未施加 floor 的 linearPower，而不是直接累加 dB。
 */
class ACSS_UI_EXPORT AcousticSpectrumPlot : public StyledPlot {
    Q_OBJECT
public:
    explicit AcousticSpectrumPlot(QWidget* parent=nullptr);
    bool setFrame(const AcousticSpectrumFrame& frame);
    const AcousticSpectrumFrame& frame() const { return m_frame; }
    void setPeakHoldEnabled(bool enabled);
    void resetPeakHold();
    QVector<double> peakHoldLevels() const { return m_peak; }
    bool setFrequencyRange(double lowerHz,double upperHz);
    bool setLogFrequency(bool enabled); ///< 对数轴不显示 DC，自动从第一个正频点开始。
    bool setLevelRange(double lowerDb,double upperDb);
    double bandPowerDb(double lowerHz,double upperHz) const;
    double peakFrequencyHz() const;
private:
    QCPGraph *m_spectrum=nullptr,*m_hold=nullptr;
    AcousticSpectrumFrame m_frame;
    QVector<double> m_peak;
    bool m_peakEnabled=false,m_log=false;
};
}
