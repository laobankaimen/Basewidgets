#include <QtAdvancedUI/Core/FramelessDialog.h>
#include <QtAdvancedUI/Core/CustomTitleBar.h>
#include <QtAdvancedUI/Inputs/ToolButton.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>
#include <QPainter>
#include <QPaintEvent>
#include <QGraphicsDropShadowEffect>
#include <QRegion>
#include <QResizeEvent>

using namespace acss;

namespace QtAdvancedUI {

FramelessDialog::FramelessDialog(QWidget* parent)
    : StyledDialog(parent)
{
    // Frameless and styled
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
    initUI();
}

FramelessDialog::FramelessDialog(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : StyledDialog(manager, parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    initUI();
}

FramelessDialog::~FramelessDialog() = default;

void FramelessDialog::initUI()
{
    // 自定义标题栏（包含最小化/最大化/关闭、拖动/停靠）
    auto* customBar = new CustomTitleBar(this);
    customBar->attachToWindow(this);
    customBar->setTitle(windowTitle());
    // 对话框：仅显示关闭按钮，禁用双击放大
    customBar->setButtonsVisible(false, false, true);
    customBar->setDoubleClickToToggleEnabled(false);
    m_titleBar = customBar;
    m_titleLabel = nullptr; // 使用 CustomTitleBar 内部的标题标签
    m_closeBtn = nullptr;

    // Content host
    m_contentHost = new QWidget(this);
    m_contentHost->setObjectName("FramelessDialogContentHost");
    m_contentHost->setAttribute(Qt::WA_StyledBackground, true);
    auto* hostLayout = new QVBoxLayout(m_contentHost);
    hostLayout->setContentsMargins(12, 12, 12, 12);
    hostLayout->setSpacing(8);
    m_contentHost->setLayout(hostLayout);

    // Main layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_contentHost, 1);
    setLayout(mainLayout);

    // Default sizing to avoid too small dialogs
    setMinimumSize(380, 240);
    // Do not force a large initial size; size will adjust to content in setContentWidget

    // 内容样式：使用更浅背景以增强阴影对比
    const bool dark = StyleManagerAdapter::instance()->isCurrentThemeDark();
    const QString bg = dark ? QString("#3a3c40") : QString("#f7f7f7");
    m_backgroundColor = QColor(bg);
    m_contentHost->setStyleSheet(QString(
        "QWidget#FramelessDialogContentHost { background-color: %1; }\n"
    ).arg(bg));

    // 默认禁用角拖拽缩放（Dialogs 默认不可拉伸，可通过 API 开启）
    setResizableEdgesEnabled(false);
    setResizeMargin(6);
    // 将圆角调整为更适中的默认值（与 MainWindow 统一）
    setCornerRadius(12);
    // 默认关闭阴影以避免 Win10 layered window 错误
    setDropShadowEnabled(false);
}

void FramelessDialog::setTitle(const QString& title)
{
    setWindowTitle(title);
    // CustomTitleBar 会通过 windowTitleChanged 信号自动更新
}

QString FramelessDialog::title() const
{
    return m_titleLabel ? m_titleLabel->text() : QString();
}

void FramelessDialog::setContentWidget(QWidget* w)
{
    if (!m_contentHost || !w) return;
    auto* hostLayout = qobject_cast<QVBoxLayout*>(m_contentHost->layout());
    if (!hostLayout) return;
    // clear previous
    while (hostLayout->count() > 0) {
        auto* item = hostLayout->takeAt(0);
        if (item) {
            if (auto* wid = item->widget()) wid->setParent(nullptr);
            delete item;
        }
    }
    hostLayout->addWidget(w);
    // Resize the dialog to fit content while respecting minimum size
    adjustSize();
    const QSize sz = size();
    const int minW = minimumSize().width();
    const int minH = minimumSize().height();
    resize(qMax(sz.width(), minW), qMax(sz.height(), minH));
}

bool FramelessDialog::eventFilter(QObject* watched, QEvent* event)
{
    // 仅处理属于本对话框窗口的事件
    QWidget* w = qobject_cast<QWidget*>(watched);
    if (!w || w->window() != this) return StyledDialog::eventFilter(watched, event);

    // 鼠标事件处理：角拖拽缩放
    if (!m_resizeEnabled) return StyledDialog::eventFilter(watched, event);

    switch (event->type()) {
    case QEvent::MouseMove: {
        auto* me = static_cast<QMouseEvent*>(event);
        const QPoint global = me->globalPos();
        const QPoint local = mapFromGlobal(global);
        const QRect r = rect();

        // 正在缩放时，更新窗口几何
        if (m_resizing) {
            QRect geo = m_startGeo;
            const QPoint delta = global - m_dragStartPos;
            const int minW = minimumSize().width();
            const int minH = minimumSize().height();

            switch (m_resizeRegion) {
            case Left:       geo.setLeft(geo.left() + delta.x()); break;
            case Right:      geo.setRight(geo.right() + delta.x()); break;
            case Top:        geo.setTop(geo.top() + delta.y()); break;
            case Bottom:     geo.setBottom(geo.bottom() + delta.y()); break;
            case TopLeft:    geo.setTop(geo.top() + delta.y()); geo.setLeft(geo.left() + delta.x()); break;
            case TopRight:   geo.setTop(geo.top() + delta.y()); geo.setRight(geo.right() + delta.x()); break;
            case BottomLeft: geo.setBottom(geo.bottom() + delta.y()); geo.setLeft(geo.left() + delta.x()); break;
            case BottomRight:geo.setBottom(geo.bottom() + delta.y()); geo.setRight(geo.right() + delta.x()); break;
            default: break;
            }

            // 约束最小尺寸
            if (geo.width() < minW) geo.setWidth(minW);
            if (geo.height() < minH) geo.setHeight(minH);
            setGeometry(geo);
            return true;
        }

        // 未按下时，根据位置更新光标与区域
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

        // 更新光标
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
        return StyledDialog::eventFilter(watched, event);
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

    return StyledDialog::eventFilter(watched, event);
}

void FramelessDialog::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    // 绘制圆角背景（启用圆角时）
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::HighQualityAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.setPen(Qt::NoPen);
    p.setBrush(m_backgroundColor);

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
void FramelessDialog::setResizableEdgesEnabled(bool enabled)
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

void FramelessDialog::setResizeMargin(int px)
{
    m_resizeMargin = qMax(1, px);
}

void FramelessDialog::setCornerRadius(int radiusPx)
{
    const int r = qMax(0, radiusPx);
    if (m_cornerRadius == r) return;
    m_cornerRadius = r;
    if (r > 0) m_lastCornerRadiusNonZero = r;
    // 启用圆角需要半透明背景以便裁剪
    setAttribute(Qt::WA_TranslucentBackground, m_cornerRadius > 0);
    // 同步更新标题栏顶部圆角
    if (m_titleBar) {
        auto* customBar = qobject_cast<CustomTitleBar*>(m_titleBar);
        if (customBar) customBar->setCornerRadius(r);
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
    update();
}

void FramelessDialog::resizeEvent(QResizeEvent* e)
{
    StyledDialog::resizeEvent(e);
    if (m_cornerRadius > 0) {
        const QRect rct = rect();
        const int radius = qMin(m_cornerRadius, qMin(rct.width(), rct.height())/2);
        QPainterPath path;
        path.addRoundedRect(rct, radius, radius);
        setMask(QRegion(path.toFillPolygon().toPolygon()));
    }
}

void FramelessDialog::setRoundedCornersEnabled(bool enabled)
{
    if (enabled) {
        // 恢复最近一次非零圆角半径（默认 12）
        setCornerRadius(m_lastCornerRadiusNonZero > 0 ? m_lastCornerRadiusNonZero : 12);
    } else {
        setCornerRadius(0);
    }
}

void FramelessDialog::setDropShadowEnabled(bool enabled)
{
    if (m_dropShadowEnabled == enabled) return;
    m_dropShadowEnabled = enabled;
    if (enabled) {
        auto* eff = new QGraphicsDropShadowEffect(this);
        eff->setBlurRadius(80);
        eff->setOffset(0, 8);
        eff->setColor(QColor(0, 0, 0, 140));
        setGraphicsEffect(eff);
        // 为阴影配合启用透明背景
        setAttribute(Qt::WA_TranslucentBackground, true);
    } else {
        setGraphicsEffect(nullptr);
        // 禁用阴影时关闭半透明背景（圆角遮罩即可，不再依赖 layered window）
        setAttribute(Qt::WA_TranslucentBackground, false);
    }
    update();
}

void FramelessDialog::changeEvent(QEvent* e)
{
    StyledDialog::changeEvent(e);
    if (e->type() == QEvent::WindowStateChange) {
        auto* customBar = qobject_cast<CustomTitleBar*>(m_titleBar);
        if (customBar) {
            const bool noTopRadius = isFullScreen() || isMaximized();
            customBar->setCornerRadius(noTopRadius ? 0 : m_cornerRadius);
        }
    }
}

} // namespace QtAdvancedUI