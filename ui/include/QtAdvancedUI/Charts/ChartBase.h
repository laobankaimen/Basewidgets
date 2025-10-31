#pragma once

#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QtAdvancedUI/Core/StyledWidget.h>
#include <QtAdvancedUI/Charts/QCustomplot.h>
#include <QPointer>
#include <QTimer>

class QMouseEvent;
class QWheelEvent;
class QResizeEvent;

namespace QtAdvancedUI {

// 通用图表基类：封装 QCustomPlot，并提供双击复位与光标跟踪
class ACSS_UI_EXPORT ChartBase : public StyledWidget {
    Q_OBJECT
public:
    explicit ChartBase(QWidget* parent = nullptr);
    ~ChartBase() override;

    QCustomPlot* plot() const; // moved out-of-line to avoid requiring full type here

    void setCursorTrackingEnabled(bool enable = true);
    void setDoubleClickResetEnabled(bool enable = true);
    void setUpdateIntervalMs(int ms);

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void resizeEvent(QResizeEvent* e) override;

    // 派生类钩子：首次数据就绪后捕获初始范围
    void captureInitialRangesIfNeeded();
    void restoreInitialRanges();

    // 派生类可覆盖以更新悬浮提示
    virtual void updateCursorHint(const QPoint& pos);

    // 触发刷新（批量更新场景下做节流）
    void requestReplotQueued();

protected:
    QPointer<QCustomPlot> m_plot;
    QPointer<QTimer> m_updateTimer;
    bool m_cursorTracking = true;
    bool m_resetOnDoubleClick = true;
    bool m_initialCaptured = false;
    double m_xLower = 0.0, m_xUpper = 1.0;
    double m_yLower = 0.0, m_yUpper = 1.0;
};

} // namespace QtAdvancedUI