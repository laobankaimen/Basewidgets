#include <basewidgets/Containers/CollapsibleSection.h>
#include <basewidgets/Inputs/ToolButton.h>
#include <QVBoxLayout>
#include <QApplication>
#include <QSignalBlocker>
#include <QKeyEvent>
#include <QStylePainter>
#include <QStyleOptionToolButton>
namespace basewidgets {
namespace {
class SectionButton : public ToolButton {
public:
    using ToolButton::ToolButton;
    QSize sizeHint() const override { const auto size=ToolButton::sizeHint(); return QSize(qMin(360,size.width()),size.height()); }
    QSize minimumSizeHint() const override { return QSize(48,ToolButton::sizeHint().height()); }
protected:
    void paintEvent(QPaintEvent*) override {
        QStylePainter painter(this); QStyleOptionToolButton option; initStyleOption(&option);
        option.text=fontMetrics().elidedText(text(),Qt::ElideRight,qMax(0,width()-36));
        painter.drawComplexControl(QStyle::CC_ToolButton,option);
    }
    void keyPressEvent(QKeyEvent* event) override {
        if((event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter)&&!event->isAutoRepeat()) { event->accept(); click(); return; }
        ToolButton::keyPressEvent(event);
    }
};
}
CollapsibleSection::CollapsibleSection(const QString& title,QWidget* parent) : StyledWidget(parent)
{
    auto* outer=new QVBoxLayout(this); outer->setContentsMargins(0,0,0,0); outer->setSpacing(6);
    m_header=new SectionButton(this); m_header->setObjectName("CollapsibleSectionHeader");
    m_header->setAutoRaise(true); m_header->setAppearance(ButtonAppearance::Subtle);
    m_header->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); m_header->setCheckable(true); m_header->setChecked(true);
    m_header->setFocusPolicy(Qt::StrongFocus); m_header->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    m_content=new QWidget(this); m_layout=new QVBoxLayout(m_content); m_layout->setContentsMargins(8,0,8,6); m_layout->setSpacing(6);
    outer->addWidget(m_header); outer->addWidget(m_content); setFocusProxy(m_header); setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Maximum);
    connect(m_header,&QToolButton::toggled,this,&CollapsibleSection::setExpanded); setTitle(title); updateArrow();
}
void CollapsibleSection::setTitle(const QString& title) { m_header->setText(title); m_header->setToolTip(title); m_header->setAccessibleName(title); updateGeometry(); }
QString CollapsibleSection::title() const { return m_header->text(); }
void CollapsibleSection::setExpanded(bool expanded)
{
    if(m_expanded==expanded)return;
    if(!expanded) {
        auto* focus=QApplication::focusWidget(); if(focus&&(focus==m_content||m_content->isAncestorOf(focus)))m_header->setFocus(Qt::OtherFocusReason);
    }
    m_expanded=expanded; { const QSignalBlocker block(m_header); m_header->setChecked(expanded); }
    m_content->setVisible(expanded); updateArrow(); updateGeometry(); emit expandedChanged(expanded);
}
void CollapsibleSection::updateArrow()
{
    m_header->setArrowType(m_expanded?Qt::DownArrow:(layoutDirection()==Qt::RightToLeft?Qt::LeftArrow:Qt::RightArrow));
    m_header->setAccessibleDescription(m_expanded?tr("已展开，按空格或回车折叠"):tr("已折叠，按空格或回车展开"));
}
void CollapsibleSection::changeEvent(QEvent* event) { StyledWidget::changeEvent(event); if(m_header&&event->type()==QEvent::LayoutDirectionChange)updateArrow(); }
}
