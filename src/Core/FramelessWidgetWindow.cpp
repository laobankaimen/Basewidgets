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
#include <QHoverEvent>

#include <basewidgets/Core/FramelessWidgetWindow.h>
#include <basewidgets/Core/WindowPlacement.h>
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
#include <QGuiApplication>
#include <QPoint>
#include <QTimer>
#include <QWindow>
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

    // Persistence belongs to the application, which can choose a unique settings key.
    resize(1100, 760);
    WindowPlacement::watch(this);
}

void FramelessWidgetWindow::setResizeableAreaWidth(int width)
{
    if (1 > width) width = 1;
    m_borderWidth = width;
}

#ifdef Q_OS_WIN
bool FramelessWidgetWindow::nativeWindowsEvent(const QByteArray &eventType, void *message, NativeEventResult *result)
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

            const LONG border_width = qMax(1,qRound(m_borderWidth*devicePixelRatioF()));
            RECT winrect;
            GetWindowRect(HWND(winId()), &winrect);

            long x = GET_X_LPARAM(msg->lParam);
            long y = GET_Y_LPARAM(msg->lParam);

            if(m_bResizeable && !isMaximized())
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
                // Convert native global -> native client -> Qt local. Dividing a global
                // desktop origin by DPR breaks on negative/mixed-scale secondary screens.
                POINT clientPoint{x,y}; ScreenToClient(msg->hwnd,&clientPoint);
                const qreal dpr=devicePixelRatioF();
                const QPoint titlePos=m_titleBar->mapFrom(this,QPoint(qRound(clientPoint.x/dpr),qRound(clientPoint.y/dpr)));

                QWidget* child = m_titleBar->childAt(titlePos);
                //鼠标标题栏的子控件，让子控件接收事件（过滤掉其他不需要鼠标交互的控件）
                if (child && (child->objectName().contains("TitleBarCloseButton") ||
                    child->objectName().contains("TitleBarMaxButton") ||
                    child->objectName().contains("TitleBarMinButton"))) {
                    *result = HTCLIENT;
                    return true;
                    }
                //不在子控件上，且在标题栏内
                if (m_titleBar->rect().contains(titlePos)) {
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
FramelessWidgetWindow::ResizeRegion FramelessWidgetWindow::resizeRegionAt(const QPoint& pos) const
{
    if (!rect().contains(pos) || !m_bResizeable || isMaximized() || isFullScreen()) return NoneR;
    const bool horizontal = minimumWidth() < maximumWidth();
    const bool vertical = minimumHeight() < maximumHeight();
    const bool left = horizontal && pos.x() < m_borderWidth;
    const bool right = horizontal && pos.x() >= width() - m_borderWidth;
    const bool top = vertical && pos.y() < m_borderWidth;
    const bool bottom = vertical && pos.y() >= height() - m_borderWidth;
    if (top && left) return TopLeft;
    if (top && right) return TopRight;
    if (bottom && left) return BottomLeft;
    if (bottom && right) return BottomRight;
    if (top) return Top;
    if (bottom) return Bottom;
    if (left) return Left;
    if (right) return Right;
    return NoneR;
}

void FramelessWidgetWindow::updateResizeCursor(ResizeRegion region)
{
    m_resizeRegion = region;
    switch (region) {
    case TopLeft: case BottomRight: setCursor(Qt::SizeFDiagCursor); break;
    case TopRight: case BottomLeft: setCursor(Qt::SizeBDiagCursor); break;
    case Left: case Right: setCursor(Qt::SizeHorCursor); break;
    case Top: case Bottom: setCursor(Qt::SizeVerCursor); break;
    default: unsetCursor(); break;
    }
}

bool FramelessWidgetWindow::eventFilter(QObject* watched, QEvent* event)
{
    auto* widget = qobject_cast<QWidget*>(watched);
    if (!widget || widget->window() != this) return QMainWindow::eventFilter(watched, event);
    if (event->type() == QEvent::Polish) widget->setAttribute(Qt::WA_Hover);
    if (!m_bResizeable || isMaximized() || isFullScreen()) return QMainWindow::eventFilter(watched, event);
    if (event->type() == QEvent::HoverMove && !m_resizing) {
        const auto* hover = static_cast<QHoverEvent*>(event);
        updateResizeCursor(resizeRegionAt(widget->mapTo(this, hover->pos())));
    } else if (event->type() == QEvent::Leave && watched == this && !m_resizing) {
        updateResizeCursor(NoneR);
    } else if (event->type() == QEvent::MouseMove) {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (!m_resizing) {
            updateResizeCursor(resizeRegionAt(mapFromGlobal(mouse->globalPos())));
        } else {
            const QPoint delta = mouse->globalPos() - m_dragStartPos;
            QRect geometry = m_startGeo;
            const Qt::Edges edges = resizeRegionToEdges();
            const QSize min = minimumSize().expandedTo(minimumSizeHint());
            if (edges & Qt::LeftEdge)
                geometry.setLeft(m_startGeo.right() - qBound(min.width(), m_startGeo.width()-delta.x(), maximumWidth()) + 1);
            if (edges & Qt::RightEdge)
                geometry.setWidth(qBound(min.width(), m_startGeo.width()+delta.x(), maximumWidth()));
            if (edges & Qt::TopEdge)
                geometry.setTop(m_startGeo.bottom() - qBound(min.height(), m_startGeo.height()-delta.y(), maximumHeight()) + 1);
            if (edges & Qt::BottomEdge)
                geometry.setHeight(qBound(min.height(), m_startGeo.height()+delta.y(), maximumHeight()));
            setGeometry(geometry);
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        auto* mouse = static_cast<QMouseEvent*>(event);
        // Recompute on press: a previous hover position must not resize the wrong edge.
        updateResizeCursor(resizeRegionAt(mapFromGlobal(mouse->globalPos())));
        if (mouse->button() == Qt::LeftButton && m_resizeRegion != NoneR) {
            if (windowHandle() && windowHandle()->startSystemResize(resizeRegionToEdges())) return true;
            m_resizing = true;
            m_dragStartPos = mouse->globalPos();
            m_startGeo = frameGeometry();
            grabMouse();
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (mouse->button() == Qt::LeftButton && m_resizing) {
            m_resizing = false;
            releaseMouse();
            updateResizeCursor(NoneR);
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

bool FramelessWidgetWindow::nativeLinuxEvent(const QByteArray &eventType, void *message, NativeEventResult *result)
{
    return QMainWindow::nativeEvent(eventType, message, result);
}

Qt::Edges FramelessWidgetWindow::resizeRegionToEdges() const
{
    switch (m_resizeRegion) {
    case Left:
        return Qt::LeftEdge;
    case Right:
        return Qt::RightEdge;
    case Top:
        return Qt::TopEdge;
    case Bottom:
        return Qt::BottomEdge;
    case TopLeft:
        return Qt::TopEdge | Qt::LeftEdge;
    case TopRight:
        return Qt::TopEdge | Qt::RightEdge;
    case BottomLeft:
        return Qt::BottomEdge | Qt::LeftEdge;
    case BottomRight:
        return Qt::BottomEdge | Qt::RightEdge;
    case NoneR:
    default:
        return Qt::Edges();
    }
}


#endif

void FramelessWidgetWindow::setFramelessWindowState(FramelessWindowState state)
{
    // Qt owns platform window-state transitions and sends the matching change events.
    switch (state) {
    case Maximized: showMaximized(); break;
    case Minimized: showMinimized(); break;
    case FullScreen: showFullScreen(); break;
    case Normal: showNormal(); break;
    }
}

void FramelessWidgetWindow::showEvent(QShowEvent *ev)
{
    QMainWindow::showEvent(ev);
}

void FramelessWidgetWindow::closeEvent(QCloseEvent *ev)
{
    QMainWindow::closeEvent(ev);

}

void FramelessWidgetWindow::showFullScreen()
{
    if (!isFullScreen()) m_preFullScreenState = windowState();
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
    auto* header = new QWidget(this);
    auto* headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(0);
    auto* title = new CustomTitleBar(header);
    title->attachToWindow(this);
    title->setFixedHeight(m_titlebarHeight);
    m_titleBar = title;
    headerLayout->addWidget(title);
    m_menuBarWidget = new QWidget(header);
    auto* menuLayout = new QVBoxLayout(m_menuBarWidget);
    menuLayout->setContentsMargins(0, 0, 0, 0);
    menuLayout->setSpacing(0);
    headerLayout->addWidget(m_menuBarWidget);
    m_menuBarWidget->hide();
    QMainWindow::setMenuWidget(header);

    m_contentHost = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(m_contentHost);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    QMainWindow::setCentralWidget(m_contentHost);
    setAttribute(Qt::WA_Hover);

    auto* shortcut = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(shortcut, &QShortcut::activated, this, [this] {
        if (isFullScreen()) {
            if (m_preFullScreenState & Qt::WindowMaximized) showMaximized();
            else showNormal();
        } else showFullScreen();
    });
}

void FramelessWidgetWindow::setTitle(const QString& title)
{
    setWindowTitle(title);
}

void FramelessWidgetWindow::setContentWidget(QWidget* w)
{
    if (w == m_centralWidget) {
        return;
    }
    if(m_centralWidget && m_contentHost) {
        m_contentHost->layout()->removeWidget(m_centralWidget);
        auto* old = m_centralWidget.data();
        m_centralWidget = nullptr;
        old->hide();
        old->deleteLater();
    }
    if (m_contentHost && w) {
        m_centralWidget = w;
        m_contentHost->layout()->addWidget(m_centralWidget);
        m_centralWidget->show();
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
        if (m_resizing) releaseMouse();
        m_resizing = false;
        updateResizeCursor(NoneR);
    }
#endif
    setVisible(visible);
}

void FramelessWidgetWindow::setMenuBar(QMenuBar* menuBar)
{
    if (menuBar == m_menuBar) {
        return;
    }
    if(m_menuBar && m_menuBarWidget) {
        m_menuBarWidget->layout()->removeWidget(m_menuBar);
        auto* old = m_menuBar.data();
        m_menuBar = nullptr;
        old->hide();
        old->deleteLater();
    }
    if (m_menuBarWidget && menuBar) {
        m_menuBar = menuBar;
        m_menuBar->setNativeMenuBar(false);
        m_menuBarWidget->layout()->addWidget(m_menuBar);
        m_menuBar->show();
    }
    m_menuBarWidget->setVisible(m_menuBar != nullptr);
}

void FramelessWidgetWindow::setToolBar(QToolBar* toolbar)
{
    if (toolbar == m_toolBar) return;
    if (m_toolBar) {
        QMainWindow::removeToolBar(m_toolBar);
        m_toolBar->hide();
        m_toolBar->deleteLater();
    }
    m_toolBar = toolbar;
    if (toolbar) {
        QMainWindow::addToolBar(Qt::TopToolBarArea, toolbar);
        toolbar->show();
    }
}

void FramelessWidgetWindow::setStatusBar(QStatusBar* statusbar)
{
    if (statusbar == m_statusBar) return;
    m_statusBar = statusbar;
    QMainWindow::setStatusBar(statusbar);
}

bool FramelessWidgetWindow::nativeEvent(const QByteArray &eventType, void *message, NativeEventResult *result)
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
        if (m_titleBar) m_titleBar->setVisible(!isFullScreen());
#ifdef Q_OS_LINUX
        if (m_resizing) releaseMouse();
        m_resizing = false;
        updateResizeCursor(NoneR);
#endif
        // 由于m_titleBar是QWidget*类型，我们需要转换为CustomTitleBar*
        auto* customTitleBar = qobject_cast<CustomTitleBar*>(m_titleBar);
        if (customTitleBar) {
            customTitleBar->updateForWindowState();
        }
    }
    QMainWindow::changeEvent(e);
}

} // namespace basewidgets
