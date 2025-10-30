#pragma once

#include <QtAdvancedUI/Charts/ChartBase.h>
#include <QVector>
#include <QPointF>


namespace QtAdvancedUI {

class ACSS_UI_EXPORT LineChart : public ChartBase {
    Q_OBJECT
public:
    explicit LineChart(QWidget* parent = nullptr);

    void setMaxPoints(int maxPoints);
    // Compatibility wrappers for examples
    void setMaxPointCount(int count);
    void clear();
    void appendPoints(const QVector<QPointF>& pts);
    void appendPoint(const QPointF& pt);

protected:
    void updateCursorHint(const QPoint& pos) override;

private:
    void flushPending();

    QCPGraph* m_graph = nullptr;
    int m_maxPoints = 200000; // 大数据量默认
    QVector<double> m_pendingX;
    QVector<double> m_pendingY;
};

} // namespace QtAdvancedUI