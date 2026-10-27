/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_FRAMELESSWIDGETWINDOW_H
#define BASEWIDGETS_FRAMELESSWIDGETWINDOW_H

#include <basewidgets/QtAdvancedUIExport.h>
#include <basewidgets/Core/BaseWidgetInterface.h>

#include <QMainWindow>
#include <QShowEvent>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QPointer>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>

class QVBoxLayout;
class QMenuBar;
class QtAdvancedStylesheet;
class QMainWindow;

namespace basewidgets {

class CustomTitleBar;
/** 无边框主窗口：标题/菜单在顶部，工具栏/状态栏由 QMainWindow 原生区域管理。
 * 使用 setContentWidget 设置业务内容；不要绕过它替换 centralWidget/menuWidget。
 * 自动恢复热插拔后不可达窗口；应用使用 WindowPlacement::save/restore 分别持久化各窗口。
 * 可用 WindowPlacement::watch(this)->setRecoveryEnabled(false) 禁用自动位置恢复。
 */
class ACSS_UI_EXPORT FramelessWidgetWindow : public QMainWindow , public BaseWidgetInterface
{
    Q_OBJECT
public:
    explicit FramelessWidgetWindow(QWidget* parent = nullptr);
    ~FramelessWidgetWindow() override;

    /** 同步 QWidget::windowTitle 与自定义标题栏。 */
    void setTitle(const QString& title) override;
    /** 接管内容控件；替换时隐藏并 deleteLater 旧内容；nullptr 清空。 */
    void setContentWidget(QWidget* w) override;
    /** 实际业务内容的借用指针；尚未设置时为 nullptr。 */
    QWidget* contentWidget() const { return m_centralWidget; }
    /** 接管嵌入式菜单栏；替换时延迟删除旧菜单栏，nullptr 移除。 */
    void setMenuBar(QMenuBar* menuBar);
    QMenuBar* menuBar() const {return m_menuBar;};
    /** 替换本接口管理的顶部工具栏；其他工具栏可使用 QMainWindow::addToolBar。 */
    void setToolBar(QToolBar* toolBar);
    QToolBar* toolBar() const {return m_toolBar;};
    /** 交给 QMainWindow 管理状态栏和所有权；nullptr 移除。 */
    void setStatusBar(QStatusBar* statusBar);
    QStatusBar* statusBar() const {return m_statusBar;};
    /** 允许/禁止边缘缩放，不覆盖调用方的最小/最大尺寸限制。 */
    void setResizableEdgesEnabled(bool enabled = true);
    bool isResizeable() const {return m_bResizeable;};
    /** 设置边缘命中宽度，单位为 Qt 逻辑像素，最小 1。保留旧拼写以兼容。 */
    void setResizeableAreaWidth(int width = 5);
    /** 通过 Qt 切换窗口状态；F11 离开全屏时恢复进入前的最大化状态。 */
    void setFramelessWindowState(FramelessWindowState state) override;

public slots:
    void showFullScreen();
    void showEvent(QShowEvent *ev) override;
    void closeEvent(QCloseEvent *ev) override;
#ifdef Q_OS_WIN
    // 内容边距管理
    void setContentsMargins(const QMargins &margins);
    void setContentsMargins(int left, int top, int right, int bottom);
    QMargins contentsMargins() const;
    QRect contentsRect() const;
    void getContentsMargins(int *left, int *top, int *right, int *bottom) const;    
#elif defined(Q_OS_LINUX)

#endif

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, NativeEventResult *result) override;
    void changeEvent(QEvent* e) override;
#ifdef Q_OS_WIN
    bool nativeWindowsEvent(const QByteArray &eventType, void *message, NativeEventResult *result);
#elif defined(Q_OS_LINUX)
    bool eventFilter(QObject* watched, QEvent* event) override;
    bool nativeLinuxEvent(const QByteArray &eventType, void *message, NativeEventResult *result);
#endif

private:
    void initUI() override;

private:
    QWidget* m_contentHost = nullptr;
    QPointer<QWidget> m_centralWidget;
    QPointer<QMenuBar> m_menuBar;
    QWidget* m_menuBarWidget = nullptr;
    QPointer<QToolBar> m_toolBar;
    QPointer<QStatusBar> m_statusBar;
    QWidget* m_titleBar = nullptr;
#ifdef Q_OS_WIN
    Qt::WindowStates m_wasMaxmizedBeforeMinimized;
#elif defined(Q_OS_LINUX)
    Qt::Edges resizeRegionToEdges() const;
    enum ResizeRegion { NoneR, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
    ResizeRegion resizeRegionAt(const QPoint& position) const;
    void updateResizeCursor(ResizeRegion region);
    ResizeRegion m_resizeRegion = NoneR;
    bool m_resizing = false;
    QRect m_startGeo;
    QPoint m_dragStartPos;
#endif
    Qt::WindowStates m_preFullScreenState{Qt::WindowNoState};
    int m_borderWidth = 5;
    QMargins m_margins;
    QMargins m_frames;
    bool m_bJustMaximized = false;
    bool m_bResizeable = false;
    int m_titlebarHeight = 40;
};

} // namespace basewidgets

#endif // BASEWIDGETS_FRAMELESSWIDGETWINDOW_H
