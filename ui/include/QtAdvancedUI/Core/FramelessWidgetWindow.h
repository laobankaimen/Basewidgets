#ifndef QTADVANCEDUI_FRAMELESSWIDGETWINDOW_H
#define QTADVANCEDUI_FRAMELESSWIDGETWINDOW_H

#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include "qsystemdetection.h"

#include <QWidget>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPoint>
#include <QMainWindow>
#include <QWidget>
#include <QList>
#include <QMargins>
#include <QRect>
#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#elif defined(Q_OS_LINUX)

#endif

class QVBoxLayout;
class QMenuBar;
class QtAdvancedStylesheet;

namespace QtAdvancedUI {

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
    void setTitleBar(QWidget* titlebar);
    void addIgnoreWidget(QWidget* widget);
public slots:
    void showFullScreen();
#ifdef Q_OS_WIN
    // 内容边距管理
    void setContentsMargins(const QMargins &margins);
    void setContentsMargins(int left, int top, int right, int bottom);
    QMargins contentsMargins() const;
    QRect contentsRect() const;
    void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
#elif defined(Q_OS_LINUX)
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
#endif

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    void changeEvent(QEvent* e) override;
#ifdef Q_OS_WIN
    bool nativeWindowsEvent(const QByteArray &eventType, void *message, long *result);
#elif defined(Q_OS_LINUX)
    bool nativeLinuxEvent(const QByteArray &eventType, void *message, long *result);
#endif

private slots:
    void onTitleBarDestroyed();

private:
    void initUI();

private:
    QWidget* m_contentHost = nullptr;
    QMenuBar* m_menuBar = nullptr;

    QWidget* m_titlebar = Q_NULLPTR;
#ifdef Q_OS_WIN

#elif defined(Q_OS_LINUX)
    enum ResizeRegion { None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
    ResizeRegion m_resizeRegion = None;
    bool m_resizing = false;
    int m_resizeMargin = 6;
    QRect m_startGeo;
    QPoint m_dragStartPos;
    Qt::WindowStates m_preMinimizedState{Qt::WindowNoState};
    Qt::WindowStates m_preFullScreenState{Qt::WindowNoState};
#endif
    int m_borderWidth = 5;
    QMargins m_margins;
    QMargins m_frames;
    bool m_bJustMaximized = false;
    bool m_bResizeable = false;
    int m_titlebarHeight = 30;

};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_FRAMELESSWIDGETWINDOW_H
