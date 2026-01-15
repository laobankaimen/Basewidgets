/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QGraphicsDropShadowEffect>
#include <QMenuBar>
#include <QShortcut>
#include <QSettings>

#include "basewidgets/Charts/QCPBarChart.h"
#include "basewidgets/Dialogs/DialogUtils.h"
#include <basewidgets/Core/FramelessWidgetWindow.h>
#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winuser.h>
#include <windowsx.h>
#include <dwmapi.h>

#pragma comment (lib,"Dwmapi.lib")
#pragma comment (lib,"user32.lib")
#elif defined(Q_OS_LINUX)
#include <QMouseEvent>
#include <QResizeEvent>
#include <QApplication>
#include <QPoint>
#include <QTimer>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#endif

namespace basewidgets {

// Windows平台实现 - 复刻framelesswindow的代码
FramelessWidgetWindow::FramelessWidgetWindow(QWidget* parent)
    : QMainWindow(parent),
      m_bJustMaximized(false),
      m_bResizeable(true)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);

    initUI();
    setResizableEdgesEnabled(m_bResizeable);

    //获取初始化显示大小
    QSettings setting("ZKHX",objectName().isEmpty()?"MainWindow":objectName());
    restoreGeometry(setting.value("geometry").toByteArray());
}

void FramelessWidgetWindow::setResizeableAreaWidth(int width)
{
    if (1 > width) width = 1;
    m_borderWidth = width;
}

#ifdef Q_OS_WIN
bool FramelessWidgetWindow::nativeWindowsEvent(const QByteArray &eventType, void *message, long *result)
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
#else
    MSG* msg = reinterpret_cast<MSG*>(message);
#endif
    
    switch (msg->message)
    {
        case WM_NCCALCSIZE:
        {
            NCCALCSIZE_PARAMS& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
            if (params.rgrc[0].top != 0)
                params.rgrc[0].top -= 1;

            *result = WVR_REDRAW;
            return true;
        }
        case WM_NCHITTEST:
        {
            *result = 0;
            //全屏禁止拖动和改变大小
            if(windowState() == Qt::WindowFullScreen || windowState() == (Qt::WindowMaximized|Qt::WindowFullScreen)) {
                *result = HTCLIENT;
                return true;
            }

            const LONG border_width = m_borderWidth;
            RECT winrect;
            GetWindowRect(HWND(winId()), &winrect);

            long x = GET_X_LPARAM(msg->lParam);
            long y = GET_Y_LPARAM(msg->lParam);

            if(m_bResizeable)
            {
                bool resizeWidth = minimumWidth() != maximumWidth();
                bool resizeHeight = minimumHeight() != maximumHeight();

                if(resizeWidth)
                {
                    //left border
                    if (x >= winrect.left && x < winrect.left + border_width)
                    {
                        *result = HTLEFT;
                    }
                    //right border
                    if (x < winrect.right && x >= winrect.right - border_width)
                    {
                        *result = HTRIGHT;
                    }
                }
                if(resizeHeight)
                {
                    //bottom border
                    if (y < winrect.bottom && y >= winrect.bottom - border_width)
                    {
                        *result = HTBOTTOM;
                    }
                    //top border
                    if (y >= winrect.top && y < winrect.top + border_width)
                    {
                        *result = HTTOP;
                    }
                }
                if(resizeWidth && resizeHeight)
                {
                    //bottom left corner
                    if (x >= winrect.left && x < winrect.left + border_width &&
                            y < winrect.bottom && y >= winrect.bottom - border_width)
                    {
                        *result = HTBOTTOMLEFT;
                    }
                    //bottom right corner
                    if (x < winrect.right && x >= winrect.right - border_width &&
                            y < winrect.bottom && y >= winrect.bottom - border_width)
                    {
                        *result = HTBOTTOMRIGHT;
                    }
                    //top left corner
                    if (x >= winrect.left && x < winrect.left + border_width &&
                            y >= winrect.top && y < winrect.top + border_width)
                    {
                        *result = HTTOPLEFT;
                    }
                    //top right corner
                    if (x < winrect.right && x >= winrect.right - border_width &&
                            y >= winrect.top && y < winrect.top + border_width)
                    {
                        *result = HTTOPRIGHT;
                    }
                }
            }
            if (0!=*result) return true;

            if (m_titleBar) {
                double dpr = this->devicePixelRatioF();
                QPoint titlePos = m_titleBar->mapFromGlobal(QPoint(x/dpr,y/dpr));

                QWidget* child = m_titleBar->childAt(titlePos);
                //鼠标标题栏的子控件，让子控件接收事件（过滤掉其他不需要鼠标交互的控件）
                if (child && (child->objectName().contains("TitleBarCloseButton") ||
                    child->objectName().contains("TitleBarMaxButton") ||
                    child->objectName().contains("TitleBarMinButton"))) {
                    *result = HTCLIENT;
                    return true;
                    }
                //不在子控件上，且在标题栏内
                if (m_titleBar->geometry().contains(titlePos)) {
                    *result = HTCAPTION;
                    return true;
                }
            }
            return false;
        }
        case WM_GETMINMAXINFO:
        {
            if (::IsZoomed(msg->hwnd)) {
                RECT frame = { 0, 0, 0, 0 };
                AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);

                //record frame area data
                double dpr = this->devicePixelRatioF();

                m_frames.setLeft(abs(frame.left)/dpr+0.5);
                m_frames.setTop(abs(frame.bottom)/dpr+0.5);
                m_frames.setRight(abs(frame.right)/dpr+0.5);
                m_frames.setBottom(abs(frame.bottom)/dpr+0.5);

                QMainWindow::setContentsMargins(m_frames.left()+m_margins.left(),
                                                m_frames.top()+m_margins.top(),
                                                m_frames.right()+m_margins.right(),
                                                m_frames.bottom()+m_margins.bottom());
                m_bJustMaximized = true;
            }else {
                if (m_bJustMaximized)
                {
                    QMainWindow::setContentsMargins(m_margins);
                    m_frames = QMargins();
                    m_bJustMaximized = false;
                }
            }
            return false;
        }
        default:
            return QMainWindow::nativeEvent(eventType, message, result);
    }
}

