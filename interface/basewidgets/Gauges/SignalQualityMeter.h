#pragma once
#include <basewidgets/Core/StyledWidget.h>
#include <basewidgets/Acoustics/AcousticAnalysis.h>
namespace basewidgets {
/** ADC 归一化信号质量仪表：RMS/Peak dBFS、峰均比、DC、削波和缺样计数。
 * 这里的 1 是 ADC 满幅参考，满幅正弦 RMS=-3.01 dBFS；不能将此读数标为水声 SPL。
 * 整数 ADC 满量程不恰好归一化为 ±1 时，请显式传门限给 AcousticAnalysis::statistics，再 setStatistics。
 * setSamples 同步统计，只适合短块；大数据在工作线程计算 statistics 后 setStatistics。
 */
class ACSS_UI_EXPORT SignalQualityMeter : public StyledWidget {
    Q_OBJECT
public:
    explicit SignalQualityMeter(QWidget* parent=nullptr);
    void setChannelName(const QString& name);
    void setSamples(const QVector<double>& normalizedSamples);
    bool setStatistics(const SignalStatistics& statistics);
    const SignalStatistics& statistics() const { return m_statistics; }
    bool setDisplayRange(double lowerDb,double upperDb); ///< 仅改变仪表范围，不改变数据或削波阈值。
    QSize sizeHint() const override { return QSize(360,230); }
    QSize minimumSizeHint() const override { return QSize(260,210); }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QString m_name;
    SignalStatistics m_statistics;
    double m_lower=-96,m_upper=0;
};
}
