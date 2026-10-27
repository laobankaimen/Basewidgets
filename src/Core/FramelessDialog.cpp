/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Core/FramelessDialog.h>
#include <basewidgets/Core/WindowPlacement.h>
#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Inputs/ToolButton.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/ThemeTokens.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QDialogButtonBox>
#include <QStyle>
#include <QPainter>
#include <QPaintEvent>
#include <QGraphicsDropShadowEffect>
#include <QRegion>
#include <QResizeEvent>
#include <QPushButton>
#include <QShowEvent>
#include <QTimer>
#include <QWindow>
#include <QScreen>
#include <QGuiApplication>
#include <QPointer>

#include "basewidgets/Containers/Frame.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <winuser.h>
#include <windowsx.h>
#include <dwmapi.h>

#pragma comment (lib,"Dwmapi.lib")
#pragma comment (lib,"user32.lib")
#endif

using namespace acss;
namespace basewidgets {

FramelessDialog::FramelessDialog(bool popDialog, QWidget* parent)
    : QDialog(parent),isPopDialog(popDialog)
{
    // Frameless and styled
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    // Modal dialogs need normal activation on Wayland as well as X11/Windows.
    // Do not suppress normal activation when the compositor returns from a native drag.
    initUI();
    WindowPlacement::watch(this);
    updateSurfacePalette();
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,&FramelessDialog::updateSurfacePalette);
}

FramelessDialog::FramelessDialog(QWidget* parent)
    : FramelessDialog(true, parent)
{
}

FramelessDialog::~FramelessDialog() = default;

void FramelessDialog::updateSurfacePalette()
{
    // Window role is inherited by the title/content/footer. This keeps one continuous
    // dialog surface while Base remains available for input fields and item views.
    QPalette surface=ThemeTokens::instance()->palette();
    surface.setColor(QPalette::Window,ThemeTokens::instance()->bg1());
    setPalette(surface);
    update();
}

void FramelessDialog::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(),palette().window());
    // Draw inside a reserved logical-pixel inset; child backgrounds cannot cover the border.
    painter.setPen(QPen(ThemeTokens::instance()->border(),1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF(rect()).adjusted(.5,.5,-.5,-.5));
    if (btnBox && btnBox->parentWidget()->isVisible()) {
        const int y=btnBox->parentWidget()->mapTo(this,QPoint()).y();
        painter.setPen(ThemeTokens::instance()->borderLight());
        painter.drawLine(16,y,width()-17,y);
    }
}

void FramelessDialog::changeEvent(QEvent* event)
{
    QDialog::changeEvent(event);
    if (event->type()==QEvent::WindowStateChange)
        if (auto* title=qobject_cast<CustomTitleBar*>(m_titleBar)) title->updateForWindowState();
}

void FramelessDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    if (m_centeredOnce) return;
    m_centeredOnce=true;
    QPointer<FramelessDialog> self(this);
    QTimer::singleShot(0,this,[self] {
        if (self && self->isVisible()) WindowPlacement::centerOnParent(self);
    });
}

void FramelessDialog::initUI()
{
    // 自定义标题栏（包含最小化/最大化/关闭、拖动/停靠）
    auto* customTitleBar = new CustomTitleBar(this);
    customTitleBar->attachToWindow(this);
    customTitleBar->setFixedHeight(m_titlebarHeight);
    // 对话框：仅显示关闭按钮，禁用双击放大
    customTitleBar->setButtonsVisible(false, false, true);
    customTitleBar->setDoubleClickToToggleEnabled(false);

    // 设置标题栏
    m_titleBar = customTitleBar;

    // Content host
    m_contentHost = new QWidget(this);
    m_contentHost->setProperty("bwMsgContainer", QVariant(true));
    m_contentHost->setProperty("msgContainer", QVariant(true));

    // Main layout
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(1, 1, 1, 1);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_contentHost, 1);
    //dialog buttons
    if(isPopDialog) {
        auto btnWidget = new QFrame(this);
        btnWidget->setProperty("bwDialogFooter", QVariant(true));
        btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,this);
        btnBox->setObjectName("dialogBtnBox");
        btnBox->setContentsMargins(0,0,0,0);
        // Explicitly clear standard icons too: a platform theme may have initialized
        // them before style polishing. Caller-created icon buttons are untouched.
        for (auto* button : btnBox->buttons()) button->setIcon(QIcon());
        if (auto* okBtn = btnBox->button(QDialogButtonBox::Ok)) {
            okBtn->setText(QObject::tr("确定"));
            okBtn->setProperty("bwPrimary", QVariant(true));
            okBtn->setProperty("primary", QVariant(true));
        }
        if (auto* cancelBtn = btnBox->button(QDialogButtonBox::Cancel)) cancelBtn->setText(QObject::tr("取消"));
        QObject::connect(btnBox, &QDialogButtonBox::accepted, this, &FramelessDialog::okButtonClicked);
        QObject::connect(btnBox, &QDialogButtonBox::rejected, this, &FramelessDialog::cancelButtonClicked);
        auto* btnLayout = new QVBoxLayout(btnWidget);
        btnLayout->setContentsMargins(16, 12, 16, 16);
        btnLayout->addWidget(btnBox);
        mainLayout->addWidget(btnWidget);
    }

    // Default sizing to avoid too small dialogs
    setMinimumSize(380, 240);
}