void FramelessWidgetWindow::setContentsMargins(const QMargins &margins)
{
    QMainWindow::setContentsMargins(margins+m_frames);
    m_margins = margins;
}

void FramelessWidgetWindow::setContentsMargins(int left, int top, int right, int bottom)
{
    QMainWindow::setContentsMargins(left+m_frames.left(),
                                    top+m_frames.top(),
                                    right+m_frames.right(),
                                    bottom+m_frames.bottom());
    m_margins.setLeft(left);
    m_margins.setTop(top);
    m_margins.setRight(right);
    m_margins.setBottom(bottom);
}
QMargins FramelessWidgetWindow::contentsMargins() const
{
    QMargins margins = QMainWindow::contentsMargins();
    margins -= m_frames;
    return margins;
}
void FramelessWidgetWindow::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
    QMainWindow::getContentsMargins(left,top,right,bottom);
    if (!(left&&top&&right&&bottom)) return;
    if (isMaximized())
    {
        *left -= m_frames.left();
        *top -= m_frames.top();
        *right -= m_frames.right();
        *bottom -= m_frames.bottom();
    }
}
QRect FramelessWidgetWindow::contentsRect() const
{
    QRect rect = QMainWindow::contentsRect();
    int width = rect.width();
    int height = rect.height();
    rect.setLeft(rect.left() - m_frames.left());
    rect.setTop(rect.top() - m_frames.top());
    rect.setWidth(width);
    rect.setHeight(height);
    return rect;
}

