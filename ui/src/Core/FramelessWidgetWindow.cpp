#include <QtAdvancedUI/Core/FramelessWidgetWindow.h>
#include <QtAdvancedUI/Core/CustomTitleBar.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QGraphicsDropShadowEffect>
#include <QRegion>
#include <QResizeEvent>
#include <QMenuBar>
#include <qshortcut.h>

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
#include <QPoint>

// X11 原生支持
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xcomposite.h>

// Wayland 支持检测
#ifdef QT_WAYLANDCLIENT_LIB
#include <QtWaylandClient/QWaylandClientExtension>
#include <qwayland-xdg-decoration-unstable-v1.h>
#include <qwayland-xdg-shell.h>
#endif

#endif

namespace QtAdvancedUI {


// Windows平台实现 - 复刻framelesswindow的代码
FramelessWidgetWindow::FramelessWidgetWindow(QWidget* parent)
    : QMainWindow(parent),
      m_titlebar(Q_NULLPTR),
      m_contentHost(nullptr),
      m_borderWidth(5),
      m_bJustMaximized(true),
      m_bResizeable(true),
      m_titlebarHeight(36),
      m_menuBar(nullptr)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    
    setResizableEdgesEnabled(m_bResizeable);
    initUI();
}

void FramelessWidgetWindow::setResizeableAreaWidth(int width)
{
    if (1 > width) width = 1;
    m_borderWidth = width;
}

void FramelessWidgetWindow::setTitleBar(QWidget* titlebar)
{
    m_titlebar = titlebar;
    if (!titlebar) return;
    connect(titlebar, SIGNAL(destroyed(QObject*)), this, SLOT(onTitleBarDestroyed()));
}

