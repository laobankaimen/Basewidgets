#pragma once

#include <QtAdvancedUI/Charts/ChartBase.h>
#include <QVector>
#include <QPointF>


namespace QtAdvancedUI {

class ACSS_UI_EXPORT JourneyChart : public ChartBase {
    Q_OBJECT
public:
    explicit JourneyChart(QWidget* parent = nullptr);

    void setMaxPoints(int maxPoints);
    // Compatibility wrappers for examples
    void setMaxPointCount(int count);
    void clear();
    void appendPath(const QVector<QPointF>& pts);
    void appendPoints(const QVector<QPointF>& pts);

protected:
    void updateCursorHint(const QPoint& pos) override;

private:
    void flushPending();

    QCPGraph* m_graph = nullptr;
    int m_maxPoints = 200000;
    QVector<double> m_px;
    QVector<double> m_py;
};

} // namespace QtAdvancedUI