void FramelessDialog::setTitle(const QString& title)
{
    setWindowTitle(title);
}

void FramelessDialog::setContentWidget(QWidget* widget)
{
    if (!m_contentHost || widget == m_contentWidget) return;
    auto* layout = m_contentHost->layout();
    if (!layout) {
        layout = new QVBoxLayout(m_contentHost);
        layout->setContentsMargins(12, 12, 12, 0);
    }
    if (m_contentWidget) {
        layout->removeWidget(m_contentWidget);
        m_contentWidget->hide();
        m_contentWidget->deleteLater();
    }
    m_contentWidget = widget;
    if (widget) {
        layout->addWidget(widget);
        widget->show();
    }
}

void FramelessDialog::setResizeable(bool canResize) {
    if (auto* customTitleBar = qobject_cast<CustomTitleBar*>(m_titleBar)) {
        customTitleBar->setButtonsVisible(canResize, canResize, true);
        customTitleBar->setDoubleClickToToggleEnabled(canResize);
    }
    setSizeGripEnabled(canResize);
}

void FramelessDialog::setCustomDialogBtn(bool showOK, bool showCancel, bool okHighLight) {
    if(!isPopDialog) return;
    QPushButton* okBtn = btnBox->button(QDialogButtonBox::Ok);
    if(okBtn) {
        okBtn->setVisible(showOK);
        okBtn->setDefault(showOK);
        okBtn->setAutoDefault(showOK);
        okBtn->setProperty("bwPrimary", okHighLight);
        okBtn->setProperty("primary", okHighLight);
    }
    QPushButton* cancelBtn = btnBox->button(QDialogButtonBox::Cancel);
    if(cancelBtn)  cancelBtn->setVisible(showCancel);
}

void FramelessDialog::setFramelessWindowState(FramelessWindowState state)
{
    // Use the same Qt path for every platform; avoid hand-written X11 state messages.
    switch (state) {
    case Maximized: showMaximized(); break;
    case Minimized: showMinimized(); break;
    case FullScreen: showFullScreen(); break;
    case Normal: showNormal(); break;
    }
}

#ifdef Q_OS_WIN
bool FramelessDialog::nativeEvent(const QByteArray &eventType, void *message, NativeEventResult *result) {
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
            POINT clientPoint{x,y}; ScreenToClient(msg->hwnd,&clientPoint);
            const qreal dpr=devicePixelRatioF();
            const QPoint titlePos=m_titleBar->mapFrom(this,QPoint(qRound(clientPoint.x/dpr),qRound(clientPoint.y/dpr)));

            QWidget* child = m_titleBar->childAt(titlePos);
            //鼠标标题栏的子控件，让子控件接收事件（过滤掉其他不需要鼠标交互的控件）
            if (child && (child->objectName().contains("TitleBarCloseButton") ||
                child->objectName().contains("TitleBarMaxButton") ||
                child->objectName().contains("TitleBarMinButton"))) {
                *result = HTCLIENT;
                return true;
            }
            //不在子控件上，且在标题栏内
            if (m_titleBar->rect().contains(titlePos)) {
                *result = HTCAPTION;
                return true;
            }
        }
        *result = HTCLIENT;
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
    void FramelessDialog::reject() {
        if (isPopDialog)
            QDialog::reject();
        else
            Q_EMIT onDialogClosing();
    }
} // namespace basewidgets