void FramelessWidgetWindow::onTitleBarDestroyed()
{
    if (m_titlebar == QObject::sender())
    {
        m_titlebar = Q_NULLPTR;
    }
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
                if (x >= winrect.left && x < winrect.left + border_width)
                {
                    *result = HTLEFT;
                }
                if (x < winrect.right && x >= winrect.right - border_width)
                {
                    *result = HTRIGHT;
                }
            }
            if(resizeHeight)
            {
                if (y < winrect.bottom && y >= winrect.bottom - border_width)
                {
                    *result = HTBOTTOM;
                }
                if (y >= winrect.top && y < winrect.top + border_width)
                {
                    *result = HTTOP;
                }
            }
            if(resizeWidth && resizeHeight)
            {
                if (x >= winrect.left && x < winrect.left + border_width &&
                        y < winrect.bottom && y >= winrect.bottom - border_width)
                {
                    *result = HTBOTTOMLEFT;
                }
                if (x < winrect.right && x >= winrect.right - border_width &&
                        y < winrect.bottom && y >= winrect.bottom - border_width)
                {
                    *result = HTBOTTOMRIGHT;
                }
                if (x >= winrect.left && x < winrect.left + border_width &&
                        y >= winrect.top && y < winrect.top + border_width)
                {
                    *result = HTTOPLEFT;
                }
                if (x < winrect.right && x >= winrect.right - border_width &&
                        y >= winrect.top && y < winrect.top + border_width)
                {
                    *result = HTTOPRIGHT;
                }
            }
        }
        if (0!=*result) return true;
        
        if (m_titlebar) {
            double dpr = this->devicePixelRatioF();
            QPoint titlePos = m_titlebar->mapFromGlobal(QPoint(x/dpr,y/dpr));

            QWidget* child = m_titlebar->childAt(titlePos);
            //鼠标标题栏的子控件，让子控件接收事件（过滤掉其他不需要鼠标交互的控件）
            if (child && (child->objectName().contains("TitleBarCloseButton") ||
                child->objectName().contains("TitleBarMaxButton") ||
                child->objectName().contains("TitleBarMinButton"))) {
                *result = HTCLIENT;
                return true;
            }
            //不在子控件上，且在标题栏内
            if (m_titlebar->geometry().contains(titlePos)) {
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
// Linux平台实现 - 使用原生X11/Wayland接口

// X11原子属性
static Atom _NET_WM_WINDOW_TYPE = 0;
static Atom _NET_WM_WINDOW_TYPE_NORMAL = 0;
static Atom _NET_WM_STATE = 0;
static Atom _NET_WM_STATE_MAXIMIZED_VERT = 0;
static Atom _NET_WM_STATE_MAXIMIZED_HORZ = 0;
static Atom _NET_WM_STATE_FULLSCREEN = 0;
static Atom _NET_WM_STATE_FOCUSED = 0;
static Atom _NET_WM_MOVERESIZE = 0;
static Atom _NET_FRAME_EXTENTS = 0;
static Atom _NET_REQUEST_FRAME_EXTENTS = 0;
static Atom WM_DELETE_WINDOW = 0;
static Atom WM_PROTOCOLS = 0;

class FramelessWidgetWindowPrivate {
public:
    Display* display = nullptr;
    Window x11Window = 0;
    bool compositingEnabled = false;
    bool waylandMode = false;
    
    // 初始化X11原子
    void initializeX11Atoms(Display* dpy) {
        if (_NET_WM_WINDOW_TYPE != 0) return; // 已经初始化
        
        display = dpy;
        
        _NET_WM_WINDOW_TYPE = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
        _NET_WM_WINDOW_TYPE_NORMAL = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NORMAL", False);
        _NET_WM_STATE = XInternAtom(dpy, "_NET_WM_STATE", False);
        _NET_WM_STATE_MAXIMIZED_VERT = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
        _NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
        _NET_WM_STATE_FULLSCREEN = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
        _NET_WM_STATE_FOCUSED = XInternAtom(dpy, "_NET_WM_STATE_FOCUSED", False);
        _NET_WM_MOVERESIZE = XInternAtom(dpy, "_NET_WM_MOVERESIZE", False);
        _NET_FRAME_EXTENTS = XInternAtom(dpy, "_NET_FRAME_EXTENTS", False);
        _NET_REQUEST_FRAME_EXTENTS = XInternAtom(dpy, "_NET_REQUEST_FRAME_EXTENTS", False);
        WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        WM_PROTOCOLS = XInternAtom(dpy, "WM_PROTOCOLS", False);
    }
    
    // 使用X11协议请求窗口管理器调整大小
    void requestWindowResize(Window win, int direction, int x_root, int y_root) {
        if (!display || _NET_WM_MOVERESIZE == 0) return;
        
        // 确保窗口管理器支持EWMH
        if (!compositingEnabled) {
            // 检查是否支持_compositing
            int event_base, error_base;
            if (XCompositeQueryExtension(display, &event_base, &error_base)) {
                compositingEnabled = true;
            }
        }
        
        XEvent event;
        memset(&event, 0, sizeof(event));
        
        event.xclient.type = ClientMessage;
        event.xclient.serial = 0;
        event.xclient.send_event = True;
        event.xclient.display = display;
        event.xclient.window = win;
        event.xclient.message_type = _NET_WM_MOVERESIZE;
        event.xclient.format = 32;
        
        event.xclient.data.l[0] = x_root; // 根坐标X
        event.xclient.data.l[1] = y_root; // 根坐标Y
        event.xclient.data.l[2] = direction; // 调整方向
        event.xclient.data.l[3] = Button1; // 鼠标按钮
        event.xclient.data.l[4] = 0; // 源指示器
        
        XSendEvent(display, DefaultRootWindow(display), False,
                  SubstructureRedirectMask | SubstructureNotifyMask, &event);
        XFlush(display);
        
        qDebug() << "X11 resize request sent:" << direction << "at" << x_root << y_root;
    }
    
    // 获取窗口框架扩展
    QMargins getFrameExtents(Window win) {
        QMargins extents;
        if (!display || _NET_FRAME_EXTENTS == 0) return extents;
        
        Atom actual_type;
        int actual_format;
        unsigned long nitems, bytes_after;
        unsigned char* data = nullptr;
        
        if (XGetWindowProperty(display, win, _NET_FRAME_EXTENTS, 0, 4, False,
                              XA_CARDINAL, &actual_type, &actual_format,
                              &nitems, &bytes_after, &data) == Success && data) {
            if (nitems >= 4) {
                long* extents_data = reinterpret_cast<long*>(data);
                extents.setLeft(extents_data[0]);   // 左边框
                extents.setRight(extents_data[1]);  // 右边框
                extents.setTop(extents_data[2]);    // 上边框
                extents.setBottom(extents_data[3]); // 下边框
            }
            XFree(data);
        }
        
        return extents;
    }
};

FramelessWidgetWindow::FramelessWidgetWindow(QWidget* parent)
    : QMainWindow(parent),
      m_titlebar(Q_NULLPTR),
      m_contentHost(nullptr),
      m_borderWidth(5),
      m_bJustMaximized(false),
      m_bResizeable(true),
      m_titlebarHeight(30),
      m_menuBar(nullptr),
      d_ptr(new FramelessWidgetWindowPrivate())
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    
    // 检测运行环境
    detectWindowingSystem();
    
    setResizeable(m_bResizeable);
    initUI();
}

FramelessWidgetWindow::FramelessWidgetWindow(QtAdvancedStylesheet* manager, QWidget* parent)
    : QMainWindow(parent),
      m_titlebar(Q_NULLPTR),
      m_contentHost(nullptr),
      m_borderWidth(5),
      m_bJustMaximized(false),
      m_bResizeable(true),
      m_titlebarHeight(30),
      m_menuBar(nullptr),
      d_ptr(new FramelessWidgetWindowPrivate())
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    
    // 检测运行环境
    detectWindowingSystem();
    
    setResizeable(m_bResizeable);
    initUI();
}

