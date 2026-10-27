/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/Inputs/ToolButton.h>
#include <basewidgets/Inputs/ElidedLabel.h>

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
    setMouseTracking(true);
    // 统一标题栏高度，确保在 MainWindow 与 Dialog 中一致
    setFixedHeight(fixedHieght);
    initUI();
    connect(StyleManagerAdapter::instance(), &acss::QtAdvancedStylesheet::themeChanged, this, &CustomTitleBar::updateIcons);
    updateIcons();
}

CustomTitleBar::~CustomTitleBar() = default;

void CustomTitleBar::attachToWindow(QWidget* window)
{
    if (m_window) disconnect(m_window, nullptr, this, nullptr);
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
    if (m_titleLabel) { m_titleLabel->setText(title); m_titleLabel->setToolTip(title); }
}

void CustomTitleBar::updateIcons()
{
    // Window-control glyphs are drawn by NativeStyle, keeping contrast on red hover surfaces.

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
        int iconSize = 20;
        m_iconLabel->setPixmap(icon.pixmap(QSize(iconSize, iconSize)));
    }
}

void CustomTitleBar::updateForWindowState()
{
    if (!m_window || !m_btnMax) return;
    const bool maximized = (m_window->windowState() == Qt::WindowMaximized || isFullScreen());
    m_btnMax->setProperty("bwWindowControl",maximized ? "restore" : "maximize");
    m_btnMax->setToolTip(maximized ? tr("还原") : tr("最大化"));
    m_btnMax->setAccessibleName(m_btnMax->toolTip());

    m_btnMin->setVisible(m_showMinimize && !isFullScreen());
    m_btnMax->setVisible(m_showMaximize && !isFullScreen());
    m_btnClose->setVisible(m_showClose);

}

bool CustomTitleBar::event(QEvent* event)
{
    // Compositors can consume the release during native move. Do not leave the
    // fallback drag active after a grab is cancelled, the window deactivates or hides.
    if (event->type()==QEvent::UngrabMouse || event->type()==QEvent::WindowDeactivate
        || event->type()==QEvent::Hide) {
        m_dragging=false;
        if (event->type()!=QEvent::UngrabMouse && QWidget::mouseGrabber()==this) releaseMouse();
    }
    return QWidget::event(event);
}

#ifndef Q_OS_WIN
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
        m_dragging=false;
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        if (QWindow* handle = m_window->windowHandle()) {
            if (handle->startSystemMove()) {
                // The compositor owns the drag. Do not retain an explicit widget grab.
                if (QWidget::mouseGrabber()==this) releaseMouse();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                if (QGuiApplication::platformName().startsWith("wayland")) {
                    // Qt5 Widgets keeps an implicit pressed receiver until a release
                    // passes through QWidgetWindow. Wayland may consume the real release
                    // during xdg_toplevel.move, otherwise later hover stays on the title.
                    // Send one Qt-only release AFTER the initiating press finishes. It
                    // cannot activate a button: this handoff starts on title-bar blank space.
                    const QPointer<QWindow> window=handle;
                    const QPoint local=m_window->mapFromGlobal(e->globalPos());
                    const QPoint global=e->globalPos();
                    const Qt::MouseButtons remaining=e->buttons() & ~Qt::LeftButton;
                    const Qt::KeyboardModifiers modifiers=e->modifiers();
                    QTimer::singleShot(0,this,[window,local,global,remaining,modifiers] {
                        if (!window || !window->isVisible()) return;
                        QMouseEvent release(QEvent::MouseButtonRelease,local,local,global,
                                            Qt::LeftButton,remaining,modifiers,Qt::MouseEventSynthesizedByApplication);
                        QCoreApplication::sendEvent(window,&release);
                    });
                }
#endif
                e->accept();
                return;
            }
        }
