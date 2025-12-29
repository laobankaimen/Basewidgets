/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef QTADVANCEDUI_BUSYINDICATOR_H
#define QTADVANCEDUI_BUSYINDICATOR_H

#include <QtAdvancedUI/Core/StyledWidget.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

class QTimer;

namespace QtAdvancedUI {

// 旋转加载指示器（12段圆点），适合作为不确定加载动画
class ACSS_UI_EXPORT BusyIndicator : public StyledWidget {
    Q_OBJECT
public:
    explicit BusyIndicator(QWidget* parent = nullptr);
    ~BusyIndicator() override;

    void start();
    void stop();
    bool isRunning() const { return m_running; }

    void setColor(const QColor& c) { m_color = c; }
    void setSegmentCount(int c) { m_segments = qMax(6, c); }
    void setInterval(int ms);

protected:
    void paintEvent(QPaintEvent* e) override;

private slots:
    void onTick();

private:
    QTimer* m_timer = nullptr;
    int m_step = 0;
    int m_segments = 10;
    int m_interval = 80; // ms
    bool m_running = true;
    QColor m_color = QColor(76, 217, 100); // 近似 iOS 绿
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_BUSYINDICATOR_H

