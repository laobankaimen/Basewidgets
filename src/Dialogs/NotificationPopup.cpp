#include <basewidgets/Dialogs/NotificationPopup.h>
#include <basewidgets/Containers/NotificationPanel.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPainter>
namespace basewidgets {
NotificationPopup::NotificationPopup(QWidget* parent) : QFrame(parent,Qt::Popup|Qt::FramelessWindowHint)
{
    setObjectName("NotificationPopup"); auto* layout=new QVBoxLayout(this); layout->setContentsMargins(1,1,1,1);
    m_panel=new NotificationPanel(this); layout->addWidget(m_panel);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { auto p=ThemeTokens::instance()->palette(); p.setColor(QPalette::Window,ThemeTokens::instance()->bg1()); setPalette(p); update(); });
    auto p=ThemeTokens::instance()->palette(); p.setColor(QPalette::Window,ThemeTokens::instance()->bg1()); setPalette(p);
}
void NotificationPopup::setModel(NotificationModel* model) { m_panel->setModel(model); }
void NotificationPopup::popup(QWidget* anchor)
{
    if(!anchor||!anchor->isVisible())return;
    m_anchor=anchor; const QPoint below=anchor->mapToGlobal(QPoint(anchor->width(),anchor->height()+6));
    QScreen* screen=QGuiApplication::screenAt(anchor->mapToGlobal(anchor->rect().center())); if(!screen)screen=anchor->screen();
    const QRect available=screen?screen->availableGeometry():QRect(below-QPoint(400,0),QSize(400,520));
    resize(QSize(400,520).boundedTo(available.size()));
    int x=below.x()-width(),y=below.y();
    if(y+height()>available.bottom()+1)y=anchor->mapToGlobal(QPoint()).y()-height()-6;
    x=qBound(available.left(),x,qMax(available.left(),available.right()-width()+1)); y=qBound(available.top(),y,qMax(available.top(),available.bottom()-height()+1));
    move(x,y); m_panel->showList(); show(); m_panel->setFocus(Qt::PopupFocusReason);
}
void NotificationPopup::paintEvent(QPaintEvent*)
{
    QPainter p(this); p.fillRect(rect(),ThemeTokens::instance()->bg1()); p.setBrush(Qt::NoBrush); p.setPen(ThemeTokens::instance()->border()); p.drawRect(rect().adjusted(0,0,-1,-1));
}
void NotificationPopup::keyPressEvent(QKeyEvent* event)
{
    if(event->key()==Qt::Key_Escape) { hide(); if(m_anchor&&m_anchor->isVisible())m_anchor->setFocus(Qt::PopupFocusReason); event->accept(); return; }
    QFrame::keyPressEvent(event);
}
}
