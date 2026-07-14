/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/Inputs/ToolButton.h>

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
#include <QWindow>

#include "basewidgets/Core/BaseWidgetInterface.h"

using namespace acss;

namespace basewidgets {
static int fixedHieght = 40;
CustomTitleBar::CustomTitleBar(QWidget* parent)
    : QWidget(parent)
{
    setProperty("title", QVariant(true));
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    // 统一标题栏高度，确保在 MainWindow 与 Dialog 中一致
    setFixedHeight(fixedHieght);
    initUI();
    connect(StyleManagerAdapter::instance(), &acss::QtAdvancedStylesheet::stylesheetChanged, this, &CustomTitleBar::updateIcons);
    updateIcons();
}

CustomTitleBar::~CustomTitleBar() = default;

void CustomTitleBar::attachToWindow(QWidget* window)
{
    m_window = window;
    if (!m_window) return;
    // 跟随窗口标题变化
    QObject::connect(m_window, &QWidget::windowTitleChanged, this, &CustomTitleBar::setTitle);
    // 跟随窗口图标变化
    QObject::connect(m_window, &QWidget::windowIconChanged, this, &CustomTitleBar::setIcon);

    setTitle(m_window->windowTitle());
    if (!m_window->windowIcon().isNull()) {
        setIcon(m_window->windowIcon());
    }
    updateForWindowState();
}

void CustomTitleBar::setTitle(const QString& title)
{
    if (m_titleLabel) m_titleLabel->setText(title);
}

void CustomTitleBar::updateIcons()
{
    if (m_btnMin) m_btnMin->setIcon(QIcon("bwicon:buttons/hx_custom_hide.svg"));
    if (m_btnClose) m_btnClose->setIcon(QIcon("bwicon:buttons/hx_custom_close.svg"));

    QIcon icon = StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/material_icon.svg");
    // 更新左上角图标（确保跟随主题颜色变化）
    if(m_window && !m_window->windowIcon().isNull()) {
        icon = m_window->windowIcon();
    }
    else if(!QApplication::windowIcon().isNull()) {
        icon = QApplication::windowIcon();
    }
    setIcon(icon);

    updateForWindowState();
}

void CustomTitleBar::setIcon(const QIcon& icon)
{
    if (m_iconLabel) {
        int iconSize = 30;
        m_iconLabel->setPixmap(icon.pixmap(QSize(iconSize, iconSize)));
    }
}

void CustomTitleBar::updateForWindowState()
{
    if (!m_window || !m_btnMax) return;
    const bool maximized = (m_window->windowState() == Qt::WindowMaximized || isFullScreen());
    const QIcon icon = maximized ? QIcon("bwicon:buttons/hx_custom_tosmall.svg")
                                 : QIcon("bwicon:buttons/hx_custom_tobig.svg");
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
    if (!m_window) {
        QWidget::mousePressEvent(e);
        return;
    }
    if (m_window->windowState() == Qt::WindowMaximized || isFullScreen()) {
        QWidget::mousePressEvent(e);
        return;
    }
    if (e->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        if (QWindow* handle = m_window->windowHandle()) {
            if (handle->startSystemMove()) {
                e->accept();
                return;
            }
        }
#endif
        m_dragging = true;
        m_dragStartPos = e->globalPos() - m_window->frameGeometry().topLeft();
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_window) {
        QWidget::mouseMoveEvent(e);
        return;
    }
    if (m_dragging) {
        m_window->move(e->globalPos() - m_dragStartPos);
        e->accept();
        return;
    }
    QWidget::mouseMoveEvent(e);
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_window) {
        QWidget::mouseReleaseEvent(e);
        return;
    }
    if (m_dragging && e->button() == Qt::LeftButton) {
        m_dragging = false;
        e->accept();
        return;
    }
    QWidget::mouseReleaseEvent(e);
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (!m_window) {
        QWidget::mouseDoubleClickEvent(e);
        return;
    }
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
    m_iconLabel = new QLabel(this);
    // 调整图标大小为 20x20，避免占满整个标题栏高度 (40px)
    int iconSize = 20;
    m_iconLabel->setPixmap(StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/material_icon.svg").pixmap(QSize(iconSize, iconSize)));
    // 移除手动设置的边距，使用布局间距控制
    m_iconLabel->setContentsMargins(0, 0, 0, 0);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("TitleLabel");
    m_titleLabel->setProperty("title", QVariant(true));
    // 确保标题文字左对齐并垂直居中
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QSize buttonSize = QSize(height()+5, height());
    QSize btnIconSize = QSize(18, 18);

    m_btnMin = new ToolButton(this);
    m_btnMin->setObjectName("TitleBarMinButton");
    // m_btnMin->setToolTip(tr("最小化"));
    m_btnMin->setCursor(Qt::PointingHandCursor);
    m_btnMin->setAutoRaise(false);
    m_btnMin->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnMin->setFixedSize(buttonSize);
    m_btnMin->setIconSize(btnIconSize);

    m_btnMax = new ToolButton(this);
    m_btnMax->setObjectName("TitleBarMaxButton");
    // m_btnMax->setToolTip(tr("最大化/还原"));
    m_btnMax->setCursor(Qt::PointingHandCursor);
    m_btnMax->setAutoRaise(false);
    m_btnMax->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnMax->setFixedSize(buttonSize);
    m_btnMax->setIconSize(btnIconSize);

    m_btnClose = new ToolButton(this);
    m_btnClose->setObjectName("TitleBarCloseButton");
    // m_btnClose->setToolTip(tr("关闭"));
    m_btnClose->setCursor(Qt::PointingHandCursor);
    m_btnClose->setAutoRaise(false);
    m_btnClose->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnClose->setFixedSize(buttonSize);
    m_btnClose->setIconSize(btnIconSize);

    auto* h = new QHBoxLayout(this);
    h->setContentsMargins(5, 0, 0, 0);
    h->setSpacing(2); // 减小图标与标题的间距
    h->addWidget(m_iconLabel, 0);
    h->addWidget(m_titleLabel, 1);
    h->addWidget(m_btnMin, 0);
    h->addWidget(m_btnMax, 0);
    h->addWidget(m_btnClose, 0);
    setLayout(h);

    // 行为连接
    connect(m_btnMin, &ToolButton::clicked, this, [this] {
        auto *window = dynamic_cast<BaseWidgetInterface *>(m_window);
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
    if(isDoubleClick && isFullScreen()){
        return;//全屏双击不生效
    }
    FramelessWindowState states;
    if (m_window->windowState() == Qt::WindowMaximized || isFullScreen()) {
        states = Normal;
    }
    else {
        states = Maximized;//默认全部最大化，全屏没有动画效果
    }
    auto *window = dynamic_cast<BaseWidgetInterface *>(m_window);
    if (!window) return;
    window->setFramelessWindowState(states);
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
    return (m_window->windowState() == Qt::WindowFullScreen || m_window->windowState() == (Qt::WindowMaximized|Qt::WindowFullScreen));
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

} // namespace basewidgets
