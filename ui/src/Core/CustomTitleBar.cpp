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

#include "QtAdvancedUI/Core/FramelessWidgetWindow.h"

using namespace acss;

namespace QtAdvancedUI {

CustomTitleBar::CustomTitleBar(QWidget* parent)
    : QWidget(parent)
{
    setProperty("title", QVariant(true));
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    // 统一标题栏高度，确保在 MainWindow 与 Dialog 中一致
    setFixedHeight(40);
    initUI();
}

CustomTitleBar::~CustomTitleBar() = default;

void CustomTitleBar::attachToWindow(QWidget* window)
{
    m_window = window;
    if (!m_window) return;
    // 跟随窗口标题变化
    QObject::connect(m_window, &QWidget::windowTitleChanged, this, &CustomTitleBar::setTitle);
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
    const bool maximized = (m_window->windowState() == Qt::WindowMaximized || isFullScreen());
    const QIcon icon = maximized ? StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/hx_custom_tosmall.svg")
                                 : StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/hx_custom_tobig.svg");
    m_btnMax->setIcon(icon);

    //全屏不显示最大化最小化按钮（通用做法）
    if(isFullScreen())
        setButtonsVisible(false,false,true);
    else
        setButtonsVisible(true,true,true);
}

#ifdef Q_OS_LINUX
void CustomTitleBar::mousePressEvent(QMouseEvent* e)
{
    if (!m_window) return;
    if (m_window->windowState() == Qt::WindowMaximized || isFullScreen()) return;//最大化/全屏禁止拖动
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
        e->accept();
        return;
    }
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
#endif
void CustomTitleBar::initUI()
{
    m_titleLabel = new QLabel(this);
    m_titleLabel->setProperty("title", QVariant(true));

    QSize buttonSize = QSize(height()+5, height());
    QSize iconSize = QSize(18, 18);

    m_btnMin = new ToolButton(this);
    m_btnMin->setObjectName("TitleBarMinButton");
    // m_btnMin->setToolTip(tr("最小化"));
    m_btnMin->setCursor(Qt::PointingHandCursor);
    m_btnMin->setAutoRaise(false);
    m_btnMin->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnMin->setFixedSize(buttonSize);
    m_btnMin->setIcon(StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/hx_custom_hide.svg"));
    m_btnMin->setIconSize(iconSize);

    m_btnMax = new ToolButton(this);
    m_btnMax->setObjectName("TitleBarMaxButton");
    // m_btnMax->setToolTip(tr("最大化/还原"));
    m_btnMax->setCursor(Qt::PointingHandCursor);
    m_btnMax->setAutoRaise(false);
    m_btnMax->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnMax->setFixedSize(buttonSize);
    m_btnMax->setIcon(StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/hx_custom_tobig.svg"));
    m_btnMax->setIconSize(iconSize);

    m_btnClose = new ToolButton(this);
    m_btnClose->setObjectName("TitleBarCloseButton");
    // m_btnClose->setToolTip(tr("关闭"));
    m_btnClose->setCursor(Qt::PointingHandCursor);
    m_btnClose->setAutoRaise(false);
    m_btnClose->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnClose->setFixedSize(buttonSize);
    m_btnClose->setIcon(StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/hx_custom_close.svg"));
    m_btnClose->setIconSize(iconSize);

    auto* h = new QHBoxLayout(this);
    h->setContentsMargins(5, 0, 0, 0);
    h->setSpacing(1);
    h->addWidget(m_titleLabel, 1);
    h->addWidget(m_btnMin, 0);
    h->addWidget(m_btnMax, 0);
    h->addWidget(m_btnClose, 0);
    setLayout(h);

    // 行为连接
    connect(m_btnMin, &ToolButton::clicked, this, [this] {
        FramelessWidgetWindow *window = qobject_cast<FramelessWidgetWindow *>(m_window);
        if (!window) return;
        window->setFramelessWindowState(Minimized);
    });
    connect(m_btnMax, &ToolButton::clicked, this, [this]{ toggleMaximizeRestore(); });
    connect(m_btnClose, &ToolButton::clicked, this, [this]{
        if (m_window) m_window->close();
    });
}

// 简化toggleMaximizeRestore，移除状态记录逻辑
void CustomTitleBar::toggleMaximizeRestore(bool isDoubleClick)
{
    auto *window = qobject_cast<FramelessWidgetWindow *>(m_window);
    if (!window) return;

    if(isDoubleClick && isFullScreen()){
        return;//全屏双击不生效
    }
    if (window->windowState() == Qt::WindowMaximized || isFullScreen()) {
        window->setFramelessWindowState(Normal);
    }
    else {
        window->setFramelessWindowState(Maximized);//默认全部最大化，全屏没有动画效果
    }
/*#ifdef Q_OS_WIN
//     if (window->windowState() == Qt::WindowMaximized || isFullScreen()) {
//         window->showNormal();
//     }
//     else {
//         window->showMaximized();//默认全部最大化，全屏没有动画效果
//     }
// #elif defined(Q_OS_LINUX)
//     //Linux下需要处理缩放动画
//     QPropertyAnimation* animation = new QPropertyAnimation(m_window,"geometry");
//     animation->setDuration(150);
//     animation->setEasingCurve(QEasingCurve::OutQuad);
//     if (m_window->windowState() == Qt::WindowMaximized
//             || isFullScreen()) {
//         QRect startGeometry = m_window->geometry();
//         animation->setStartValue(startGeometry);
//         animation->setEndValue(normalGeometry);
//         //确保执行到位
//         connect(animation,&QPropertyAnimation::finished,this,[this,window]() {
//             window->setFramelessWindowState(Normal);
//             updateForWindowState();
//         });
//     }
//     else {
//         if(m_window->windowState() == Qt::WindowNoState) {
//             normalGeometry = m_window->geometry();
//         }
//         QScreen *primaryScreen = QGuiApplication::screenAt(this->geometry().center());
//         QRect maximizedGeometry = primaryScreen->geometry();
//
//         animation->setStartValue(normalGeometry);
//         animation->setEndValue(maximizedGeometry);
//
//         //确保执行到位
//         connect(animation,&QPropertyAnimation::finished,this,[this,window]() {
//             window->setFramelessWindowState(Maximized);
//             updateForWindowState();
//         });
//     }
//     animation->start(QPropertyAnimation::DeleteWhenStopped);
     if (window->windowState() == Qt::WindowMaximized ||
             isFullScreen()) {
         window->setFramelessWindowState(Normal);
     }
     else {
         window->setFramelessWindowState(Maximized);//默认全部最大化，全屏没有动画效果
     }
// #endif*/
}

bool CustomTitleBar::isFullScreen()
{
    auto *window = qobject_cast<FramelessWidgetWindow *>(m_window);
    if (!window) return false;
    return (window->windowState() == Qt::WindowFullScreen || window->windowState() == (Qt::WindowMaximized|Qt::WindowFullScreen));
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