#elif defined(Q_OS_LINUX)
//拖动缩放大小
bool FramelessWidgetWindow::eventFilter(QObject* watched, QEvent* event)
{
    QWidget* w = qobject_cast<QWidget*>(watched);
    if (!w || w->window() != this) return QMainWindow::eventFilter(watched, event);
    if (!m_bResizeable) return QMainWindow::eventFilter(watched, event);
    if (isMaximized()|| isFullScreen()) return QMainWindow::eventFilter(watched, event);

    switch (event->type()) {
    case QEvent::MouseMove: {
        auto* me = static_cast<QMouseEvent*>(event);
        const QPoint global = me->globalPos();
        const QPoint local = mapFromGlobal(global);
        const QRect r = rect();

        if (m_resizing) {
            QRect geo = m_startGeo;
            const QPoint d = global - m_dragStartPos;
            const int minW = minimumSize().width();
            const int minH = minimumSize().height();

            switch (m_resizeRegion) {
            case Left:       geo.setLeft(geo.left() + d.x()); break;
            case Right:      geo.setRight(geo.right() + d.x()); break;
            case Top:        geo.setTop(geo.top() + d.y()); break;
            case Bottom:     geo.setBottom(geo.bottom() + d.y()); break;
            case TopLeft:    geo.setTop(geo.top() + d.y()); geo.setLeft(geo.left() + d.x()); break;
            case TopRight:   geo.setTop(geo.top() + d.y()); geo.setRight(geo.right() + d.x()); break;
            case BottomLeft: geo.setBottom(geo.bottom() + d.y()); geo.setLeft(geo.left() + d.x()); break;
            case BottomRight:geo.setBottom(geo.bottom() + d.y()); geo.setRight(geo.right() + d.x()); break;
            default: break;
            }

            if (geo.width() < minW) geo.setWidth(minW);
            if (geo.height() < minH) geo.setHeight(minH);
            setGeometry(geo);
            return true;
        }

        ResizeRegion region = NoneR;
        const int m = m_resizeMargin;
        const bool onLeft   = local.x() <= m;
        const bool onRight  = local.x() >= r.width() - m;
        const bool onTop    = local.y() <= m;
        const bool onBottom = local.y() >= r.height() - m;
        if (onTop && onLeft) region = TopLeft;
        else if (onTop && onRight) region = TopRight;
        else if (onBottom && onLeft) region = BottomLeft;
        else if (onBottom && onRight) region = BottomRight;
        else if (onTop) region = Top;
        else if (onBottom) region = Bottom;
        else if (onLeft) region = Left;
        else if (onRight) region = Right;
        m_resizeRegion = region;

        switch (region) {
        case TopLeft:
        case BottomRight:
            setCursor(Qt::SizeFDiagCursor); break;
        case TopRight:
        case BottomLeft:
            setCursor(Qt::SizeBDiagCursor); break;
        case Left:
        case Right:
            setCursor(Qt::SizeHorCursor); break;
        case Top:
        case Bottom:
            setCursor(Qt::SizeVerCursor); break;
        default:
            unsetCursor(); break;
        }
        return QMainWindow::eventFilter(watched, event);
    }
    case QEvent::MouseButtonPress: {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton && m_resizeRegion != NoneR) {
            m_resizing = true;
            m_dragStartPos = me->globalPos();
            m_startGeo = frameGeometry();
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton && m_resizing) {
            m_resizing = false;
            m_resizeRegion = NoneR;
            unsetCursor();
            return true;
        }
        break;
    }
    default:
        break;
    }

    return QMainWindow::eventFilter(watched, event);
}

bool FramelessWidgetWindow::nativeLinuxEvent(const QByteArray &eventType, void *message, long *result)
{
    return QMainWindow::nativeEvent(eventType, message, result);
}


#endif

