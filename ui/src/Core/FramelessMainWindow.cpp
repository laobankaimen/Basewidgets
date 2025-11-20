#include <QtAdvancedUI/Core/FramelessMainWindow.h>
#include <QtAdvancedUI/Core/CustomTitleBar.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

#include <QVBoxLayout>
#include <QMenuBar>
#include <QShowEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QApplication>
#include <QRegion>
#include <QResizeEvent>

#ifdef Q_OS_WIN
    #include <windows.h>
    #include <winuser.h>
    #include <windowsx.h>
    #include <dwmapi.h>
    #include <objidl.h> // Fixes error C2504: 'IUnknown' : base class undefined
    #include <gdiplus.h>
    #include <gdipluscolor.h>
#endif
#ifdef Q_OS_LINUX
#endif
using namespace acss;

namespace QtAdvancedUI {

FramelessMainWindow::FramelessMainWindow(QWidget* parent)
    : StyledMainWindow(parent)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
    initUI();

    setAttribute(Qt::WA_PaintOnScreen,true);
}

FramelessMainWindow::FramelessMainWindow(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : StyledMainWindow(manager, parent)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    initUI();
}

FramelessMainWindow::~FramelessMainWindow() = default;

void FramelessMainWindow::initUI()
{
    // 复合：标题栏 + 菜单栏
    m_menuComposite = new QWidget(this);
    m_menuComposite->setObjectName("FramelessMainWindowMenuComposite");
    m_menuComposite->setAttribute(Qt::WA_StyledBackground, true);
    m_menuCompositeLayout = new QVBoxLayout(m_menuComposite);
    m_menuCompositeLayout->setContentsMargins(0, 0, 0, 0);
    m_menuCompositeLayout->setSpacing(0);

    // 标题栏
    m_titleBar = new CustomTitleBar(m_menuComposite);
    m_titleBar->attachToWindow(this);
    m_menuCompositeLayout->addWidget(m_titleBar);

    // 菜单栏占位：由 setMenuBar 填充
    m_menuCompositeLayout->addSpacing(0);

    // 将复合区设为菜单区域
    setMenuWidget(m_menuComposite);
    // 默认启用角拖拽缩放与圆角；阴影默认关闭以避免 Win10 layered 错误
    setResizableEdgesEnabled(true);
    setResizeMargin(6);
    // 默认关闭圆角，用户可通过 API 开启
    setCornerRadius(0);
    setDropShadowEnabled(false);
}

void FramelessMainWindow::setMenuBar(QMenuBar* menuBar)
{
    // 将菜单栏接入标题栏下方
    m_menuBarRef = menuBar;
    if (!m_menuBarRef) {
        return;
    }
    // 删除原有可能存在的菜单栏（防止重复添加）
    const int count = m_menuCompositeLayout->count();
    if (count >= 2) {
        // 第二项为菜单栏或占位
        auto* item = m_menuCompositeLayout->itemAt(1);
        if (item && item->widget()) {
            item->widget()->setParent(nullptr);
            m_menuCompositeLayout->removeItem(item);
        }
    }
    m_menuBarRef->setParent(m_menuComposite);
    m_menuCompositeLayout->insertWidget(1, m_menuBarRef);
}

void FramelessMainWindow::showEvent(QShowEvent* e)
{
    StyledMainWindow::showEvent(e);
    // 让标题栏跟随窗口标题变化
    if (m_titleBar) {
        QObject::connect(this, &QWidget::windowTitleChanged, m_titleBar, &CustomTitleBar::setTitle);
        m_titleBar->setTitle(windowTitle());
    }
}

bool FramelessMainWindow::eventFilter(QObject* watched, QEvent* event)
{
    QWidget* w = qobject_cast<QWidget*>(watched);
    if (!w || w->window() != this) return StyledMainWindow::eventFilter(watched, event);
    if (!m_resizeEnabled) return StyledMainWindow::eventFilter(watched, event);

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

        ResizeRegion region = None;
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
        return StyledMainWindow::eventFilter(watched, event);
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
    default:
        break;
    }

    return StyledMainWindow::eventFilter(watched, event);
}

void FramelessMainWindow::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::HighQualityAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.setPen(Qt::NoPen);
    //p.setBrush(m_backgroundColor);
    const QRect r = rect();
    if (m_cornerRadius > 0) {
        QPainterPath path;
        path.addRoundedRect(r, m_cornerRadius, m_cornerRadius);
        p.drawPath(path);
    } else {
        p.drawRect(r);
    }
}

// ===== 公共开关 API =====
void FramelessMainWindow::setResizableEdgesEnabled(bool enabled)
{
    if (m_resizeEnabled == enabled) return;
#ifdef Q_OS_WIN
    bool visible = isVisible();
    m_resizeEnabled = enabled;
    if (m_resizeEnabled){
        setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

        //此行代码可以带回Aero效果，同时也带回了标题栏和边框,在nativeEvent()会再次去掉标题栏
        HWND hwnd = (HWND)this->winId();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
        ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    }else{
        setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
        //        setWindowFlag(Qt::WindowMaximizeButtonHint,false);

        HWND hwnd = (HWND)this->winId();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
        ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
    }

    //保留一个像素的边框宽度，否则系统不会绘制边框阴影(部分系统会提示API未定义)
    //const MARGINS shadow = { 1, 1, 1, 1 };
    //DwmExtendFrameIntoClientArea(HWND(winId()), &shadow);
    setVisible(visible);
#endif

#ifdef Q_OS_LINUX
    m_resizeEnabled = enabled;
    if (enabled) {
        qApp->installEventFilter(this);
    } else {
        qApp->removeEventFilter(this);
        m_resizing = false; m_resizeRegion = None; unsetCursor();
    }
#endif
}

