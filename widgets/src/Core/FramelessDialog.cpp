/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Core/FramelessDialog.h>
#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Inputs/ToolButton.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

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
#elif defined(Q_OS_LINUX)
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
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
#if defined(Q_OS_LINUX)
    if (!QX11Info::isPlatformX11() || QX11Info::display() == nullptr) {
        setAttribute(Qt::WA_ShowWithoutActivating, true);
    }
#endif

    initUI();
}

FramelessDialog::FramelessDialog(QWidget* parent)
    : FramelessDialog(true, parent)
{
}

FramelessDialog::~FramelessDialog() = default;

void FramelessDialog::showEvent(QShowEvent* e)
{
    QDialog::showEvent(e);
#if defined(Q_OS_LINUX)
    if (QX11Info::isPlatformX11() && QX11Info::display() != nullptr) return;
    if (m_centeredOnce) return;
    m_centeredOnce = true;

    QPointer<FramelessDialog> self(this);
    QTimer::singleShot(0, this, [self]() {
        if (!self) return;

        QWidget* parent = self->parentWidget();
        QWidget* parentWindowWidget = parent ? parent->window() : nullptr;

        QWindow* parentWindowHandle = parentWindowWidget ? parentWindowWidget->windowHandle() : nullptr;
        if (QWindow* handle = self->windowHandle()) {
            if (parentWindowHandle) {
                handle->setTransientParent(parentWindowHandle);
            }
        }

        QRect refRect;
        if (parentWindowWidget) {
            refRect = parentWindowWidget->frameGeometry();
        } else if (self->screen()) {
            refRect = self->screen()->availableGeometry();
        } else if (QGuiApplication::primaryScreen()) {
            refRect = QGuiApplication::primaryScreen()->availableGeometry();
        }

        if (!refRect.isValid()) return;

        QSize dialogSize = self->frameGeometry().size();
        if (dialogSize.isEmpty()) dialogSize = self->size();
        if (dialogSize.isEmpty()) dialogSize = self->sizeHint();
        if (dialogSize.isEmpty()) return;

        const QPoint target = refRect.center() - QPoint(dialogSize.width() / 2, dialogSize.height() / 2);
        if (QWindow* handle = self->windowHandle()) {
            handle->setPosition(target);
        } else {
            self->move(target);
        }
    });
#endif
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
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_contentHost, 1);
    //dialog buttons
    if(isPopDialog) {
        auto btnWidget = new QFrame(this);
        btnWidget->setProperty("bwDialogFooter", QVariant(true));
        btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,this);
        btnBox->setObjectName("dialogBtnBox");
        btnBox->setContentsMargins(5,5,5,5);
        if (auto* okBtn = btnBox->button(QDialogButtonBox::Ok)) {
            okBtn->setText(QObject::tr("确定"));
            okBtn->setProperty("bwPrimary", QVariant(true));
            okBtn->setProperty("primary", QVariant(true));
        }
        if (auto* cancelBtn = btnBox->button(QDialogButtonBox::Cancel)) cancelBtn->setText(QObject::tr("取消"));
        QObject::connect(btnBox, &QDialogButtonBox::accepted, this, &FramelessDialog::okButtonClicked);
        QObject::connect(btnBox, &QDialogButtonBox::rejected, this, &FramelessDialog::cancelButtonClicked);
        auto* btnLayout = new QVBoxLayout(btnWidget);
        btnLayout->setContentsMargins(5, 10, 5, 5);
        btnLayout->addWidget(btnBox);
        mainLayout->addWidget(btnWidget);
    }
    setLayout(mainLayout);

    // Default sizing to avoid too small dialogs
    setMinimumSize(380, 240);
}

void FramelessDialog::setTitle(const QString& title)
{
    auto* customTitleBar = qobject_cast<CustomTitleBar*>(m_titleBar);
    if (customTitleBar) {
        customTitleBar->setTitle(title);
    }
}

void FramelessDialog::setContentWidget(QWidget* w)
{
    if (m_contentHost && w) {
        w->setProperty("bwMsgContent", QVariant(true));
        w->setProperty("msgContent", QVariant(true));
        auto* contentLayout = new QVBoxLayout(m_contentHost);
        contentLayout->setContentsMargins(5, 5, 5, 0);
        contentLayout->insertWidget(0,w);
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
        okBtn->setProperty("bwPrimary", okHighLight);
        okBtn->setProperty("primary", okHighLight);
    }
    QPushButton* cancelBtn = btnBox->button(QDialogButtonBox::Cancel);
    if(cancelBtn)  cancelBtn->setVisible(showCancel);
}

void FramelessDialog::setFramelessWindowState(FramelessWindowState state) {
#ifdef Q_OS_WIN
    switch (state) {
        case Maximized:
            showMaximized();
        break;
        case Minimized:
            showMinimized();
        break;
        case FullScreen:
            showFullScreen();
        break;
        case Normal:
            showNormal();
        break;
    }
#elif defined(Q_OS_LINUX)
    if (!QX11Info::isPlatformX11() || QX11Info::display() == nullptr) {
        switch (state) {
        case Maximized:
            showMaximized();
            break;
        case Minimized:
            showMinimized();
            break;
        case FullScreen:
            showFullScreen();
            break;
        case Normal:
            showNormal();
            break;
        }
        return;
    }

    Display *display = QX11Info::display();
    Window window = winId();

    Atom wmState = XInternAtom(display, "_NET_WM_STATE",false);
    Atom maxHorz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ",false);
    Atom maxVert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT",false);
    Atom fullScreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN",false);

    XEvent xev;
    memset(&xev,0,sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = window;
    xev.xclient.message_type = wmState;
    xev.xclient.format = 32;

    switch (state) {
        case Maximized:
            xev.xclient.data.l[0] = 1;//_NET_WM_STATE_ADD
        xev.xclient.data.l[1] = maxHorz;
        xev.xclient.data.l[2] = maxVert;
        break;
        case Minimized:
            XIconifyWindow(display,window,0);
        return;
        case FullScreen:
            xev.xclient.data.l[0] = 1;
        xev.xclient.data.l[1] = fullScreen;
        xev.xclient.data.l[2] = 0;
        break;
        case Normal:
            //移除全屏
                xev.xclient.data.l[0] = 0;//_NET_WM_STATE_REMOVE
        xev.xclient.data.l[1] = fullScreen;
        xev.xclient.data.l[2] = 0;
        XSendEvent(display,DefaultRootWindow(display),false,SubstructureRedirectMask | SubstructureNotifyMask,&xev);
        //移除最大化
        xev.xclient.data.l[0] = 0;//_NET_WM_STATE_REMOVE
        xev.xclient.data.l[1] = maxHorz;
        xev.xclient.data.l[2] = maxVert;
        break;
    }
    XSendEvent(display,DefaultRootWindow(display),false,SubstructureRedirectMask | SubstructureNotifyMask,&xev);
    XFlush(display);
#endif
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