void FramelessWidgetWindow::detectWindowingSystem()
{
#ifdef QT_WAYLANDCLIENT_LIB
    if (QGuiApplication::platformName().contains("wayland")) {
        d_ptr->waylandMode = true;
        qDebug() << "Wayland mode detected";
        
        // Wayland下使用Qt的原生支持
        if (m_bResizeable) {
            // 启用Wayland下的窗口装饰协议支持
            setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
            
            // 尝试启用xdg-decoration协议支持
            if (QGuiApplication::platformName() == "wayland") {
                // 设置Wayland特定的窗口属性
                QWindow* window = this->windowHandle();
                if (window) {
                    // 请求服务器端装饰（如果支持）
                    window->setProperty("xdg-decoration-mode", "server");
                    
                    // 启用窗口管理器协议支持
                    setWindowFlags(windowFlags() | Qt::WindowSystemMenuHint);
                }
            }
        }
    } else
#endif
    if (QGuiApplication::platformName().contains("xcb")) {
        d_ptr->waylandMode = false;
        qDebug() << "X11 mode detected";
        
        // 初始化X11连接
        if (QX11Info::isPlatformX11()) {
            d_ptr->initializeX11Atoms(QX11Info::display());
            d_ptr->x11Window = static_cast<Window>(winId());
            
            // 设置窗口属性以支持更好的窗口管理器集成
            Display* display = QX11Info::display();
            Window rootWindow = DefaultRootWindow(display);
            
            // 请求窗口管理器支持
            if (_NET_REQUEST_FRAME_EXTENTS != 0) {
                XEvent event;
                memset(&event, 0, sizeof(event));
                event.xclient.type = ClientMessage;
                event.xclient.window = d_ptr->x11Window;
                event.xclient.message_type = _NET_REQUEST_FRAME_EXTENTS;
                event.xclient.format = 32;
                
                XSendEvent(display, rootWindow, False,
                          SubstructureRedirectMask | SubstructureNotifyMask, &event);
            }
        }
    } else {
        qDebug() << "Unknown windowing system:" << QGuiApplication::platformName();
    }
}

void FramelessWidgetWindow::setResizeable(bool resizeable)
{
    m_bResizeable = resizeable;
    
    if (d_ptr->waylandMode) {
        // Wayland下通过Qt机制处理
        if (m_bResizeable) {
            setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
        } else {
            setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
        }
    } else {
        // X11下设置窗口属性
        if (d_ptr->display && d_ptr->x11Window) {
            // 设置窗口类型
            Atom window_type = _NET_WM_WINDOW_TYPE_NORMAL;
            XChangeProperty(d_ptr->display, d_ptr->x11Window, _NET_WM_WINDOW_TYPE,
                           XA_ATOM, 32, PropModeReplace,
                           reinterpret_cast<unsigned char*>(&window_type), 1);
            
            // 请求框架扩展信息
            if (_NET_REQUEST_FRAME_EXTENTS != 0) {
                XEvent event;
                memset(&event, 0, sizeof(event));
                event.xclient.type = ClientMessage;
                event.xclient.window = d_ptr->x11Window;
                event.xclient.message_type = _NET_REQUEST_FRAME_EXTENTS;
                event.xclient.format = 32;
                
                XSendEvent(d_ptr->display, DefaultRootWindow(d_ptr->display), False,
                          SubstructureRedirectMask | SubstructureNotifyMask, &event);
            }
        }
    }
}