void FramelessMainWindow::setResizeMargin(int px)
{
    m_resizeMargin = qMax(1, px);
}

void FramelessMainWindow::setCornerRadius(int radiusPx)
{
#ifdef Q_OS_WIN

#endif
#ifdef  Q_OS_LINUX
    const int r = qMax(0, radiusPx);
    if (m_cornerRadius == r) return;
    m_cornerRadius = r;
    if (r > 0) m_lastCornerRadiusNonZero = r;
    // 仅在启用阴影时使用半透明背景
    setAttribute(Qt::WA_TranslucentBackground, m_dropShadowEnabled);
    // 让标题栏顶部圆角与窗口保持一致
    if (m_titleBar) {
        const bool noTopRadius = isFullScreen() || isMaximized();
        m_titleBar->setCornerRadius(noTopRadius ? 0 : r);
    }
    // 使用窗口遮罩裁剪物理窗口形状，避免四角可见
    if (m_cornerRadius > 0) {
        const QRect rct = rect();
        const int radius = qMin(m_cornerRadius, qMin(rct.width(), rct.height())/2);
        QPainterPath path;
        path.addRoundedRect(rct, radius, radius);
        setMask(QRegion(path.toFillPolygon().toPolygon()));
    } else {
        clearMask();
    }
#endif
    update();
}

void FramelessMainWindow::resizeEvent(QResizeEvent* e)
{
    StyledMainWindow::resizeEvent(e);
    /*if (m_cornerRadius > 0) {
        const QRect rct = rect();
        const int radius = qMin(m_cornerRadius, qMin(rct.width(), rct.height())/2);
        QPainterPath path;
        path.addRoundedRect(rct, radius, radius);
        setMask(QRegion(path.toFillPolygon().toPolygon()));
    }
    // 全屏/最大化时关闭标题栏圆角
    if (m_titleBar) {
        const bool noTopRadius = isFullScreen() || isMaximized();
        m_titleBar->setCornerRadius(noTopRadius ? 0 : m_cornerRadius);
    }*/
}

void FramelessMainWindow::setDropShadowEnabled(bool enabled)
{
    if (m_dropShadowEnabled == enabled) return;
    m_dropShadowEnabled = enabled;
    if (enabled) {
        auto* eff = new QGraphicsDropShadowEffect(this);
        eff->setBlurRadius(80);
        eff->setOffset(0, 8);
        eff->setColor(QColor(0, 0, 0, 140));
        setGraphicsEffect(eff);
        setAttribute(Qt::WA_TranslucentBackground, true);
    } else {
        setGraphicsEffect(nullptr);
        setAttribute(Qt::WA_TranslucentBackground, false);
    }
    update();
}

void FramelessMainWindow::changeEvent(QEvent* e)
{
    StyledMainWindow::changeEvent(e);
    if (e->type() == QEvent::WindowStateChange) {
        if (m_titleBar) {
            const bool noTopRadius = isFullScreen() || isMaximized();
            m_titleBar->setCornerRadius(noTopRadius ? 0 : m_cornerRadius);
        }
    }
}

bool FramelessMainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result) {
#ifdef Q_OS_WIN
    //Workaround for known bug -> check Qt forum : https://forum.qt.io/topic/93141/qtablewidget-itemselectionchanged/13
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

        //this kills the window frame and title bar we added with WS_THICKFRAME and WS_CAPTION
        *result = WVR_REDRAW;
        return true;
    }
    case WM_NCHITTEST:
    {
        *result = 0;

        const LONG border_width = m_resizeMargin;
        RECT winrect;
        GetWindowRect(HWND(winId()), &winrect);

        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);

        if(m_resizeEnabled)
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

        //*result still equals 0, that means the cursor locate OUTSIDE the frame area
        //but it may locate in titlebar area
        if (!m_titleBar) return false;

        //support highdpi
        double dpr = this->devicePixelRatioF();
        QPoint pos = m_titleBar->mapFromGlobal(QPoint(x/dpr,y/dpr));

        if (!m_titleBar->rect().contains(pos)) return false;
        QWidget* child = m_titleBar->childAt(pos);
        if (!child)
        {
            *result = HTCAPTION;
            return true;
        }else{
            /*if (m_whiteList.contains(child))
            {
                *result = HTCAPTION;
                return true;
            }*/
        }
        return false;
    } //end case WM_NCHITTEST
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

            QMainWindow::setContentsMargins(m_frames.left()+m_margins.left(), \
                                            m_frames.top()+m_margins.top(), \
                                            m_frames.right()+m_margins.right(), \
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
#endif

#ifdef Q_OS_LINUX
    return StyledMainWindow::nativeEvent(eventType, message, result);
#endif
}

void FramelessMainWindow::setRoundedCornersEnabled(bool enabled)
{
    if (enabled) {
        setCornerRadius(m_lastCornerRadiusNonZero > 0 ? m_lastCornerRadiusNonZero : 12);
    } else {
        setCornerRadius(0);
    }
}

void FramelessMainWindow::setContentsMargins(const QMargins &margins)
{
    QMainWindow::setContentsMargins(margins+m_frames);
    m_margins = margins;
}

void FramelessMainWindow::showFullScreen()
{
    if (isMaximized())
    {
        QMainWindow::setContentsMargins(m_margins);
        m_frames = QMargins();
    }
    QMainWindow::showFullScreen();
}


} // namespace QtAdvancedUI