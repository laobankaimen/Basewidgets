#ifndef QTADVANCEDUI_CIRCULARGAUGE_H
#define QTADVANCEDUI_CIRCULARGAUGE_H

#include <QtAdvancedUI/Core/StyledWidget.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

// 圆形刻度与指针仪表盘
class ACSS_UI_EXPORT CircularGauge : public StyledWidget {
    Q_OBJECT
public:
    explicit CircularGauge(QWidget* parent = nullptr);
    explicit CircularGauge(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~CircularGauge() override;

    void setRange(double min, double max);
    void setValue(double v);
    double value() const { return m_value; }

    void setStartAngle(double deg); // 默认 225 度
    void setEndAngle(double deg);   // 默认 -45 度
    void setTickCount(int count);   // 默认 10
    void setUnit(const QString& unit);
    void setTitle(const QString& title);

    void setDialColor(const QColor& c) { m_dialColor = c; }
    void setNeedleColor(const QColor& c) { m_needleColor = c; }
    void setTickColor(const QColor& c) { m_tickColor = c; }

protected:
    void paintEvent(QPaintEvent* e) override;
    void applyStylesheet();

private:
    double m_min = 0.0;
    double m_max = 100.0;
    double m_value = 0.0;
    double m_startDeg = 225.0;
    double m_endDeg = -45.0;
    int m_tickCount = 10;
    QString m_unit;
    QString m_title;

    QColor m_dialColor = QColor(60,60,60);
    QColor m_needleColor = QColor(220, 0, 0);
    QColor m_tickColor = QColor(160,160,160);
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_CIRCULARGAUGE_H