void FramelessWidgetWindow::setResizeableAreaWidth(int width)
{
    if (1 > width) width = 1;
    m_borderWidth = width;
}

void FramelessWidgetWindow::setTitleBar(QWidget* titlebar)
{
    m_titlebar = titlebar;
    if (!titlebar) return;
    connect(titlebar, SIGNAL(destroyed(QObject*)), this, SLOT(onTitleBarDestroyed()));
}

void FramelessWidgetWindow::onTitleBarDestroyed()
{
    if (m_titlebar == QObject::sender())
    {
        m_titlebar = Q_NULLPTR;
    }
}

void FramelessWidgetWindow::addIgnoreWidget(QWidget* widget)
{
    if (!widget) return;
    if (m_whiteList.contains(widget)) return;
    m_whiteList.append(widget);
}

// 原生X11事件处理
bool FramelessWidgetWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    if (d_ptr->waylandMode) {
        return QMainWindow::nativeEvent(eventType, message, result);
    }
    
    // X11事件处理
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
        uint8_t eventType = event->response_type & ~0x80;
        
        switch (eventType) {
        case XCB_BUTTON_PRESS: {
            xcb_button_press_event_t* bp = reinterpret_cast<xcb_button_press_event_t*>(event);
            
            // 处理调整大小请求
            if (m_bResizeable && bp->detail == XCB_BUTTON_INDEX_1) {
                int direction = getX11ResizeDirection(bp->root_x, bp->root_y);
                if (direction >= 0) {
                    d_ptr->requestWindowResize(d_ptr->x11Window, direction, bp->root_x, bp->root_y);
                    *result = 1;
                    return true;
                }
            }
            break;
        }
        case XCB_BUTTON_RELEASE: {
            xcb_button_release_event_t* br = reinterpret_cast<xcb_button_release_event_t*>(event);
            
            // 重置调整大小状态
            if (m_resizing && br->detail == XCB_BUTTON_INDEX_1) {
                m_resizing = false;
                m_resizeRegion = None;
                unsetCursor();
                *result = 1;
                return true;
            }
            break;
        }
        case XCB_MOTION_NOTIFY: {
            xcb_motion_notify_event_t* motion = reinterpret_cast<xcb_motion_notify_event_t*>(event);
            
            // 更新光标形状
            if (m_bResizeable && !m_resizing) {
                int direction = getX11ResizeDirection(motion->root_x, motion->root_y);
                updateCursorForX11Direction(direction);
            }
            break;
        }
        case XCB_PROPERTY_NOTIFY: {
            xcb_property_notify_event_t* prop = reinterpret_cast<xcb_property_notify_event_t*>(event);
            
            // 监听窗口状态变化
            if (prop->atom == _NET_WM_STATE) {
                updateWindowStateFromX11();
            }
            // 监听框架扩展变化
            else if (prop->atom == _NET_FRAME_EXTENTS) {
                updateFrameExtentsFromX11();
            }
            break;
        }
        case XCB_CONFIGURE_NOTIFY: {
            xcb_configure_notify_event_t* config = reinterpret_cast<xcb_configure_notify_event_t*>(event);
            
            // 窗口配置变化，更新内部状态
            if (config->window == d_ptr->x11Window) {
                updateContentsMargins();
            }
            break;
        }
        }
    }
    
    return QMainWindow::nativeEvent(eventType, message, result);
}

int FramelessWidgetWindow::getX11ResizeDirection(int root_x, int root_y)
{
    if (!m_bResizeable) return -1;
    
    QRect geo = frameGeometry();
    const int border = m_borderWidth;
    
    bool onLeft = (root_x >= geo.left() && root_x <= geo.left() + border);
    bool onRight = (root_x <= geo.right() && root_x >= geo.right() - border);
    bool onTop = (root_y >= geo.top() && root_y <= geo.top() + border);
    bool onBottom = (root_y <= geo.bottom() && root_y >= geo.bottom() - border);
    
    if (onTop && onLeft) return X11_SizeTopLeft;
    if (onTop && onRight) return X11_SizeTopRight;
    if (onBottom && onLeft) return X11_SizeBottomLeft;
    if (onBottom && onRight) return X11_SizeBottomRight;
    if (onLeft) return X11_SizeLeft;
    if (onRight) return X11_SizeRight;
    if (onTop) return X11_SizeTop;
    if (onBottom) return X11_SizeBottom;
    
    return -1;
}

