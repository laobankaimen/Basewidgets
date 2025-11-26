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

#ifdef Q_OS_WIN
#include <windows.h>
#include <winuser.h>
#include <windowsx.h>
#include <dwmapi.h>

#pragma comment (lib,"Dwmapi.lib")
#pragma comment (lib,"user32.lib")
#endif

using namespace acss;

namespace QtAdvancedUI {

FramelessDialog::FramelessDialog(QWidget* parent)
    : QDialog(parent)
{
    // Frameless and styled
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

    // 禁用角拖拽缩放
    // 默认关闭圆角，用户可通过 API 开启
    setCornerRadius(0);

    //主题切换
    connect(StyleManagerAdapter::instance(), &acss::QtAdvancedStylesheet::stylesheetChanged, this, [this] {
        update();
    });
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

void FramelessDialog::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    // 绘制圆角背景（启用圆角时）
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::HighQualityAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.setPen(Qt::NoPen);
    p.setBrush(StyleManagerAdapter::instance()->generateThemePalette().window());

    const QRect r = rect();
    if (m_cornerRadius > 0) {
        QPainterPath path;
        path.addRoundedRect(r, m_cornerRadius, m_cornerRadius);
        p.drawPath(path);
    } else {
        p.drawRect(r);
    }
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
    QDialog::resizeEvent(e);
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

void FramelessDialog::changeEvent(QEvent* e)
{
    QDialog::changeEvent(e);
    if (e->type() == QEvent::WindowStateChange) {
        auto* customBar = qobject_cast<CustomTitleBar*>(m_titleBar);
        if (customBar) {
            const bool noTopRadius = isFullScreen() || isMaximized();
            customBar->setCornerRadius(noTopRadius ? 0 : m_cornerRadius);
        }
    }
}
#ifdef Q_OS_WIN
bool FramelessDialog::nativeEvent(const QByteArray &eventType, void *message, long *result) {
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
        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);

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
        return true;
    }
    case WM_NCLBUTTONDBLCLK: {
        //拦截双击放大
        *result = 0;
        return true;
    }
    default:
        return QDialog::nativeEvent(eventType, message, result);
    }
}
#endif
} // namespace QtAdvancedUI
