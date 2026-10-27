#pragma once
#include <basewidgets/Charts/AcousticSpectrogramPlot.h>
#include <basewidgets/Charts/AcousticSpectrumPlot.h>
#include <basewidgets/Acoustics/DemonAnalysis.h>
namespace basewidgets {
/** LOFAR = 原始声学信号的窄带 PSD 历程；频带裁剪仅改变显示，不冒充额外解调。 */
class ACSS_UI_EXPORT LofarPlot : public AcousticSpectrogramPlot {
    Q_OBJECT
public:
    explicit LofarPlot(QWidget* parent=nullptr);
    bool configure(const AcousticSpectrumFrame& prototype,int historyFrames,double period,double lowerHz,double upperHz);
    bool appendFrame(double centerTime,const AcousticSpectrumFrame& frame);
};
/** DEMON 当前谱，横轴是调制频率而不是载频。 */
class ACSS_UI_EXPORT DemonSpectrumPlot : public AcousticSpectrumPlot {
    Q_OBJECT
public:
    explicit DemonSpectrumPlot(QWidget* parent=nullptr);
    bool setResult(const DemonResult& result);
private:
    DemonSettings m_settings;
    bool m_configured=false;
};
/** DEMON 历程：载频带/抽取等参数变化必须重新 configure，不能混合不同解调条件。 */
class ACSS_UI_EXPORT DemonHistoryPlot : public AcousticSpectrogramPlot {
    Q_OBJECT
public:
    explicit DemonHistoryPlot(QWidget* parent=nullptr);
    bool configure(const DemonResult& prototype,int historyFrames,double framePeriodSeconds);
    bool appendResult(double centerTime,const DemonResult& result);
private:
    DemonSettings m_settings;
    double m_inputRate=0;
    int m_inputCount=0;
};
}
