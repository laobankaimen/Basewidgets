#ifndef QTADVANCEDUI_TOGGLESWITCH_H
#define QTADVANCEDUI_TOGGLESWITCH_H

#include <QAbstractButton>
#include <QPropertyAnimation>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

// 滑动开关按钮，带动画
class ACSS_UI_EXPORT ToggleSwitch : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(qreal offset READ offset WRITE setOffset)
public:
    explicit ToggleSwitch(QWidget* parent = nullptr);
    ~ToggleSwitch() override;

    QSize sizeHint() const override;

    void setOnColor(const QColor& c) { m_on = c; }
    void setOffColor(const QColor& c) { m_off = c; }
    void setKnobColor(const QColor& c) { m_knob = c; }
    void updateColors();
    void setAnimationDuration(int ms);

protected:
    void paintEvent(QPaintEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    qreal offset() const { return m_offset; }
    void setOffset(qreal v);
    void animateTo(bool checked);
    void initSwitch();

private:
    QPropertyAnimation* m_anim = nullptr;
    qreal m_offset = 0.0;
    int m_w = 44;
    int m_h = 24;
    QColor m_on = QColor(76, 217, 100);
    QColor m_off = QColor(180, 180, 180);
    QColor m_knob = QColor(255, 255, 255);
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_TOGGLESWITCH_H