#endif
        // QWidget::move is not a supported fallback on Wayland and can leave a
        // misleading drag state when the compositor rejected an input serial.
        if (QGuiApplication::platformName().startsWith("wayland")) {
            e->ignore();
            return;
        }
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
    if (m_dragging && !(e->buttons() & Qt::LeftButton)) {
        m_dragging=false; // missed release: next mouse motion recovers instead of dragging forever
        if (QWidget::mouseGrabber()==this) releaseMouse();
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
        if (QWidget::mouseGrabber()==this) releaseMouse();
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
    m_iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_titleLabel = new ElidedLabel(this);
    m_titleLabel->setObjectName("TitleLabel");
    m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_titleLabel->setProperty("title", QVariant(true));
    // 确保标题文字左对齐并垂直居中
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QSize buttonSize = QSize(height()+5, height());
    QSize btnIconSize = QSize(18, 18);

    m_btnMin = new ToolButton(this);
    m_btnMin->setObjectName("TitleBarMinButton");
    m_btnMin->setToolTip(tr("最小化"));
    m_btnMin->setAccessibleName(tr("最小化"));
    m_btnMin->setProperty("bwWindowControl","minimize");
    m_btnMin->setCursor(Qt::PointingHandCursor);
    m_btnMin->setAutoRaise(true);
    m_btnMin->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnMin->setFixedSize(buttonSize);
    m_btnMin->setIconSize(btnIconSize);

    m_btnMax = new ToolButton(this);
    m_btnMax->setObjectName("TitleBarMaxButton");
    m_btnMax->setToolTip(tr("最大化/还原"));
    m_btnMax->setAccessibleName(tr("最大化/还原"));
    m_btnMax->setProperty("bwWindowControl","maximize");
    m_btnMax->setCursor(Qt::PointingHandCursor);
    m_btnMax->setAutoRaise(true);
    m_btnMax->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnMax->setFixedSize(buttonSize);
    m_btnMax->setIconSize(btnIconSize);

    m_btnClose = new ToolButton(this);
    m_btnClose->setObjectName("TitleBarCloseButton");
    m_btnClose->setToolTip(tr("关闭"));
    m_btnClose->setAccessibleName(tr("关闭"));
    m_btnClose->setProperty("bwWindowControl","close");
    m_btnClose->setAppearance(ButtonAppearance::Danger);
    m_btnClose->setCursor(Qt::PointingHandCursor);
    m_btnClose->setAutoRaise(true);
    m_btnClose->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_btnClose->setFixedSize(buttonSize);
    m_btnClose->setIconSize(btnIconSize);

    auto* h = new QHBoxLayout(this);
    h->setContentsMargins(5, 0, 0, 0);
    h->setSpacing(0); // Caption buttons share contiguous hit regions, as desktop users expect.
    h->addWidget(m_iconLabel, 0);
    h->addWidget(m_titleLabel, 1);
    h->addWidget(m_btnMin, 0);
    h->addWidget(m_btnMax, 0);
    h->addWidget(m_btnClose, 0);
    setLayout(h);

    // 行为连接
    connect(m_btnMin, &ToolButton::clicked, this, [this] {
        auto *window = dynamic_cast<BaseWidgetInterface *>(m_window.data());
        if (window) window->setFramelessWindowState(Minimized);
        else if (m_window) m_window->showMinimized();
    });
    connect(m_btnMax, &ToolButton::clicked, this, [this]{ toggleMaximizeRestore(); });
    connect(m_btnClose, &ToolButton::clicked, this, [this]{
        if (m_window) m_window->close();
    });
}

// 简化toggleMaximizeRestore，移除状态记录逻辑
void CustomTitleBar::toggleMaximizeRestore(bool isDoubleClick)
{
    if (!m_window) return;
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
    auto *window = dynamic_cast<BaseWidgetInterface *>(m_window.data());
    if (window) window->setFramelessWindowState(states);
    else if (m_window) {
        if (states == Maximized) m_window->showMaximized();
        else m_window->showNormal();
    }

}

bool CustomTitleBar::isFullScreen()
{
    return m_window && m_window->isFullScreen();
}

void CustomTitleBar::setButtonsVisible(bool showMinimize, bool showMaximize, bool showClose)
{
    m_showMinimize = showMinimize;
    m_showMaximize = showMaximize;
    m_showClose = showClose;
    if (m_btnMin) m_btnMin->setVisible(showMinimize && !isFullScreen());
    if (m_btnMax) m_btnMax->setVisible(showMaximize && !isFullScreen());
    if (m_btnClose) m_btnClose->setVisible(showClose);
}

void CustomTitleBar::setDoubleClickToToggleEnabled(bool enabled)
{
    m_doubleClickToggleEnabled = enabled;
}

} // namespace basewidgets
