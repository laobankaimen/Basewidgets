#ifndef BASEWIDGETS_COMPASSGAUGE_H
#define BASEWIDGETS_COMPASSGAUGE_H
#include <basewidgets/Core/StyledWidget.h>
namespace basewidgets {
/** 航向可视化：北=0°，东=90°，数值顺时针增加。刻度固定，指针表示航向。
 * 有限值按 360° 归一化；NaN/Inf 切换为无数据。GUI 线程更新，不持有内部计时器。
 */
class ACSS_UI_EXPORT CompassGauge : public StyledWidget {
    Q_OBJECT
    Q_PROPERTY(double heading READ heading WRITE setHeading NOTIFY headingChanged)
public:
    explicit CompassGauge(QWidget* parent=nullptr);
    void setHeading(double degrees);
    double heading() const { return m_heading; }
    void setValue(int degrees) { setHeading(degrees); } ///< 兼容旧数值入口。
    void setTitle(const QString& title);
    QString directionText() const;
    bool valueAvailable() const { return m_available; }
    void setValueAvailable(bool available);
    QSize sizeHint() const override { return QSize(300,300); }
    QSize minimumSizeHint() const override { return QSize(180,210); }
signals:
    void headingChanged(double degrees);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    void refreshDescription();
    double m_heading=0;
    bool m_available=true;
    QString m_title;
};
}
#endif
