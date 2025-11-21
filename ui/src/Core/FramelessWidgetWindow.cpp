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
#include <objidl.h>

#pragma comment (lib,"Dwmapi.lib")
#pragma comment (lib,"user32.lib")
#elif defined(Q_OS_LINUX)
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPoint>
#endif

namespace QtAdvancedUI {

#ifdef Q_OS_WIN
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
    
    setResizeable(m_bResizeable);
    initUI();
}

void FramelessWidgetWindow::setResizeable(bool resizeable)
{
    bool visible = isVisible();
    m_bResizeable = resizeable;
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
    
    setVisible(visible);
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

bool FramelessWidgetWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
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
void FramelessWidgetWindow::showFullScreen()
{
    if (isMaximized())
    {
        QMainWindow::setContentsMargins(m_margins);
        m_frames = QMargins();
    }
    QMainWindow::showFullScreen();
}

#elif defined(Q_OS_LINUX)
// Linux平台实现
FramelessWidgetWindow::FramelessWidgetWindow(QWidget* parent)
    : QWidget(parent),
      m_titlebar(Q_NULLPTR),
      m_contentHost(nullptr),
      m_resizeRegion(None),
      m_resizing(false),
      m_borderWidth(5),
      m_titlebarHeight(30),
      m_menuBar(nullptr)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    
    setMouseTracking(true);
    initUI();
}

FramelessWidgetWindow::FramelessWidgetWindow(QtAdvancedStylesheet* manager, QWidget* parent)
    : QWidget(parent),
      m_titlebar(Q_NULLPTR),
      m_contentHost(nullptr),
      m_resizeRegion(None),
      m_resizing(false),
      m_borderWidth(5),
      m_titlebarHeight(30),
      m_menuBar(nullptr)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    
    setMouseTracking(true);
    initUI();
}

void FramelessWidgetWindow::setResizeable(bool resizeable)
{
    m_bResizeable = resizeable;
}

bool FramelessWidgetWindow::isResizeable() const
{
    return m_bResizeable;
}

void FramelessWidgetWindow::setResizeableAreaWidth(int width)
{
    m_borderWidth = width;
}

void FramelessWidgetWindow::setTitleBar(QWidget* titlebar)
{
    // Linux平台不需要特殊处理
    Q_UNUSED(titlebar)
}

void FramelessWidgetWindow::addIgnoreWidget(QWidget* widget)
{
    // Linux平台不需要特殊处理
    Q_UNUSED(widget)
}

void FramelessWidgetWindow::onTitleBarDestroyed()
{
    // Linux平台不需要特殊处理
}

#endif

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
#ifdef Q_OS_WIN
    // 为QMainWindow创建中心部件
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(customTitleBar);
    mainLayout->addWidget(m_contentHost, 1);
    
    setCentralWidget(centralWidget);


#elif defined(Q_OS_LINUX)
    // 为QWidget直接设置布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(customTitleBar);
    mainLayout->addWidget(m_contentHost, 1);
#endif
    
    //applyThemeStyles();
    //通过快捷键全屏，避免误操作，全屏无动画效果
    QShortcut *shortcut = new QShortcut(QKeySequence(tr("F11")),this);
    connect(shortcut,&QShortcut::activated,[=](){
        showFullScreen();
    });
    move(10,10);
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
    setResizeable(enabled);
}

bool FramelessWidgetWindow::isMaximized() const
{
#ifdef Q_OS_WIN
    return QMainWindow::isMaximized();
#elif defined(Q_OS_LINUX)
    return QWidget::isMaximized();
#endif
}

void FramelessWidgetWindow::setMenuBar(QMenuBar* menuBar)
{
    m_menuBar = menuBar;
}

QMenuBar* FramelessWidgetWindow::menuBar() const
{
    return m_menuBar;
}

#ifdef Q_OS_LINUX
// Linux平台的事件处理
bool FramelessWidgetWindow::eventFilter(QObject* obj, QEvent* event)
{
    // 只处理鼠标事件，并且只在非最大化状态下处理
    if (!isMaximized() && (event->type() == QEvent::MouseMove || 
                           event->type() == QEvent::MouseButtonPress || 
                           event->type() == QEvent::MouseButtonRelease))
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (handleResizeMouseEvent(mouseEvent))
        {
            return true;
        }
    }
    
    return QWidget::eventFilter(obj, event);
}

bool FramelessWidgetWindow::handleResizeMouseEvent(QMouseEvent* event)
{
    const QPoint globalPos = event->globalPos();
    const QRect frameRect = frameGeometry();
    
    m_resizeRegion = calculateResizeRegion(globalPos, frameRect);
    
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
        if (event->button() == Qt::LeftButton && m_resizeRegion != None)
        {
            m_resizing = true;
            m_resizeStartPos = globalPos;
            m_resizeStartGeometry = frameRect;
            event->accept();
            return true;
        }
        break;
        
    case QEvent::MouseMove:
        if (m_resizing)
        {
            handleResize(globalPos);
            event->accept();
            return true;
        }
        else
        {
            updateCursor(m_resizeRegion);
        }
        break;
        
    case QEvent::MouseButtonRelease:
        if (event->button() == Qt::LeftButton && m_resizing)
        {
            m_resizing = false;
            event->accept();
            return true;
        }
        break;
        
    default:
        break;
    }
    
    return false;
}

