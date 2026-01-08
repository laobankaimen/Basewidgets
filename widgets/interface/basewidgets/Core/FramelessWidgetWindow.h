/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_FRAMELESSWIDGETWINDOW_H
#define BASEWIDGETS_FRAMELESSWIDGETWINDOW_H

#include <basewidgets/QtAdvancedUIExport.h>

#include <QMainWindow>
#include <QShowEvent>
#include <QCloseEvent>
#include <QHBoxLayout>


enum FramelessWindowState {Maximized,Minimized,FullScreen,Normal};

class QVBoxLayout;
class QMenuBar;
class QtAdvancedStylesheet;
class QMainWindow;

namespace basewidgets {

class CustomTitleBar;


class ACSS_UI_EXPORT FramelessWidgetWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit FramelessWidgetWindow(QWidget* parent = nullptr);
    ~FramelessWidgetWindow() override;

    void setTitle(const QString& title);
    void setContentWidget(QWidget* w);
    QWidget* contentWidget() const { return m_contentHost; }

    QMenuBar* menuBar() const;
    void setMenuBar(QMenuBar* menuBar);
    void setResizableEdgesEnabled(bool enabled = true);
    bool isResizeable() const {return m_bResizeable;};
    void setResizeableAreaWidth(int width = 5);
signals:
    void onCloseClicked();
public slots:
    void showFullScreen();
    void setFramelessWindowState(FramelessWindowState state);
    void showEvent(QShowEvent *ev);
    void closeEvent(QCloseEvent *ev);
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
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    void changeEvent(QEvent* e) override;
#ifdef Q_OS_WIN
    bool nativeWindowsEvent(const QByteArray &eventType, void *message, long *result);
#elif defined(Q_OS_LINUX)
    bool eventFilter(QObject* watched, QEvent* event) override;
    bool nativeLinuxEvent(const QByteArray &eventType, void *message, long *result);
#endif

private:
    void initUI();

private:
    QWidget* m_contentHost = nullptr;
    QMenuBar* m_menuBar = nullptr;
    QWidget* m_topMenu = nullptr;
    QWidget* m_titleBar = nullptr;
    QVBoxLayout* mainLayout = nullptr;
#ifdef Q_OS_WIN
    Qt::WindowStates m_wasMaxmizedBeforeMinimized;
#elif defined(Q_OS_LINUX)
    enum ResizeRegion { NoneR, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
    ResizeRegion m_resizeRegion = NoneR;
    bool m_resizing = false;
    int m_resizeMargin = 6;
    QRect m_startGeo;
    QPoint m_dragStartPos;
    Qt::WindowStates m_preMinimizedState{Qt::WindowNoState};
    Qt::WindowStates m_preFullScreenState{Qt::WindowNoState};
    FramelessWindowState m_currentWindowState{Normal};
#endif
    int m_borderWidth = 5;
    QMargins m_margins;
    QMargins m_frames;
    bool m_bJustMaximized = false;
    bool m_bResizeable = false;
    int m_titlebarHeight = 40;
};

} // namespace basewidgets

#endif // BASEWIDGETS_FRAMELESSWIDGETWINDOW_H

