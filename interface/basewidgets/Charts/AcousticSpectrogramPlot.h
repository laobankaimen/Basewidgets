#pragma once
#include <basewidgets/Charts/HistoryHeatmapPlot.h>
#include <basewidgets/Acoustics/AcousticAnalysis.h>
namespace basewidgets {
/** 带严格谱元信息的时频图。继承 HistoryHeatmapPlot 的色标/门限/菜单/选区/滚动能力。
 * FFT/fs/单位/scaling/window/去均值/floor 必须一致，改变处理参数需 configure。
 */
class ACSS_UI_EXPORT AcousticSpectrogramPlot : public HistoryHeatmapPlot {
    Q_OBJECT
public:
    explicit AcousticSpectrogramPlot(QWidget* parent=nullptr);
    bool configure(const AcousticSpectrumFrame& prototype,int historyFrames,double framePeriodSeconds);
    bool appendFrame(double centerTimeSeconds,const AcousticSpectrumFrame& frame);
    bool setFrequencyRange(double lowerHz,double upperHz);
protected:
    AcousticSpectrumFrame m_prototype;
};
}
