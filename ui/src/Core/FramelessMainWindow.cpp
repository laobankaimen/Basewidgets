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

using namespace acss;

namespace QtAdvancedUI {

FramelessMainWindow::FramelessMainWindow(QWidget* parent)
    : StyledMainWindow(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
    initUI();
}

FramelessMainWindow::FramelessMainWindow(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : StyledMainWindow(manager, parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
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

    // 背景色取主题色
    const bool dark = StyleManagerAdapter::instance()->isCurrentThemeDark();
    m_backgroundColor = QColor(dark ? QString("#2b2d31") : QString("#ffffff"));

    // 默认启用角拖拽缩放、圆角与阴影（可通过 API 关闭）
    setResizableEdgesEnabled(true);
    setResizeMargin(6);
    setCornerRadius(12);
    setDropShadowEnabled(true);
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
    p.setPen(Qt::NoPen);
    p.setBrush(m_backgroundColor);
    const QRect r = rect();
    if (m_cornerRadius > 0) {
        QPainterPath path;
        path.addRoundedRect(r.adjusted(1, 1, -1, -1), m_cornerRadius, m_cornerRadius);
        p.drawPath(path);
    } else {
        p.drawRect(r);
    }
}

// ===== 公共开关 API =====
void FramelessMainWindow::setResizableEdgesEnabled(bool enabled)
{
    if (m_resizeEnabled == enabled) return;
    m_resizeEnabled = enabled;
    if (enabled) {
        qApp->installEventFilter(this);
    } else {
        qApp->removeEventFilter(this);
        m_resizing = false; m_resizeRegion = None; unsetCursor();
    }
}

void FramelessMainWindow::setResizeMargin(int px)
{
    m_resizeMargin = qMax(1, px);
}

void FramelessMainWindow::setCornerRadius(int radiusPx)
{
    const int r = qMax(0, radiusPx);
    if (m_cornerRadius == r) return;
    m_cornerRadius = r;
    setAttribute(Qt::WA_TranslucentBackground, m_cornerRadius > 0 || m_dropShadowEnabled);
    update();
}

void FramelessMainWindow::setDropShadowEnabled(bool enabled)
{
    if (m_dropShadowEnabled == enabled) return;
    m_dropShadowEnabled = enabled;
    if (enabled) {
        auto* eff = new QGraphicsDropShadowEffect(this);
        eff->setBlurRadius(24);
        eff->setOffset(0, 3);
        eff->setColor(QColor(0, 0, 0, 90));
        setGraphicsEffect(eff);
        setAttribute(Qt::WA_TranslucentBackground, true);
    } else {
        setGraphicsEffect(nullptr);
        setAttribute(Qt::WA_TranslucentBackground, m_cornerRadius > 0);
    }
    update();
}

} // namespace QtAdvancedUI