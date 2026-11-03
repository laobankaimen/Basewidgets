#include <basewidgets/Inputs/NotificationBell.h>
#include <basewidgets/Views/NotificationModel.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <QPainter>
#include <cmath>
namespace basewidgets {
NotificationBell::NotificationBell(QWidget* parent) : ToolButton(parent)
{
    setAutoRaise(true); setAppearance(ButtonAppearance::Subtle); setFocusPolicy(Qt::StrongFocus); setToolButtonStyle(Qt::ToolButtonIconOnly);
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed); setToolTip(tr("消息：暂无未读")); setAccessibleName(toolTip());
    setIconSize(QSize(20,20));
    auto refreshIcon=[this] {
        setIcon(StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/titlebar/bell.svg"));
        update();
    };
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,refreshIcon);
    refreshIcon();
}
NotificationModel* NotificationBell::model() const { return m_model; }
void NotificationBell::setModel(NotificationModel* model)
{
    if(m_model==model) { if(!model)setUnreadCount(0); return; }
    if(m_model)disconnect(m_model,nullptr,this,nullptr); m_model=model;
    if(model) {
        connect(model,&NotificationModel::unreadCountChanged,this,&NotificationBell::setUnreadCount);
        connect(model,&QObject::destroyed,this,[this] { m_model=nullptr; setUnreadCount(0); });
    }
    setUnreadCount(model?model->unreadCount():0);
}
void NotificationBell::setUnreadCount(int count)
{
    count=qMax(0,count); if(m_unread==count)return; m_unread=count;
    setToolTip(count?tr("消息：%1 条未读").arg(count):tr("消息：暂无未读")); setAccessibleName(toolTip()); update(); emit unreadCountChanged(count);
}
void NotificationBell::setDotOnly(bool enabled) { m_dotOnly=enabled; update(); }
QSize NotificationBell::sizeHint() const { const int side=qMax(36,fontMetrics().height()+14); return QSize(side,side); }
void NotificationBell::paintEvent(QPaintEvent* event)
{
    ToolButton::paintEvent(event); auto* t=ThemeTokens::instance(); QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    if(!m_unread)return;
    const QString text=m_unread>99?QStringLiteral("99+"):QString::number(m_unread);
    QFont badge=font(); badge.setPixelSize(10); badge.setBold(true); p.setFont(badge);
    const int h=m_dotOnly?8:14,w=m_dotOnly?8:qMax(14,p.fontMetrics().horizontalAdvance(text)+6);
    QRectF area(width()-w-2,2,w,h); if(layoutDirection()==Qt::RightToLeft)area.moveLeft(2);
    // Use a readable text/background pair; muted when the entry is disabled.
    p.setBrush(isEnabled()?t->danger():t->textDisabled()); p.setPen(Qt::NoPen); p.drawRoundedRect(area,h/2.,h/2.);
    if(!m_dotOnly) { const QColor bg=isEnabled()?t->danger():t->textDisabled(); auto linear=[](double v) { return v<=.04045?v/12.92:std::pow((v+.055)/1.055,2.4); }; const double luminance=.2126*linear(bg.redF())+.7152*linear(bg.greenF())+.0722*linear(bg.blueF()); p.setPen(luminance>.179?Qt::black:Qt::white); p.drawText(area,Qt::AlignCenter,text); }
}
}
