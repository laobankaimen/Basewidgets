#include <basewidgets/Core/WindowPlacement.h>
#include <basewidgets/Dialogs/NotificationPopup.h>
#include <basewidgets/Containers/NotificationPanel.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QCursor>
#include <QAbstractButton>
namespace basewidgets {
NotificationPopup::NotificationPopup(QWidget* parent) : QFrame(parent,Qt::Popup|Qt::FramelessWindowHint)
{
    setObjectName("NotificationPopup"); auto* layout=new QVBoxLayout(this); layout->setContentsMargins(1,1,1,1);
    m_panel=new NotificationPanel(this); m_panel->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Ignored); layout->addWidget(m_panel);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { auto p=ThemeTokens::instance()->palette(); p.setColor(QPalette::Window,ThemeTokens::instance()->bg1()); setPalette(p); update(); });
    auto p=ThemeTokens::instance()->palette(); p.setColor(QPalette::Window,ThemeTokens::instance()->bg1()); setPalette(p);
}
void NotificationPopup::setModel(NotificationModel* model) { m_panel->setModel(model); }
void NotificationPopup::popup(QWidget* anchor)
{
    if(!anchor||!anchor->isVisible())return;
    if(m_anchor && m_anchor!=anchor)if(auto* button=qobject_cast<QAbstractButton*>(m_anchor))button->setDown(false);
    m_anchor=anchor; const QPoint below=anchor->mapToGlobal(QPoint(anchor->width(),anchor->height()+6));
    QScreen* screen=QGuiApplication::screenAt(anchor->mapToGlobal(anchor->rect().center())); if(!screen)screen=WindowPlacement::screenFor(anchor);
    const QRect available=screen?screen->availableGeometry():QRect(below-QPoint(400,0),QSize(400,520));
    const int above=anchor->mapToGlobal(QPoint()).y()-6;
    const int belowSpace=qMax(0,available.bottom()+1-below.y());
    const int aboveSpace=qMax(0,above-available.top());
    const bool useBelow=belowSpace>=520 || belowSpace>=aboveSpace;
    const int heightLimit=qMax(1,useBelow?belowSpace:aboveSpace);
    resize(QSize(400,qMin(520,heightLimit)).boundedTo(available.size()));
    int x=below.x()-width(),y=useBelow?below.y():above-height();
    x=qBound(available.left(),x,qMax(available.left(),available.right()-width()+1)); y=qBound(available.top(),y,qMax(available.top(),available.bottom()-height()+1));
    move(x,y); m_panel->showList(); show(); if(auto* button=qobject_cast<QAbstractButton*>(anchor))button->setDown(true); m_panel->setFocus(Qt::PopupFocusReason);
}
void NotificationPopup::mousePressEvent(QMouseEvent* event)
{
    if(!rect().contains(mapFromGlobal(event->globalPos()))) {
        // Qt otherwise replays this click to the bell and immediately reopens us.
        setAttribute(Qt::WA_NoMouseReplay);
        hide(); event->accept(); return;
    }
    QFrame::mousePressEvent(event);
}
void NotificationPopup::mouseDoubleClickEvent(QMouseEvent* event)
{
    mousePressEvent(event);
}
void NotificationPopup::hideEvent(QHideEvent* event)
{
    if(auto* button=qobject_cast<QAbstractButton*>(m_anchor)) {
        button->setDown(false);
        button->setAttribute(Qt::WA_UnderMouse,button->rect().contains(button->mapFromGlobal(QCursor::pos())));
        button->update();
    }
    QFrame::hideEvent(event);
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
