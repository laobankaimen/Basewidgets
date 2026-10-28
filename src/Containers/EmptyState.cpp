#include <basewidgets/Containers/EmptyState.h>
#include <basewidgets/Inputs/PushButton.h>
#include "../Widgets/FeedbackPainting_p.h"
#include <QLabel>
#include <QVBoxLayout>

namespace basewidgets {
namespace {
class EmptyArtwork : public QWidget {
public:
    explicit EmptyArtwork(QWidget* parent) : QWidget(parent) { setFixedSize(88,72); }
    QIcon icon;
protected:
    void paintEvent(QPaintEvent*) override {
        auto* t=ThemeTokens::instance(); QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        if(!icon.isNull()) { icon.paint(&p,rect().adjusted(16,8,-16,-8),Qt::AlignCenter,isEnabled()?QIcon::Normal:QIcon::Disabled); return; }
        const QColor accent=isEnabled()?t->primary():t->textDisabled();
        p.setPen(Qt::NoPen); p.setBrush(feedbackPainting::blend(t->bg1(),accent,.1)); p.drawEllipse(QRectF(13,3,62,62));
        p.setPen(QPen(isEnabled()?t->textSecondary():t->textDisabled(),1.6,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        p.setBrush(t->bg1()); p.drawRoundedRect(QRectF(28,14,32,37),3,3);
        p.setPen(QPen(t->border(),1.4,Qt::SolidLine,Qt::RoundCap)); p.drawLine(35,24,53,24); p.drawLine(35,30,49,30);
        QPainterPath tray; tray.moveTo(17,40); tray.lineTo(33,40); tray.lineTo(37,46); tray.lineTo(51,46);
        tray.lineTo(55,40); tray.lineTo(71,40); tray.lineTo(67,61); tray.lineTo(21,61); tray.closeSubpath();
        p.setPen(QPen(accent,1.6,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)); p.setBrush(t->bg1()); p.drawPath(tray);
    }
};
}
EmptyState::EmptyState(QWidget* parent) : StyledWidget(parent),m_title(new QLabel(this)),m_description(new QLabel(this)),
    m_art(new EmptyArtwork(this)),m_action(new PushButton(this)),m_secondary(new PushButton(this))
{
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    auto* layout=new QVBoxLayout(this); layout->setContentsMargins(20,20,20,20); layout->setSpacing(10);
    layout->addStretch(); layout->addWidget(m_art,0,Qt::AlignHCenter);
    for(auto* label:{m_title,m_description}) {
        label->setTextFormat(Qt::PlainText); label->setWordWrap(true); label->setAlignment(Qt::AlignCenter);
        label->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred); layout->addWidget(label);
    }
    QFont heading; heading.setBold(true); m_title->setFont(heading);
    m_action->setObjectName("EmptyStateAction"); m_action->setAppearance(ButtonAppearance::Primary);
    m_secondary->setObjectName("EmptyStateSecondaryAction"); m_secondary->setAppearance(ButtonAppearance::Subtle);
    layout->addWidget(m_action,0,Qt::AlignHCenter); layout->addWidget(m_secondary,0,Qt::AlignHCenter); layout->addStretch();
    m_action->hide(); m_secondary->hide(); m_description->hide(); setTitle(tr("暂无数据"));
    connect(m_action,&QPushButton::clicked,this,&EmptyState::actionRequested);
    connect(m_secondary,&QPushButton::clicked,this,&EmptyState::secondaryActionRequested);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,&EmptyState::updateColors); updateColors();
}
void EmptyState::setTitle(const QString& text) { m_title->setText(text); m_title->setVisible(!text.isEmpty()); setAccessibleName(text); updateGeometry(); }
QString EmptyState::title() const { return m_title->text(); }
void EmptyState::setDescription(const QString& text) { m_description->setText(text); m_description->setVisible(!text.isEmpty()); setAccessibleDescription(text); updateGeometry(); }
QString EmptyState::description() const { return m_description->text(); }
void EmptyState::setIcon(const QIcon& icon) { static_cast<EmptyArtwork*>(m_art)->icon=icon; m_art->update(); }
QIcon EmptyState::icon() const { return static_cast<EmptyArtwork*>(m_art)->icon; }
void EmptyState::setActionText(const QString& text) { m_action->setText(text); m_action->setVisible(!text.isEmpty()); updateGeometry(); }
QString EmptyState::actionText() const { return m_action->text(); }
void EmptyState::setSecondaryActionText(const QString& text) { m_secondary->setText(text); m_secondary->setVisible(!text.isEmpty()); updateGeometry(); }
QString EmptyState::secondaryActionText() const { return m_secondary->text(); }
void EmptyState::updateColors()
{
    auto* t=ThemeTokens::instance(); auto pal=m_title->palette();
    pal.setColor(QPalette::WindowText,t->textPrimary()); pal.setColor(QPalette::Disabled,QPalette::WindowText,t->textDisabled()); m_title->setPalette(pal);
    pal=m_description->palette(); pal.setColor(QPalette::WindowText,t->textSecondary()); pal.setColor(QPalette::Disabled,QPalette::WindowText,t->textDisabled()); m_description->setPalette(pal);
    m_art->update(); update();
}
}
