#include <QtAdvancedUI/Core/FramelessDialog.h>
#include <QtAdvancedUI/Inputs/ToolButton.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>
#include <QPainter>

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
    // Title bar
    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("FramelessDialogTitleBar");
    m_titleBar->setAttribute(Qt::WA_StyledBackground, true);

    m_titleLabel = new QLabel(m_titleBar);
    m_titleLabel->setObjectName("FramelessDialogTitle");
    m_titleLabel->setText(windowTitle());
    m_titleLabel->setStyleSheet("font-size: 16px; font-weight: 600;");

    auto* closeBtn = new ToolButton(m_titleBar);
    closeBtn->setObjectName("FramelessDialogCloseButton");
    closeBtn->setToolTip(tr("关闭"));
    closeBtn->setAutoRaise(true);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    // 使用自绘“X”图标，避免样式返回最小化图标
    {
        const QSize iconSz(18, 18);
        const qreal dpr = qApp->devicePixelRatio();
        QPixmap pm(iconSz * dpr);
        pm.setDevicePixelRatio(dpr);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing, true);
        const bool darkTheme = StyleManagerAdapter::instance()->isCurrentThemeDark();
        const QColor xColor = darkTheme ? QColor("#ffffff") : QColor("#000000");
        QPen pen(xColor, 1.8 * dpr, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        const int m = int(4 * dpr);
        const int w = iconSz.width() * dpr;
        const int h = iconSz.height() * dpr;
        p.drawLine(QPoint(m, m), QPoint(w - m, h - m));
        p.drawLine(QPoint(m, h - m), QPoint(w - m, m));
        p.end();
        closeBtn->setIcon(QIcon(pm));
        closeBtn->setIconSize(iconSz);
    }
    QObject::connect(closeBtn, &ToolButton::clicked, this, [this]() { reject(); });
    m_closeBtn = closeBtn;

    auto* titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(10, 6, 10, 6);
    titleLayout->setSpacing(8);
    titleLayout->addWidget(m_titleLabel, 1);
    titleLayout->addWidget(closeBtn, 0);
    m_titleBar->setLayout(titleLayout);

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

    // Visual differentiation for title bar vs content
    const bool dark = StyleManagerAdapter::instance()->isCurrentThemeDark();
    const QString titleBg = dark ? QString("#1f1f1f") : QString("#f2f2f2");
    const QString closeHover = QString("#E81123"); // Windows-like hover red
    const QString closePressed = QString("#C50F1F"); // Windows-like pressed red
    m_titleBar->setStyleSheet(QString(
        "QWidget#FramelessDialogTitleBar { background-color: %1; }\n"
        "QToolButton#FramelessDialogCloseButton { border-radius: 4px; padding: 2px; icon-size: 18px; }\n"
        "QToolButton#FramelessDialogCloseButton:hover { background-color: %2; }\n"
        "QToolButton#FramelessDialogCloseButton:pressed { background-color: %3; }\n"
    ).arg(titleBg, closeHover, closePressed));
    m_contentHost->setStyleSheet(QString(
        "QWidget#FramelessDialogContentHost { background-color: %1; }\n"
    ).arg(dark ? QString("#2b2d31") : QString("#ffffff")));

    // Drag support via title bar
    m_titleBar->installEventFilter(this);
}

void FramelessDialog::setTitle(const QString& title)
{
    setWindowTitle(title);
    if (m_titleLabel) m_titleLabel->setText(title);
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
    if (watched == m_titleBar) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            auto* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragStartPos = me->globalPos() - frameGeometry().topLeft();
                return true;
            }
            break;
        }
        case QEvent::MouseMove: {
            if (m_dragging) {
                auto* me = static_cast<QMouseEvent*>(event);
                move(me->globalPos() - m_dragStartPos);
                return true;
            }
            break;
        }
        case QEvent::MouseButtonRelease: {
            auto* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                m_dragging = false;
                return true;
            }
            break;
        }
        default:
            break;
        }
    }
    return StyledDialog::eventFilter(watched, event);
}

} // namespace QtAdvancedUI