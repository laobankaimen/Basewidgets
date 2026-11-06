/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_TOGGLESWITCH_H
#define BASEWIDGETS_TOGGLESWITCH_H

#include <QAbstractButton>
#include <QPropertyAnimation>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 可选中的自绘开关。使用 setChecked/isChecked 和 toggled 信号；支持键盘 Space、RTL。
 * 动画绑定 toggled，因此程序设置和鼠标/键盘触发具有一致效果。
 * 默认固定尺寸策略；显式分配较大区域时图形居中，整个控件区域仍可点击。
 */
class ACSS_UI_EXPORT ToggleSwitch : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(qreal offset READ offset WRITE setOffset)
public:
    explicit ToggleSwitch(QWidget* parent = nullptr);
    ~ToggleSwitch() override;

    QSize sizeHint() const override;

    /** 临时覆盖颜色；后续 updateColors/主题切换会恢复主题色。 */
    void setOnColor(const QColor& c) { m_on = c; update(); }
    void setOffColor(const QColor& c) { m_off = c; update(); }
    void setKnobColor(const QColor& c) { m_knob = c; update(); }
    void updateColors();
    /** 状态切换动画时长（毫秒），最小为 60。 */
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

} // namespace basewidgets

#endif // BASEWIDGETS_TOGGLESWITCH_H

