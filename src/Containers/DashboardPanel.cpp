#include <basewidgets/Containers/DashboardPanel.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QLinearGradient>
namespace basewidgets {
DashboardPanel::DashboardPanel(const QString& title,QWidget* parent) : StyledWidget(parent)
{
    auto* outer=new QVBoxLayout(this); outer->setContentsMargins(16,12,16,16); outer->setSpacing(16);
    m_title=new QLabel(this); m_title->setTextFormat(Qt::PlainText); m_title->setWordWrap(true);
    QFont font=m_title->font(); font.setBold(true); m_title->setFont(font); outer->addWidget(m_title);
    m_content=new QVBoxLayout; m_content->setSpacing(10); outer->addLayout(m_content,1); setTitle(title);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); });
}
void DashboardPanel::setTitle(const QString& title) { m_title->setText(title); m_title->setVisible(!title.isEmpty()); updateGeometry(); update(); }
QString DashboardPanel::title() const { return m_title->text(); }
void DashboardPanel::paintEvent(QPaintEvent*)
{
    auto* t=ThemeTokens::instance(); QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient fill(0,0,0,height()); fill.setColorAt(0,t->bg1()); fill.setColorAt(1,t->bg0());
    p.setBrush(fill); p.setPen(QPen(t->borderLight(),1)); p.drawRoundedRect(QRectF(rect()).adjusted(.5,.5,-.5,-.5),5,5);
    if(!m_title->isHidden()) { const int y=m_title->geometry().bottom()+8; p.drawLine(1,y,width()-2,y); }
    QColor accent=t->primary(); accent.setAlpha(120); p.setPen(QPen(accent,2)); p.drawLine(16,1,48,1);
}
}
