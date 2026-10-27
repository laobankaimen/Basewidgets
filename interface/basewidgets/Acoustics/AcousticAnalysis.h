#ifndef BASEWIDGETS_ACOUSTICANALYSIS_H
#define BASEWIDGETS_ACOUSTICANALYSIS_H
#include <basewidgets/QtAdvancedUIExport.h>
#include <QVector>
#include <QString>
#include <QMetaType>
#include <limits>
namespace basewidgets {
/** 输入样本的实际单位；MicroPascal 只用于调用方已经完成校准的数据。 */
enum class AcousticUnit { FullScale, Volt, MicroPascal, RelativeEnvelope };
enum class SpectrumScaling { Power, PowerSpectralDensity };
enum class AcousticWindow { Rectangular, Hann };
struct ACSS_UI_EXPORT SignalStatistics {
    int sampleCount=0, validCount=0, invalidCount=0, clippedCount=0;
    double mean=0, rms=0, peak=0;
    double rmsDb=std::numeric_limits<double>::quiet_NaN();
    double peakDb=std::numeric_limits<double>::quiet_NaN();
    double crestFactorDb=std::numeric_limits<double>::quiet_NaN();
    bool valid() const { return validCount>0; }
};
/** 等间隔单边频谱。linearPower 为窗函数归一化后的功率/功率密度，未施加 dB floor。
 * frequency[k]=k*sampleRate/fftSize；输入长度必须正好为 fftSize，不进行隐式重采样或零填充。
 * samples 的单位由 unit 声明：FullScale 参考 1（满幅正弦 RMS=-3.01 dBFS），Volt 参考 1 V，
 * MicroPascal 参考 1 μPa。PSD 的分母额外含 Hz，不能当作宽带 SPL。
 */
struct ACSS_UI_EXPORT AcousticSpectrumFrame {
    double sampleRate=0;
    int fftSize=0;
    AcousticUnit unit=AcousticUnit::FullScale;
    SpectrumScaling scaling=SpectrumScaling::PowerSpectralDensity;
    AcousticWindow window=AcousticWindow::Hann;
    bool meanRemoved=true;
    double floorDb=-160;
    QVector<double> frequencies, levelsDb, linearPower;
    bool isValid() const;
    double binWidth() const { return fftSize>0 ? sampleRate/fftSize : 0; }
    QString levelLabel() const;
};
/** 无 QObject/GUI 依赖的计算辅助类，可在工作线程使用。控件本身仍必须在 GUI 线程更新。
 * 适合演示/单帧分析，不代替用户已有的多通道 DSP 后端。
 */
class ACSS_UI_EXPORT AcousticAnalysis {
public:
    static QString amplitudeLabel(AcousticUnit unit);
    static QString levelLabel(AcousticUnit unit, bool density=false);
    /** 统计忽略 NaN/Inf 并计数；clippingLevel 是明确的 ADC/量程阈值，<=0 时禁用削波统计。
     * 零输入 RMS/Peak dB 为 -Inf，crest 为 NaN；没有有效样本时 dB 为 NaN。
     */
    static SignalStatistics statistics(const QVector<double>& samples,double clippingLevel=1.0);
    /** radix-2 FFT，支持 16–65536 点。周期 Hann / rectangular；可移除均值。
     * PSD: factor*|FFT|²/(fs*sum(w²)); Power: factor*|FFT|²/sum(w)²。
     * DC/Nyquist factor=1，其他单边频点=2。失败不改变 output，error 返回原因。
     */
    static bool spectrum(const QVector<double>& samples,double sampleRate,AcousticSpectrumFrame* output,
        AcousticUnit unit=AcousticUnit::FullScale,SpectrumScaling scaling=SpectrumScaling::PowerSpectralDensity,
        AcousticWindow window=AcousticWindow::Hann,bool removeMean=true,double floorDb=-160,QString* error=nullptr);
    /** 仅对 PSD 积分；频带按频点中心闭区间选取。返回相对 1 unit² 的 dB；无有效频点返回 NaN。 */
    static double bandPowerDb(const AcousticSpectrumFrame& frame,double lowerHz,double upperHz);
    /** 线性校准比例必须由调用方给出（μPa/输入单位）；不猜测水听器灵敏度或前置增益。
     * 允许 NaN 缺样保留，拒绝 Inf/溢出。失败不改 output。
     */
    static bool scaleToMicroPascals(const QVector<double>& samples,double microPaPerUnit,QVector<double>* output);
};
}
Q_DECLARE_METATYPE(basewidgets::AcousticSpectrumFrame)
Q_DECLARE_METATYPE(basewidgets::SignalStatistics)
#endif
