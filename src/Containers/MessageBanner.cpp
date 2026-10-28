#include <basewidgets/Containers/MessageBanner.h>
#include <basewidgets/Inputs/PushButton.h>
#include "../Widgets/FeedbackPainting_p.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QEvent>

namespace basewidgets {
MessageBanner::MessageBanner(QWidget* parent) : StyledWidget(parent),
    m_title(new QLabel(this)),m_message(new QLabel(this)),m_symbol(new feedbackPainting::ToneSymbol(this)),
    m_action(new PushButton(this)),m_close(new feedbackPainting::CloseButton(this))
{
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    m_title->setTextFormat(Qt::PlainText); m_message->setTextFormat(Qt::PlainText);
    m_title->setWordWrap(true); m_message->setWordWrap(true);
    m_title->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
    m_message->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
    QFont heading; heading.setBold(true); m_title->setFont(heading);
    m_action->setObjectName("MessageBannerAction"); m_action->setAppearance(ButtonAppearance::Subtle); m_action->hide();
    m_close->setObjectName("MessageBannerClose"); m_close->setAccessibleName(tr("关闭提示")); m_close->setToolTip(tr("关闭提示"));
    auto* row=new QHBoxLayout(this); row->setContentsMargins(14,12,12,12); row->setSpacing(10);
    row->addWidget(m_symbol,0,Qt::AlignTop);
    auto* text=new QVBoxLayout; text->setSpacing(5); text->setContentsMargins(0,0,0,0);
    text->addWidget(m_title); text->addWidget(m_message); text->addWidget(m_action,0,Qt::AlignLeading);
    row->addLayout(text,1); row->addWidget(m_close,0,Qt::AlignTop);
    m_title->hide(); m_message->hide();
    connect(m_close,&QAbstractButton::clicked,this,&MessageBanner::dismiss);
    connect(m_action,&QPushButton::clicked,this,&MessageBanner::actionRequested);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,&MessageBanner::updateColors);
    updateColors();
}
void MessageBanner::setTitle(const QString& text) { m_title->setText(text); m_title->setVisible(!text.isEmpty()); setAccessibleName(text); updateGeometry(); }
QString MessageBanner::title() const { return m_title->text(); }
void MessageBanner::setMessage(const QString& text) { m_message->setText(text); m_message->setVisible(!text.isEmpty()); setAccessibleDescription(text); updateGeometry(); }
QString MessageBanner::message() const { return m_message->text(); }
void MessageBanner::setTone(FeedbackTone tone) { m_tone=tone; static_cast<feedbackPainting::ToneSymbol*>(m_symbol)->tone=tone; updateColors(); }
void MessageBanner::setActionText(const QString& text) { m_action->setText(text); m_action->setVisible(!text.isEmpty()); updateGeometry(); }
QString MessageBanner::actionText() const { return m_action->text(); }
void MessageBanner::setDismissible(bool enabled) { m_dismissible=enabled; m_close->setVisible(enabled); updateGeometry(); }
void MessageBanner::dismiss() { if(!m_dismissible||isHidden())return; hide(); emit dismissed(); }
void MessageBanner::updateColors()
{
    auto* t=ThemeTokens::instance(); auto pal=m_title->palette();
    pal.setColor(QPalette::WindowText,t->textPrimary()); pal.setColor(QPalette::Disabled,QPalette::WindowText,t->textDisabled()); m_title->setPalette(pal);
    pal=m_message->palette(); pal.setColor(QPalette::WindowText,t->textSecondary()); pal.setColor(QPalette::Disabled,QPalette::WindowText,t->textDisabled()); m_message->setPalette(pal);
    m_symbol->update(); m_close->update(); update();
}
void MessageBanner::changeEvent(QEvent* event) { StyledWidget::changeEvent(event); if(event->type()==QEvent::EnabledChange)updateColors(); }
void MessageBanner::paintEvent(QPaintEvent*)
{
    auto* t=ThemeTokens::instance(); const QColor accent=isEnabled()?feedbackPainting::toneColor(m_tone):t->textDisabled();
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(feedbackPainting::blend(t->bg1(),accent,.05)); p.setPen(QPen(t->borderLight(),1));
    p.drawRoundedRect(QRectF(rect()).adjusted(.5,.5,-.5,-.5),7,7);
    p.setPen(QPen(accent,3,Qt::SolidLine,Qt::RoundCap));
    const int x=layoutDirection()==Qt::RightToLeft?width()-2:2;
    p.drawLine(x,10,x,qMax(10,height()-10));
}
}
