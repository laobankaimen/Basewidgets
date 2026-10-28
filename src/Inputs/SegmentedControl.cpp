#include <basewidgets/Inputs/SegmentedControl.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QPainter>
#include <QStyleOptionTab>
#include <QStyle>

namespace basewidgets {
SegmentedControl::SegmentedControl(QWidget* parent) : QTabBar(parent)
{
    StyleManagerAdapter::instance();
    setShape(QTabBar::RoundedNorth); setDocumentMode(true); setDrawBase(false);
    setExpanding(true); setElideMode(Qt::ElideRight); setUsesScrollButtons(true);
    setFocusPolicy(Qt::StrongFocus); setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); });
}
QSize SegmentedControl::tabSizeHint(int index) const
{
    const QSize text=fontMetrics().size(Qt::TextShowMnemonic,tabText(index));
    const bool icon=!tabIcon(index).isNull();
    return QSize(text.width()+32+(icon?iconSize().width()+6:0),qMax(36,qMax(text.height(),icon?iconSize().height():0)+16));
}
QSize SegmentedControl::minimumTabSizeHint(int index) const
{
    return QSize(48+(!tabIcon(index).isNull()?iconSize().width():0),tabSizeHint(index).height());
}
void SegmentedControl::paintEvent(QPaintEvent*)
{
    auto* t=ThemeTokens::instance(); QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen); p.setBrush(t->bg2()); p.drawRoundedRect(QRectF(rect()).adjusted(.5,.5,-.5,-.5),7,7);
    for(int i=0;i<count();++i) {
        QStyleOptionTab option; initStyleOption(&option,i);
        const bool enabled=isEnabled()&&isTabEnabled(i), selected=i==currentIndex();
        const QRect area=tabRect(i).adjusted(3,3,-3,-3);
        if(!area.intersects(rect()))continue;
        if(selected) {
            p.setPen(QPen(enabled?t->borderLight():t->bgCtrl(),1)); p.setBrush(t->bg1());
            p.drawRoundedRect(QRectF(area).adjusted(.5,.5,-.5,-.5),5,5);
        }
        QRect label=area.adjusted(10,0,-10,0);
        if(!tabIcon(i).isNull()) {
            const QSize size=iconSize().boundedTo(QSize(qMax(0,label.width()),qMax(0,label.height())));
            QRect iconRect(label.left(),label.center().y()-size.height()/2,size.width(),size.height());
            tabIcon(i).paint(&p,QStyle::visualRect(layoutDirection(),label,iconRect),Qt::AlignCenter,enabled?QIcon::Normal:QIcon::Disabled);
            label=QStyle::visualRect(layoutDirection(),label,label.adjusted(size.width()+6,0,0,0));
        }
        p.setPen(enabled?(selected?t->textPrimary():t->textSecondary()):t->textDisabled());
        p.drawText(label,Qt::AlignCenter|Qt::TextSingleLine|Qt::TextShowMnemonic,
                   fontMetrics().elidedText(tabText(i),Qt::ElideRight,qMax(0,label.width()),Qt::TextShowMnemonic));
        if(enabled&&selected&&hasFocus()&&(option.state&QStyle::State_KeyboardFocusChange)) {
            p.setBrush(Qt::NoBrush); p.setPen(QPen(t->primary(),1));
            p.drawRoundedRect(QRectF(area).adjusted(1,1,-1,-1),4,4);
        }
    }
}
}