void FramelessWidgetWindow::updateWindowStateFromX11()
{
    if (!d_ptr->display || !d_ptr->x11Window || _NET_WM_STATE == 0) return;
    
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* data = nullptr;
    
    if (XGetWindowProperty(d_ptr->display, d_ptr->x11Window, _NET_WM_STATE, 0, 1024, False,
                          XA_ATOM, &actual_type, &actual_format,
                          &nitems, &bytes_after, &data) == Success && data) {
        Atom* states = reinterpret_cast<Atom*>(data);
        Qt::WindowStates newState = Qt::WindowNoState;
        
        for (unsigned long i = 0; i < nitems; ++i) {
            if (states[i] == _NET_WM_STATE_MAXIMIZED_VERT || 
                states[i] == _NET_WM_STATE_MAXIMIZED_HORZ) {
                newState |= Qt::WindowMaximized;
            } else if (states[i] == _NET_WM_STATE_FULLSCREEN) {
                newState |= Qt::WindowFullScreen;
            }
        }
        
        XFree(data);
        
        // 更新标题栏状态
        if (m_titlebar) {
            CustomTitleBar* customTitleBar = qobject_cast<CustomTitleBar*>(m_titlebar);
            if (customTitleBar) {
                customTitleBar->updateForWindowState();
            }
        }
    }
}

void FramelessWidgetWindow::updateFrameExtentsFromX11()
{
    QMargins extents = d_ptr->getFrameExtents(d_ptr->x11Window);
    if (extents != m_frames) {
        m_frames = extents;
        updateContentsMargins();
    }
}

void FramelessWidgetWindow::updateContentsMargins()
{
    QMainWindow::setContentsMargins(m_margins + m_frames);
}

#elif defined(Q_OS_LINUX)
// Qt后备实现（当原生接口不可用时）
bool FramelessWidgetWindow::handleQtResizeEvents(QEvent* event)
{
    if (d_ptr->waylandMode || !m_bResizeable) {
        return false;
    }
    
    switch (event->type()) {
    case QEvent::MouseMove: {
        auto* me = static_cast<QMouseEvent*>(event);
        if (m_resizing) {
            handleResizeByQt(me->globalPos());
            return true;
        } else {
            updateCursorForQt(me->globalPos());
        }
        break;
    }
    case QEvent::MouseButtonPress: {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton && m_resizeRegion != None) {
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
            m_resizeRegion = None;
            unsetCursor();
            return true;
        }
        break;
    }
    }
    
    return false;
}

void FramelessWidgetWindow::handleResizeByQt(const QPoint& globalPos)
{
    QRect geo = m_startGeo;
    QPoint delta = globalPos - m_dragStartPos;
    
    switch (m_resizeRegion) {
    case Left:       geo.setLeft(geo.left() + delta.x()); break;
    case Right:      geo.setRight(geo.right() + delta.x()); break;
    case Top:        geo.setTop(geo.top() + delta.y()); break;
    case Bottom:     geo.setBottom(geo.bottom() + delta.y()); break;
    case TopLeft:    geo.setTopLeft(geo.topLeft() + delta); break;
    case TopRight:   geo.setTopRight(geo.topRight() + delta); break;
    case BottomLeft: geo.setBottomLeft(geo.bottomLeft() + delta); break;
    case BottomRight:geo.setBottomRight(geo.bottomRight() + delta); break;
    default: break;
    }
    
    // 确保最小尺寸
    if (geo.width() < minimumWidth()) geo.setWidth(minimumWidth());
    if (geo.height() < minimumHeight()) geo.setHeight(minimumHeight());
    
    setGeometry(geo);
}

