/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_BUSYINDICATOR_H
#define BASEWIDGETS_BUSYINDICATOR_H

#include <basewidgets/Core/StyledWidget.h>
#include <basewidgets/QtAdvancedUIExport.h>

class QTimer;

namespace basewidgets {

// 旋转加载指示器（12段圆点），适合作为不确定加载动画
class ACSS_UI_EXPORT BusyIndicator : public StyledWidget {
    Q_OBJECT
public:
    explicit BusyIndicator(QWidget* parent = nullptr);
    ~BusyIndicator() override;

    /** 标记为运行；只有可见时启用计时器，页面隐藏后自动暂停。 */
    void start();
    /** 停止动画，重新显示时保持停止；再次 start 才恢复。 */
    void stop();
    bool isRunning() const { return m_running; }

    /** 显式颜色覆盖主题色，后续主题切换不会重置此颜色。 */
    void setColor(const QColor& c) { m_customColor = true; m_color = c; update(); }
    void setSegmentCount(int c) { m_segments = qMax(6, c); m_step %= m_segments; update(); }
    /** 帧间隔（毫秒），最小为 10；不改变运行/停止状态。 */
    void setInterval(int ms);

protected:
    void paintEvent(QPaintEvent* e) override;
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

private slots:
    void onTick();

private:
    QTimer* m_timer = nullptr;
    int m_step = 0;
    int m_segments = 10;
    int m_interval = 80; // ms
    bool m_running = true;
    bool m_customColor = false;
    QColor m_color = QColor(76, 217, 100); // 近似 iOS 绿
};

} // namespace basewidgets

#endif // BASEWIDGETS_BUSYINDICATOR_H

