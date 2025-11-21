#ifndef QTADVANCEDUI_FRAMELESSWIDGETWINDOW_H
#define QTADVANCEDUI_FRAMELESSWIDGETWINDOW_H

#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include "qsystemdetection.h"

#ifdef Q_OS_WIN
// Windows平台包含必要的头文件
#include <QMainWindow>
#include <QWidget>
#include <QList>
#include <QMargins>
#include <QRect>
#include <windows.h>
#include <winuser.h>
#include <windowsx.h>
#include <dwmapi.h>
#elif defined(Q_OS_LINUX)
// Linux平台使用QWidget
#include <QWidget>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPoint>
#endif

class QVBoxLayout;
class QMenuBar;
class QtAdvancedStylesheet;

namespace QtAdvancedUI {

class CustomTitleBar;

#ifdef Q_OS_WIN
class ACSS_UI_EXPORT FramelessWidgetWindow : public QMainWindow
#elif defined(Q_OS_LINUX)
class ACSS_UI_EXPORT FramelessWidgetWindow : public QWidget
#endif
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
    void setResizableEdgesEnabled(bool enabled);

#ifdef Q_OS_WIN
    // Windows平台特定方法 - 来自framelesswindow
    void setResizeable(bool resizeable = true);
    bool isResizeable() const;
    void setResizeableAreaWidth(int width = 5);
    void setTitleBar(QWidget* titlebar);
    void addIgnoreWidget(QWidget* widget);
    bool isMaximized() const;
    
    // 添加framelesswindow的内容边距管理方法
    void setContentsMargins(const QMargins &margins);
    void setContentsMargins(int left, int top, int right, int bottom);
    QMargins contentsMargins() const;
    QRect contentsRect() const;
    void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
    
public slots:
    void showFullScreen();
    
#elif defined(Q_OS_LINUX)
    // Linux平台特定方法
    enum ResizeRegion { None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
#endif

protected:
    void showEvent(QShowEvent* e) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void changeEvent(QEvent* e) override;

#ifdef Q_OS_WIN
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#elif defined(Q_OS_LINUX)
    // Linux平台特定事件处理
    bool handleResizeMouseEvent(QMouseEvent* event);
    void updateCursor(ResizeRegion region);
    void handleResize(const QPoint& globalPos);
    ResizeRegion calculateResizeRegion(const QPoint& globalPos, const QRect& frameRect);
#endif

private slots:
#ifdef Q_OS_WIN
    void onTitleBarDestroyed();
#endif

private:
    void initUI();

private:
    QWidget* m_contentHost = nullptr;
    QMenuBar* m_menuBar = nullptr;

#ifdef Q_OS_WIN
    // Windows平台特定成员
    QWidget* m_titlebar = Q_NULLPTR;
    QList<QWidget*> m_whiteList;
    int m_borderWidth = 5;
    QMargins m_margins;
    QMargins m_frames;
    bool m_bJustMaximized = false;
    bool m_bResizeable = true;
    int m_titlebarHeight = 30;
#elif defined(Q_OS_LINUX)
    // Linux平台特定成员
    ResizeRegion m_resizeRegion = None;
    bool m_resizing = false;
    QPoint m_resizeStartPos;
    QRect m_resizeStartGeometry;
    int m_borderWidth = 5;
    int m_titlebarHeight = 30;
    bool m_bResizeable = true;
#endif
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_FRAMELESSWIDGETWINDOW_H