void FramelessWidgetWindow::updateCursorForQt(const QPoint& globalPos)
{
    QRect geo = frameGeometry();
    const int border = m_borderWidth;
    
    bool onLeft = (globalPos.x() >= geo.left() && globalPos.x() <= geo.left() + border);
    bool onRight = (globalPos.x() <= geo.right() && globalPos.x() >= geo.right() - border);
    bool onTop = (globalPos.y() >= geo.top() && globalPos.y() <= geo.top() + border);
    bool onBottom = (globalPos.y() <= geo.bottom() && globalPos.y() >= geo.bottom() - border);
    
    ResizeRegion region = None;
    if (onTop && onLeft) region = TopLeft;
    else if (onTop && onRight) region = TopRight;
    else if (onBottom && onLeft) region = BottomLeft;
    else if (onBottom && onRight) region = BottomRight;
    else if (onLeft) region = Left;
    else if (onRight) region = Right;
    else if (onTop) region = Top;
    else if (onBottom) region = Bottom;
    
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
}

void FramelessWidgetWindow::updateCursorForX11Direction(int direction)
{
    switch (direction) {
    case X11_SizeTopLeft:
    case X11_SizeBottomRight:
        setCursor(Qt::SizeFDiagCursor); break;
    case X11_SizeTopRight:
    case X11_SizeBottomLeft:
        setCursor(Qt::SizeBDiagCursor); break;
    case X11_SizeLeft:
    case X11_SizeRight:
        setCursor(Qt::SizeHorCursor); break;
    case X11_SizeTop:
    case X11_SizeBottom:
        setCursor(Qt::SizeVerCursor); break;
    default:
        unsetCursor(); break;
    }
}

#endif

void FramelessWidgetWindow::showFullScreen()
{
    if (isMaximized())
    {
        QMainWindow::setContentsMargins(m_margins);
        m_frames = QMargins();
    }
    QMainWindow::showFullScreen();
}

FramelessWidgetWindow::~FramelessWidgetWindow()
{
}

void FramelessWidgetWindow::initUI()
{
    // 创建CustomTitleBar作为标题栏
    CustomTitleBar* customTitleBar = new CustomTitleBar(this);
    customTitleBar->attachToWindow(this);
    customTitleBar->setFixedHeight(m_titlebarHeight);

    // 设置标题栏
    setTitleBar(customTitleBar);

    m_contentHost = new QWidget(this);
    m_contentHost->setObjectName("contentHost");

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(customTitleBar);
    mainLayout->addWidget(m_contentHost, 1);
    
    setCentralWidget(centralWidget);

    //通过快捷键全屏，避免误操作，全屏无动画效果
    QShortcut *shortcut = new QShortcut(QKeySequence(tr("F11")),this);
    connect(shortcut,&QShortcut::activated,[=](){
        showFullScreen();
    });
}

void FramelessWidgetWindow::setTitle(const QString& title)
{
    // 由于m_titlebar是QWidget*类型，我们需要转换为CustomTitleBar*
    CustomTitleBar* customTitleBar = qobject_cast<CustomTitleBar*>(m_titlebar);
    if (customTitleBar) {
        customTitleBar->setTitle(title);
    }
}

void FramelessWidgetWindow::setContentWidget(QWidget* w)
{
    if (m_contentHost && w) {
        QVBoxLayout* contentLayout = new QVBoxLayout(m_contentHost);
        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->addWidget(w);
    }
}

void FramelessWidgetWindow::setResizableEdgesEnabled(bool enabled)
{
    //if (m_bResizeable == enabled) return;
    bool visible = isVisible();
    m_bResizeable = enabled;
#ifdef Q_OS_WIN
    if (m_bResizeable){
        setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

        HWND hwnd = (HWND)this->winId();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
        ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    }else{
        setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

        HWND hwnd = (HWND)this->winId();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
        ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
    }

    const MARGINS shadow = { 1, 1, 1, 1 };
#elif defined(Q_OS_LINUX)
    if (m_bResizeable) {
        qApp->installEventFilter(this);
    } else {
        qApp->removeEventFilter(this);
        m_resizing = false;
        m_resizeRegion = None;
        unsetCursor();
    }
#endif
    setVisible(visible);
}

void FramelessWidgetWindow::setMenuBar(QMenuBar* menuBar)
{
    m_menuBar = menuBar;
}

QMenuBar* FramelessWidgetWindow::menuBar() const
{
    return m_menuBar;
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
        // 由于m_titlebar是QWidget*类型，我们需要转换为CustomTitleBar*
        CustomTitleBar* customTitleBar = qobject_cast<CustomTitleBar*>(m_titlebar);
        if (customTitleBar) {
            customTitleBar->updateForWindowState();
        }
    }
}

} // namespace QtAdvancedUI
