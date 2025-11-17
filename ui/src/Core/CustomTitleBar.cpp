#include <QtAdvancedUI/Core/CustomTitleBar.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QtAdvancedUI/Inputs/ToolButton.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QIcon>
#include <QStyle>

using namespace acss;

namespace QtAdvancedUI {

CustomTitleBar::CustomTitleBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("CustomTitleBar");
    setAttribute(Qt::WA_StyledBackground, true);
    initUI();
    applyThemeStyles();
}

CustomTitleBar::~CustomTitleBar() = default;

void CustomTitleBar::attachToWindow(QWidget* window)
{
    m_window = window;
    if (!m_window) return;
    // 跟随窗口标题变化
    QObject::connect(m_window, &QWidget::windowTitleChanged, this, &CustomTitleBar::setTitle);
    // 监听窗口状态改变（Qt5 无 windowStateChanged 信号，使用事件过滤器）
    m_window->installEventFilter(this);
    setTitle(m_window->windowTitle());
    updateForWindowState();
}

void CustomTitleBar::setTitle(const QString& title)
{
    if (m_titleLabel) m_titleLabel->setText(title);
}

void CustomTitleBar::updateForWindowState()
{
    if (!m_window || !m_btnMax) return;
    const bool maximized = m_window->isMaximized();
    const QIcon icon = maximized ? style()->standardIcon(QStyle::SP_TitleBarNormalButton)
                                 : style()->standardIcon(QStyle::SP_TitleBarMaxButton);
    m_btnMax->setIcon(icon);
}

void CustomTitleBar::mousePressEvent(QMouseEvent* e)
{
    if (!m_window) return;
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStartPos = e->globalPos() - m_window->frameGeometry().topLeft();
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_window) return;
    if (m_dragging) {
        m_window->move(e->globalPos() - m_dragStartPos);
        e->accept();
        return;
    }
    QWidget::mouseMoveEvent(e);
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_window) return;
    if (m_dragging && e->button() == Qt::LeftButton) {
        m_dragging = false;
        snapDockIfNeeded(e->globalPos());
        e->accept();
        return;
    }
    QWidget::mouseReleaseEvent(e);
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (!m_window) return;
    if (m_doubleClickToggleEnabled && e->button() == Qt::LeftButton) {
        toggleMaximizeRestore();
        e->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(e);
}

void CustomTitleBar::initUI()
{
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("CustomTitleBarTitle");
    m_titleLabel->setStyleSheet("font-size: 15px; font-weight: 600;");

    m_btnMin = new ToolButton(this);
    m_btnMin->setObjectName("CustomTitleBarMinButton");
    m_btnMin->setToolTip(tr("最小化"));
    m_btnMin->setCursor(Qt::PointingHandCursor);
    m_btnMin->setAutoRaise(false);
    m_btnMin->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnMin->setFixedSize(QSize(28, 28));
    m_btnMin->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    m_btnMin->setIconSize(QSize(18, 18));

    m_btnMax = new ToolButton(this);
    m_btnMax->setObjectName("CustomTitleBarMaxButton");
    m_btnMax->setToolTip(tr("最大化/还原"));
    m_btnMax->setCursor(Qt::PointingHandCursor);
    m_btnMax->setAutoRaise(false);
    m_btnMax->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnMax->setFixedSize(QSize(28, 28));
    m_btnMax->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    m_btnMax->setIconSize(QSize(18, 18));

    m_btnClose = new ToolButton(this);
    m_btnClose->setObjectName("CustomTitleBarCloseButton");
    m_btnClose->setToolTip(tr("关闭"));
    m_btnClose->setCursor(Qt::PointingHandCursor);
    m_btnClose->setAutoRaise(false);
    m_btnClose->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnClose->setFixedSize(QSize(28, 28));
    m_btnClose->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    m_btnClose->setIconSize(QSize(18, 18));

    auto* h = new QHBoxLayout(this);
    h->setContentsMargins(10, 6, 10, 6);
    h->setSpacing(8);
    h->addWidget(m_titleLabel, 1);
    h->addWidget(m_btnMin, 0);
    h->addWidget(m_btnMax, 0);
    h->addWidget(m_btnClose, 0);
    setLayout(h);

    // 行为连接
    QObject::connect(m_btnMin, &ToolButton::clicked, this, [this]{ if (m_window) m_window->showMinimized(); });
    QObject::connect(m_btnMax, &ToolButton::clicked, this, [this]{ toggleMaximizeRestore(); });
    QObject::connect(m_btnClose, &ToolButton::clicked, this, [this]{ if (m_window) m_window->close(); });
}

