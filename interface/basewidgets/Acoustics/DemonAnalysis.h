#pragma once
#include <basewidgets/Acoustics/AcousticAnalysis.h>
namespace basewidgets {
/** 必须由业务选择载频带；带宽/调制带宽不是目标识别阈值。 */
struct ACSS_UI_EXPORT DemonSettings {
    double carrierLowerHz=0,carrierUpperHz=0;
    double maximumModulationHz=100;
    int decimation=8; ///< 2 的幂；先在频域低通再抽取，输出至少 16 点。
    AcousticWindow spectrumWindow=AcousticWindow::Hann;
    double floorDb=-160;
    bool operator==(const DemonSettings& other) const;
};
/** 归一化包络波动 (|analytic|/mean(|analytic|)-1) 的谱，不是原始声压/ADC 频谱。
 * 包络 PSD 参考 1/Hz；结果不自动推断轴频、叶片数、转速或目标类别。
 */
struct ACSS_UI_EXPORT DemonResult {
    DemonSettings settings;
    double inputSampleRate=0;
    double meanEnvelope=std::numeric_limits<double>::quiet_NaN(); ///< 外部后端可省略；未知用 NaN，不能填假的均值。
    int inputSampleCount=0;
    QVector<double> normalizedEnvelope; ///< 可选时域输出；只显示谱的后端可留空。
    AcousticSpectrumFrame spectrum;
    bool isValid() const;
};
/** 单块 DEMON 参考流程：带通选择→解析信号包络→均值归一化/去直流→低通→抽取→单边 PSD。
 * 频域滤波按有限块周期延拓，不是有状态流式 FIR；端点可能有边缘效应。
 * 推荐工作线程运行；生产滤波器/重叠策略可由已有 DSP 后端提供 DemonResult。
 */
class ACSS_UI_EXPORT DemonAnalysis {
public:
    static bool analyze(const QVector<double>& samples,double sampleRate,const DemonSettings& settings,
                        DemonResult* result,QString* error=nullptr);
};
}
Q_DECLARE_METATYPE(basewidgets::DemonSettings)
Q_DECLARE_METATYPE(basewidgets::DemonResult)