void FramelessWidgetWindow::setFramelessWindowState(FramelessWindowState state)
{
#ifdef Q_OS_WIN
    switch (state) {
        case Maximized:
            showMaximized();
            break;
        case Minimized:
            showMinimized();
            break;
        case FullScreen:
            showFullScreen();
            break;
        case Normal:
            showNormal();
            break;
    }
#elif defined(Q_OS_LINUX)
    Display *display = QX11Info::display();
    Window window = winId();

    Atom wmState = XInternAtom(display, "_NET_WM_STATE",false);
    Atom maxHorz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ",false);
    Atom maxVert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT",false);
    Atom hidden = XInternAtom(display, "_NET_WM_STATE_HIDDEN",false);
    Atom fullScreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN",false);

    XEvent xev;
    memset(&xev,0,sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = window;
    xev.xclient.message_type = wmState;
    xev.xclient.format = 32;

    switch (state) {
    case Maximized:
        xev.xclient.data.l[0] = 1;//_NET_WM_STATE_ADD
        xev.xclient.data.l[1] = maxHorz;
        xev.xclient.data.l[2] = maxVert;
        break;
    case Minimized:
        XIconifyWindow(display,window,0);
        break;
    case FullScreen:
        xev.xclient.data.l[0] = 1;
        xev.xclient.data.l[1] = fullScreen;
        xev.xclient.data.l[2] = 0;
        break;
    case Normal:
        //移除全屏
        xev.xclient.data.l[0] = 0;//_NET_WM_STATE_REMOVE
        xev.xclient.data.l[1] = fullScreen;
        xev.xclient.data.l[2] = 0;
        XSendEvent(display,DefaultRootWindow(display),false,SubstructureRedirectMask | SubstructureNotifyMask,&xev);
        //移除最大化
        xev.xclient.data.l[0] = 0;//_NET_WM_STATE_REMOVE
        xev.xclient.data.l[1] = maxHorz;
        xev.xclient.data.l[2] = maxVert;
        break;
    }
    XSendEvent(display,DefaultRootWindow(display),false,SubstructureRedirectMask | SubstructureNotifyMask,&xev);
    XFlush(display);
#endif

}

void FramelessWidgetWindow::showEvent(QShowEvent *ev)
{
    QMainWindow::showEvent(ev);
}

void FramelessWidgetWindow::closeEvent(QCloseEvent *ev)
{
    QSettings setting("ZKHX",objectName().isEmpty()?"MainWindow":objectName());
    setting.setValue("geometry",saveGeometry());
    //QMainWindow::closeEvent(ev);
    if(DialogUtils::askQuestion(this, "请确认", "是否关闭软件？")) {
        ev->accept();
    }
    else
        ev->ignore();
}

void FramelessWidgetWindow::showFullScreen()
{
    if (isMaximized())
    {
        QMainWindow::setContentsMargins(m_margins);
        m_frames = QMargins();
    }
    QMainWindow::showFullScreen();
}

FramelessWidgetWindow::~FramelessWidgetWindow()= default;

void FramelessWidgetWindow::initUI()
{
    // 创建CustomTitleBar作为标题栏
    auto* customTitleBar = new CustomTitleBar(this);
    customTitleBar->attachToWindow(this);
    customTitleBar->setFixedHeight(m_titlebarHeight);

    // 设置标题栏
    m_titleBar = customTitleBar;

    m_contentHost = new QWidget(this);
    auto* centralLayout = new QVBoxLayout(m_contentHost);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    m_menuBarWidget = new QWidget(this);
    auto* menuLayout = new QVBoxLayout(m_menuBarWidget);
    menuLayout->setContentsMargins(0, 0, 0, 0);
    m_toolBarWidget = new QWidget(this);
    auto* toolLayout = new QVBoxLayout(m_toolBarWidget);
    toolLayout->setContentsMargins(0, 0, 0, 0);
    m_statusBarWidget = new QWidget(this);
    auto* statusLayout = new QVBoxLayout(m_statusBarWidget);
    statusLayout->setContentsMargins(0, 0, 0, 0);

    auto centralWidget = new QWidget(this);
    mainLayout = new QVBoxLayout(centralWidget);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_menuBarWidget);
    mainLayout->addWidget(m_toolBarWidget);
    mainLayout->addWidget(m_contentHost, 1);
    mainLayout->addWidget(m_statusBarWidget);
    
    setCentralWidget(centralWidget);

    //通过快捷键全屏，避免误操作，全屏无动画效果
    auto *shortcut = new QShortcut(QKeySequence(tr("F11")),this);
    ::QObject::connect(shortcut,&QShortcut::activated,[this](){
        //恢复到全屏前的状态
        if(windowState() == Qt::WindowFullScreen || windowState() == (Qt::WindowMaximized|Qt::WindowFullScreen)) {
            this->setFramelessWindowState(Normal);
#ifdef Q_OS_LINUX
            setResizableEdgesEnabled(true);
#endif
        }
        else {
            this->setFramelessWindowState(FullScreen);
#ifdef Q_OS_LINUX
            setResizableEdgesEnabled(false);
#endif
        }
    });
}

