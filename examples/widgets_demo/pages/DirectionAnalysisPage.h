#pragma once
#include <basewidgets/Core/StyledWidget.h>
class QTimer; class QCheckBox;
namespace basewidgets { class BeamPatternPlot; class BearingHistoryPlot; class LofarPlot; class DemonSpectrumPlot; class DemonHistoryPlot; }
/** 合成波束与已知 AM 信号示例，不接真实阵列，也不作目标判识。 */
class DirectionAnalysisPage : public basewidgets::StyledWidget {
public:
    explicit DirectionAnalysisPage(QWidget* parent=nullptr);
protected:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;
private:
    void advance();
    basewidgets::BeamPatternPlot* m_beams=nullptr;
    basewidgets::BearingHistoryPlot* m_history=nullptr;
    basewidgets::LofarPlot* m_lofar=nullptr;
    basewidgets::DemonSpectrumPlot* m_demon=nullptr;
    basewidgets::DemonHistoryPlot* m_demonHistory=nullptr;
    QTimer* m_timer=nullptr;
    QCheckBox* m_run=nullptr;
    int m_frame=0;
};
