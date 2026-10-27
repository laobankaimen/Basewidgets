#pragma once
#include <basewidgets/Core/StyledWidget.h>
class QTimer;
class QCheckBox;
namespace basewidgets {
class AcousticWaveformPlot; class AcousticSpectrumPlot; class AcousticSpectrogramPlot;
class RecordingTimelinePlot; class SignalQualityMeter; class AcquisitionStatusPanel;
}
/** 可复现的模拟水声业务页。控件只接收数据；真实业务可在工作线程 FFT，再 queued signal 更新 UI。 */
class AcousticPage : public basewidgets::StyledWidget {
public:
    explicit AcousticPage(QWidget* parent=nullptr);
protected:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;
private:
    void appendDemoBlock();
    void updateStatus();
    basewidgets::AcousticWaveformPlot* m_wave=nullptr;
    basewidgets::AcousticSpectrumPlot* m_spectrum=nullptr;
    basewidgets::AcousticSpectrogramPlot* m_spectrogram=nullptr;
    basewidgets::RecordingTimelinePlot* m_timeline=nullptr;
    basewidgets::SignalQualityMeter* m_quality=nullptr;
    basewidgets::AcquisitionStatusPanel* m_status=nullptr;
    QTimer* m_timer=nullptr;
    QCheckBox* m_run=nullptr;
    quint64 m_frame=0;
};
