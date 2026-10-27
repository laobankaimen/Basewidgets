#include <basewidgets/Core/ScreenWindow.h>
#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Containers/StatusBar.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/ToolButton.h>
namespace basewidgets {
ScreenWindow::ScreenWindow(Role role,QWidget* parent) : FramelessWidgetWindow(parent),m_role(role)
{
    // Parent may own a standalone instance, but workspace-managed secondary windows
    // have no QWidget parent: minimizing the primary must not minimize its other screens.
    setAttribute(Qt::WA_QuitOnClose,role==Role::Primary);
    setAttribute(Qt::WA_DeleteOnClose,false);
    titleBar()->setProfile(role==Role::Primary?CustomTitleBar::Profile::MainWindow:CustomTitleBar::Profile::SecondaryWindow);
    setTitle(role==Role::Primary?tr("主控工作区"):tr("子屏工作区"));
    auto* status=new StatusBar(this); status->setObjectName(role==Role::Primary?"PrimaryScreenStatusBar":"SecondaryScreenStatusBar");
    auto* badge=new Label(role==Role::Primary?tr("主控"):tr("子屏"),status);
    badge->setObjectName("ScreenRoleBadge"); badge->setMargin(6); status->addPermanentWidget(badge);
    if(role==Role::Secondary) {
        status->setSizeGripEnabled(false);
        auto* home=new ToolButton(status); home->setText(tr("返回主屏")); home->setToolButtonStyle(Qt::ToolButtonTextOnly);
        home->setAppearance(ButtonAppearance::Subtle); home->setObjectName("ReturnToPrimaryButton");
        home->setToolTip(tr("激活主控窗口，不关闭当前业务视图")); status->addPermanentWidget(home);
        connect(home,&QToolButton::clicked,this,&ScreenWindow::primaryWindowRequested);
    }
    setStatusBar(status);
}
void ScreenWindow::setContextText(const QString& text) { if(statusBar())statusBar()->showMessage(text); }
void ScreenWindow::closeEvent(QCloseEvent* event)
{
    FramelessWidgetWindow::closeEvent(event);
    if(event->isAccepted())emit closeAccepted();
}
}
