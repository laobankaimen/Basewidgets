#include <complex.h>
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
#include <QPainter>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWindowStateChangeEvent>

#include "QtAdvancedUI/Core/FramelessMainWindow.h"
#include "QtAdvancedUI/Core/FramelessWidgetWindow.h"

using namespace acss;

namespace QtAdvancedUI {

CustomTitleBar::CustomTitleBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("CustomTitleBar");
    setAttribute(Qt::WA_StyledBackground, true);
    // 统一标题栏高度，确保在 MainWindow 与 Dialog 中一致
    setFixedHeight(36);
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
    //最小化可能有两种状态
    const bool maximized = (m_window->windowState() == Qt::WindowMaximized || m_window->windowState() == Qt::WindowFullScreen);
    const QIcon icon = maximized ? StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/hx_custom_tosmall.svg")
                                 : StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/hx_custom_tobig.svg");
    m_btnMax->setIcon(icon);
}

// 移除所有拖动相关的实现，因为framelesswindow会处理拖动
void CustomTitleBar::mousePressEvent(QMouseEvent* e)
{
    // 不再处理拖动，让framelesswindow处理
    QWidget::mousePressEvent(e);
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent* e)
{
    // 不再处理拖动，让framelesswindow处理
    QWidget::mouseMoveEvent(e);
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent* e)
{
    // 不再处理拖动，让framelesswindow处理
    QWidget::mouseReleaseEvent(e);
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (!m_window) return;
    if (m_doubleClickToggleEnabled && e->button() == Qt::LeftButton) {
        toggleMaximizeRestore(true);
        e->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(e);
}

void CustomTitleBar::showEvent(QShowEvent *event) {
    // 移除所有最小化恢复逻辑，让framelesswindow处理
    QWidget::showEvent(event);
}

void CustomTitleBar::initUI()
{
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("CustomTitleBarTitle");
    m_titleLabel->setStyleSheet("font-size: 15px; font-weight: 400;");

    m_btnMin = new ToolButton(this);
    m_btnMin->setObjectName("TitleBarMinButton");
    m_btnMin->setToolTip(tr("最小化"));
    m_btnMin->setCursor(Qt::PointingHandCursor);
    m_btnMin->setAutoRaise(false);
    m_btnMin->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnMin->setFixedSize(QSize(30, 30));
    m_btnMin->setIcon(StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/hx_custom_hide.svg"));
    m_btnMin->setIconSize(QSize(18, 18));

    m_btnMax = new ToolButton(this);
    m_btnMax->setObjectName("TitleBarMaxButton");
    m_btnMax->setToolTip(tr("最大化/还原"));
    m_btnMax->setCursor(Qt::PointingHandCursor);
    m_btnMax->setAutoRaise(false);
    m_btnMax->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnMax->setFixedSize(QSize(30, 30));
    m_btnMax->setIcon(StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/hx_custom_tobig.svg"));
    m_btnMax->setIconSize(QSize(18, 18));

    m_btnClose = new ToolButton(this);
    m_btnClose->setObjectName("TitleBarCloseButton");
    m_btnClose->setToolTip(tr("关闭"));
    m_btnClose->setCursor(Qt::PointingHandCursor);
    m_btnClose->setAutoRaise(false);
    m_btnClose->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnClose->setFixedSize(QSize(30, 30));
    m_btnClose->setIcon(StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/hx_custom_close.svg"));
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
    connect(m_btnMin, &ToolButton::clicked, this, [this] { if (m_window)  m_window->showMinimized(); });
    connect(m_btnMax, &ToolButton::clicked, this, [this]{ toggleMaximizeRestore(); });
    connect(m_btnClose, &ToolButton::clicked, this, [this]{ if (m_window) m_window->close(); });

    //主题切换
    connect(StyleManagerAdapter::instance(), &acss::QtAdvancedStylesheet::stylesheetChanged, this, [this] {
        update();
    });
}

void CustomTitleBar::applyThemeStyles()
{
}

void CustomTitleBar::setCornerRadius(int radiusPx)
{
    m_cornerRadius = qMax(0, radiusPx);
    applyThemeStyles();
}

void CustomTitleBar::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(StyleManagerAdapter::instance()->generateThemePalette().window());

    const QRect r = rect();
    const int radius = qMax(0, m_cornerRadius);
    if (radius > 0) {
        QPainterPath path;
        // 仅顶部圆角，底部保持直角以贴合内容区域
        path.moveTo(r.left(), r.bottom());
        path.lineTo(r.left(), r.top() + radius);
        path.quadTo(r.left(), r.top(), r.left() + radius, r.top());
        path.lineTo(r.right() - radius, r.top());
        path.quadTo(r.right(), r.top(), r.right(), r.top() + radius);
        path.lineTo(r.right(), r.bottom());
        path.lineTo(r.left(), r.bottom());
        path.closeSubpath();
        p.drawPath(path);
    } else {
        p.drawRect(r);
    }
}

// 移除贴边停靠逻辑，因为framelesswindow会处理
void CustomTitleBar::snapDockIfNeeded(const QPoint& globalReleasePos)
{
    Q_UNUSED(globalReleasePos)
    // 不再处理贴边停靠，让framelesswindow处理
}

// 简化toggleMaximizeRestore，移除状态记录逻辑
void CustomTitleBar::toggleMaximizeRestore(bool isDoubleClick)
{
    if (!m_window) return;

    //直接判断窗口状态
    if (m_window->windowState() == Qt::WindowMaximized || m_window->windowState() == Qt::WindowFullScreen) {
        QPropertyAnimation* animation = new QPropertyAnimation(m_window,"geometry");
        animation->setDuration(150);
        animation->setEasingCurve(QEasingCurve::OutQuad);

        QRect startGeometry = m_window->geometry();
        animation->setStartValue(startGeometry);
        animation->setEndValue(normalGeometry);

        //确保执行到位
        connect(animation,&QPropertyAnimation::finished,this,[this]() {
            m_window->showNormal();
            m_window->setGeometry(normalGeometry);
            updateForWindowState();
        });
        animation->start(QPropertyAnimation::DeleteWhenStopped);
    }
    else {
        normalGeometry = m_window->geometry();
        QScreen *primaryScreen = QGuiApplication::screenAt(this->geometry().center());
        QRect maximizedGeometry = primaryScreen->geometry();
        QPropertyAnimation *animation = new QPropertyAnimation(m_window,"geometry");
        animation->setDuration(150);
        animation->setEasingCurve(QEasingCurve::OutQuad);

        animation->setStartValue(normalGeometry);
        animation->setEndValue(maximizedGeometry);

        //确保执行到位
        connect(animation,&QPropertyAnimation::finished,this,[this,isDoubleClick]() {
            auto window = dynamic_cast<FramelessMainWindow*>(m_window);
            if(isDoubleClick)
                m_window->showFullScreen();
            else
                m_window->showMaximized();
            updateForWindowState();
        });
        animation->start(QPropertyAnimation::DeleteWhenStopped);
    }
}

// 简化事件过滤器，移除状态记录逻辑
bool CustomTitleBar::eventFilter(QObject* obj, QEvent* ev)
{
    if(obj == m_window && ev->type() == QEvent::WindowStateChange) {
        // 处理最大化/全屏状态的边缘调整
        if(m_window->windowState() == Qt::WindowMaximized || m_window->windowState() == Qt::WindowFullScreen) {
            if (auto mw = dynamic_cast<FramelessMainWindow*>(m_window)) {
                mw->setResizableEdgesEnabled(false);
            } else if (auto ww = dynamic_cast<FramelessWidgetWindow*>(m_window)) {
                ww->setResizableEdgesEnabled(false);
            }
        }
        else if (m_window->windowState() == Qt::WindowNoState) {
            // 确保从最大化/全屏恢复时重新启用边缘调整
            if (auto mw = dynamic_cast<FramelessMainWindow*>(m_window)) {
                mw->setResizableEdgesEnabled(true);
            } else if (auto ww = dynamic_cast<FramelessWidgetWindow*>(m_window)) {
                ww->setResizableEdgesEnabled(true);
            }
        }

        // 更新窗口状态显示
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