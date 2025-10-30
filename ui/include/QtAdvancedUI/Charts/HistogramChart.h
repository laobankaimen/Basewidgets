#pragma once

#include <QtAdvancedUI/Charts/ChartBase.h>
#include <QVector>


namespace QtAdvancedUI {

class ACSS_UI_EXPORT HistogramChart : public ChartBase {
    Q_OBJECT
public:
    explicit HistogramChart(QWidget* parent = nullptr);

    void configureBins(int binCount, double minValue, double maxValue);
    // Compatibility wrapper for examples
    void setBinCount(int binCount);
    void addSamples(const QVector<double>& values);
    void clear();

protected:
    void updateCursorHint(const QPoint& pos) override;

private:
    void updateBars();

    QCPBars* m_bars = nullptr;
    int m_binCount = 20;
    double m_minValue = 0.0;
    double m_maxValue = 1.0;
    QVector<int> m_bins; // 频数
};

} // namespace QtAdvancedUI