FramelessWidgetWindow::ResizeRegion FramelessWidgetWindow::calculateResizeRegion(const QPoint& globalPos, const QRect& frameRect)
{
    const int x = globalPos.x();
    const int y = globalPos.y();
    const int left = frameRect.left();
    const int right = frameRect.right();
    const int top = frameRect.top();
    const int bottom = frameRect.bottom();
    
    const int border = m_borderWidth;
    
    bool leftEdge = (x >= left && x <= left + border);
    bool rightEdge = (x <= right && x >= right - border);
    bool topEdge = (y >= top && y <= top + border);
    bool bottomEdge = (y <= bottom && y >= bottom - border);
    
    if (topEdge && leftEdge) return TopLeft;
    if (topEdge && rightEdge) return TopRight;
    if (bottomEdge && leftEdge) return BottomLeft;
    if (bottomEdge && rightEdge) return BottomRight;
    if (leftEdge) return Left;
    if (rightEdge) return Right;
    if (topEdge) return Top;
    if (bottomEdge) return Bottom;
    
    return None;
}

void FramelessWidgetWindow::handleResize(const QPoint& globalPos)
{
    const QPoint delta = globalPos - m_resizeStartPos;
    QRect newGeometry = m_resizeStartGeometry;
    
    switch (m_resizeRegion)
    {
    case Left:
        newGeometry.setLeft(newGeometry.left() + delta.x());
        break;
    case Right:
        newGeometry.setRight(newGeometry.right() + delta.x());
        break;
    case Top:
        newGeometry.setTop(newGeometry.top() + delta.y());
        break;
    case Bottom:
        newGeometry.setBottom(newGeometry.bottom() + delta.y());
        break;
    case TopLeft:
        newGeometry.setTopLeft(newGeometry.topLeft() + QPoint(delta.x(), delta.y()));
        break;
    case TopRight:
        newGeometry.setTopRight(newGeometry.topRight() + QPoint(delta.x(), delta.y()));
        break;
    case BottomLeft:
        newGeometry.setBottomLeft(newGeometry.bottomLeft() + QPoint(delta.x(), delta.y()));
        break;
    case BottomRight:
        newGeometry.setBottomRight(newGeometry.bottomRight() + QPoint(delta.x(), delta.y()));
        break;
    default:
        break;
    }
    
    setGeometry(newGeometry);
}

void FramelessWidgetWindow::updateCursor(ResizeRegion region)
{
    Qt::CursorShape cursor = Qt::ArrowCursor;
    
    switch (region)
    {
    case Left:
    case Right:
        cursor = Qt::SizeHorCursor;
        break;
    case Top:
    case Bottom:
        cursor = Qt::SizeVerCursor;
        break;
    case TopLeft:
    case BottomRight:
        cursor = Qt::SizeFDiagCursor;
        break;
    case TopRight:
    case BottomLeft:
        cursor = Qt::SizeBDiagCursor;
        break;
    default:
        cursor = Qt::ArrowCursor;
        break;
    }
    
    setCursor(cursor);
}

#endif // Q_OS_LINUX

void FramelessWidgetWindow::showEvent(QShowEvent* e)
{
#ifdef Q_OS_WIN
    QMainWindow::showEvent(e);
#elif defined(Q_OS_LINUX)
    QWidget::showEvent(e);
#endif
}

bool FramelessWidgetWindow::eventFilter(QObject* watched, QEvent* event)
{
#ifdef Q_OS_LINUX
    if (watched == this && (event->type() == QEvent::MouseMove || 
                            event->type() == QEvent::MouseButtonPress || 
                            event->type() == QEvent::MouseButtonRelease))
    {
        return handleResizeMouseEvent(static_cast<QMouseEvent*>(event));
    }
#endif
    
#ifdef Q_OS_WIN
    return QMainWindow::eventFilter(watched, event);
#elif defined(Q_OS_LINUX)
    return QWidget::eventFilter(watched, event);
#endif
}

void FramelessWidgetWindow::paintEvent(QPaintEvent* e)
{
#ifdef Q_OS_WIN
    QMainWindow::paintEvent(e);
#elif defined(Q_OS_LINUX)
    QWidget::paintEvent(e);
#endif
}

void FramelessWidgetWindow::resizeEvent(QResizeEvent* e)
{
#ifdef Q_OS_WIN
    QMainWindow::resizeEvent(e);
#elif defined(Q_OS_LINUX)
    QWidget::resizeEvent(e);
#endif
    
    // 由于m_titlebar是QWidget*类型，我们需要转换为CustomTitleBar*
    CustomTitleBar* customTitleBar = qobject_cast<CustomTitleBar*>(m_titlebar);
    if (customTitleBar) {
        customTitleBar->updateForWindowState();
    }
}

void FramelessWidgetWindow::changeEvent(QEvent* e)
{
#ifdef Q_OS_WIN
    QMainWindow::changeEvent(e);
#elif defined(Q_OS_LINUX)
    QWidget::changeEvent(e);
#endif
    
    if (e->type() == QEvent::WindowStateChange) {
        // 由于m_titlebar是QWidget*类型，我们需要转换为CustomTitleBar*
        CustomTitleBar* customTitleBar = qobject_cast<CustomTitleBar*>(m_titlebar);
        if (customTitleBar) {
            customTitleBar->updateForWindowState();
        }
    }
}

} // namespace QtAdvancedUI