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

#include "QtAdvancedUI/Core/FramelessMainWindow.h"

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
    if (m_window->windowState() == Qt::WindowMaximized || m_window->windowState() == Qt::WindowFullScreen) return;//全屏禁止拖动
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

void CustomTitleBar::showEvent(QShowEvent *event) {
#ifdef Q_OS_WIN
    ;
#endif
#ifdef Q_OS_LINUX
    if(m_window->windowState() == Qt::WindowMinimized) {
        if(m_lastState == Qt::WindowMaximized)
            m_window->showMaximized();
        else if(m_lastState ==  Qt::WindowFullScreen)
            m_window->showFullScreen();
        else
            m_window->showNormal();
        m_window->setAttribute(Qt::WA_Mapped);
        m_window->update();
    }
#endif
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
            qDebug()<<m_window->windowState()<<normalGeometry;
            m_window->showNormal();
            m_window->setGeometry(normalGeometry);
            updateForWindowState();
            qDebug()<<"nos----------"<<m_window->windowState();
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
            //m_window->setGeometry(maximizedGeometry);
            updateForWindowState();
            qDebug()<<"full----------"<<m_window->windowState();
        });
        animation->start(QPropertyAnimation::DeleteWhenStopped);
    }

}

bool CustomTitleBar::eventFilter(QObject* obj, QEvent* ev)
{
    if(obj == m_window && ev->type() == QEvent::WindowStateChange) {
        if(m_window->windowState() == Qt::WindowMaximized ||m_window->windowState() == Qt::WindowFullScreen) {
            auto window = dynamic_cast<FramelessMainWindow*>(m_window);
            window->setResizableEdgesEnabled(false);//全屏禁止角拖拽缩放
        }
        else {
            auto window = dynamic_cast<FramelessMainWindow*>(m_window);
            window->setResizableEdgesEnabled(true);
        }

        /*auto *stateEvent = dynamic_cast<QWindowStateChangeEvent*>(ev);
        auto oldState = stateEvent->oldState();
        //记录最小化之前的状态
        if(!(oldState & Qt::WindowMinimized))
            m_lastState = oldState;
        if(!m_window->isMinimized())
            m_lastState = m_window->windowState();//非最小化状态时记录状态

        //当前已经是最小化状态，强制系统切换回上一次的状态
        if(m_isMinimized) {
            m_isMinimized = false;
        }
        else {
            if(m_window->windowState() == (Qt::WindowMaximized|Qt::WindowMinimized) || m_window->windowState() == (Qt::WindowFullScreen|Qt::WindowMinimized))
                m_window->showMinimized();
            else if(m_window->windowState() == Qt::WindowMinimized) {
                m_isMinimized = true;
            }
        }*/
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