void CustomTitleBar::applyThemeStyles()
{
    const bool dark = StyleManagerAdapter::instance()->isCurrentThemeDark();
    const QString titleBg = dark ? QString("#1f1f1f") : QString("#f2f2f2");
    const QString hoverBg = dark ? QString("#3A3A3A") : QString("#e6e6e6");
    const QString pressBg = dark ? QString("#555555") : QString("#d9d9d9");

    setStyleSheet(QString(
        "QWidget#CustomTitleBar { background-color: %1; }\n"
    ).arg(titleBg));

    auto buttonStyle = [&](const QString& obj){
        return QString(
            "QToolButton#%1 {\n"
            "  background: transparent;\n"
            "  border: 0px;\n"
            "  border-radius: 6px;\n"
            "  margin: 0;\n"
            "  padding: 4px;\n"
            "  qproperty-iconSize: 20px;\n"
            "}\n"
            "QToolButton#%1:hover {\n"
            "  background: %2;\n"
            "}\n"
            "QToolButton#%1:pressed {\n"
            "  background: %3;\n"
            "}\n"
        ).arg(obj, hoverBg, pressBg);
    };

    m_btnMin->setStyleSheet(buttonStyle("CustomTitleBarMinButton"));
    m_btnMax->setStyleSheet(buttonStyle("CustomTitleBarMaxButton"));
    m_btnClose->setStyleSheet(buttonStyle("CustomTitleBarCloseButton"));
}

void CustomTitleBar::snapDockIfNeeded(const QPoint& globalReleasePos)
{
    if (!m_window) return;

    QScreen* scr = QApplication::screenAt(globalReleasePos);
    if (!scr) scr = QApplication::primaryScreen();
    if (!scr) return;

    const QRect area = scr->availableGeometry();
    const int thresh = 12; // 贴边阈值

    const bool nearTop = (globalReleasePos.y() - area.top()) <= thresh;
    const bool nearLeft = (globalReleasePos.x() - area.left()) <= thresh;
    const bool nearRight = (area.right() - globalReleasePos.x()) <= thresh;

    if (nearTop) {
        m_window->showMaximized();
        updateForWindowState();
        return;
    }
    if (nearLeft) {
        // 左半屏停靠
        m_window->showNormal();
        m_window->setGeometry(area.left(), area.top(), area.width() / 2, area.height());
        updateForWindowState();
        return;
    }
    if (nearRight) {
        // 右半屏停靠
        m_window->showNormal();
        m_window->setGeometry(area.left() + area.width() / 2, area.top(), area.width() / 2, area.height());
        updateForWindowState();
        return;
    }
}

void CustomTitleBar::toggleMaximizeRestore()
{
    if (!m_window) return;
    if (m_window->isMaximized()) m_window->showNormal(); else m_window->showMaximized();
    updateForWindowState();
}

bool CustomTitleBar::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == m_window && ev->type() == QEvent::WindowStateChange) {
        updateForWindowState();
    }
    return QWidget::eventFilter(obj, ev);
}

void CustomTitleBar::setButtonsVisible(bool showMinimize, bool showMaximize, bool showClose)
{
    if (m_btnMin)   m_btnMin->setVisible(showMinimize);
    if (m_btnMax)   m_btnMax->setVisible(showMaximize);
    if (m_btnClose) m_btnClose->setVisible(showClose);
}

void CustomTitleBar::setDoubleClickToToggleEnabled(bool enabled)
{
    m_doubleClickToggleEnabled = enabled;
}

} // namespace QtAdvancedUI