void FramelessWidgetWindow::setTitle(const QString& title)
{
    // 由于m_titleBar是QWidget*类型，我们需要转换为CustomTitleBar*
    auto* customTitleBar = qobject_cast<CustomTitleBar*>(m_titleBar);
    if (customTitleBar) {
        customTitleBar->setTitle(title);
    }
}

void FramelessWidgetWindow::setContentWidget(QWidget* w)
{
    if(m_centralWidget && m_contentHost) {
        m_contentHost->layout()->removeWidget(m_centralWidget);
        m_centralWidget->deleteLater();
    }
    if (m_contentHost && w) {
        m_centralWidget = w;
        m_contentHost->layout()->addWidget(m_centralWidget);
    }
}

void FramelessWidgetWindow::setResizableEdgesEnabled(bool enabled)
{
    //if (m_bResizeable == enabled) return;
    bool visible = isVisible();
    m_bResizeable = enabled;
#ifdef Q_OS_WIN
    HWND hwnd = (HWND)this->winId();
    DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
    if (m_bResizeable){
        //此行代码可以带回Aero效果，同时也带回了标题栏和边框,在nativeEvent()会再次去掉标题栏
        setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
        ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    }else{
        setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
        ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
    }

    //圆角阴影会导致双击失效
    //const MARGINS shadow = { 1, 1, 1, 1 };
    //DwmExtendFrameIntoClientArea(hwnd, &shadow);
#elif defined(Q_OS_LINUX)
    if (m_bResizeable) {
        qApp->installEventFilter(this);
    } else {
        qApp->removeEventFilter(this);
        m_resizing = false;
        m_resizeRegion = NoneR;
        unsetCursor();
    }
#endif
    setVisible(visible);
}

void FramelessWidgetWindow::setMenuBar(QMenuBar* menuBar)
{
    if(m_menuBar && m_menuBarWidget) {
        m_menuBarWidget->layout()->removeWidget(m_menuBar);
        m_menuBar->deleteLater();
    }
    if (m_menuBarWidget && menuBar) {
        m_menuBar = menuBar;
        m_menuBarWidget->layout()->addWidget(m_menuBar);
    }
}

void FramelessWidgetWindow::setToolBar(QToolBar *toolBar) {
    if(m_toolBar && m_toolBarWidget) {
        m_toolBarWidget->layout()->removeWidget(m_toolBar);
        m_toolBar->deleteLater();
    }
    if (m_toolBarWidget && toolBar) {
        m_toolBar = toolBar;
        m_toolBarWidget->layout()->addWidget(m_toolBar);
    }
}

void FramelessWidgetWindow::setStatusBar(QStatusBar *statusBar) {
    if(m_statusBar && m_statusBarWidget) {
        m_statusBarWidget->layout()->removeWidget(m_statusBar);
        m_statusBar->deleteLater();
    }
    if (m_statusBarWidget && statusBar) {
        m_statusBar = statusBar;
        m_statusBarWidget->layout()->addWidget(m_statusBar);
    }
}

bool FramelessWidgetWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WIN
    return nativeWindowsEvent(eventType, message, result);
#elif defined(Q_OS_LINUX)
    return nativeLinuxEvent(eventType, message, result);
#else
    return QMainWindow::nativeEvent(eventType, message, result);
#endif
}

void FramelessWidgetWindow::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::WindowStateChange) {
        // 由于m_titleBar是QWidget*类型，我们需要转换为CustomTitleBar*
        auto* customTitleBar = qobject_cast<CustomTitleBar*>(m_titleBar);
        if (customTitleBar) {
            customTitleBar->updateForWindowState();
        }
    }
}

} // namespace basewidgets

