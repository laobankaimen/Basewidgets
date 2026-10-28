#include <basewidgets/Inputs/CopyField.h>
#include <basewidgets/Inputs/LineEdit.h>
#include <basewidgets/Inputs/ToolButton.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QPointer>
#include <limits>
namespace basewidgets {
namespace {
class CopyAction : public ToolButton {
public:
    explicit CopyAction(QWidget* parent) : ToolButton(parent) {
        setAutoRaise(true); setAppearance(ButtonAppearance::Subtle); setFocusPolicy(Qt::StrongFocus);
        setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred); setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    bool copied=false;
    QSize sizeHint() const override { const int size=qMax(30,fontMetrics().height()+10); return QSize(size,size); }
protected:
    void paintEvent(QPaintEvent* event) override {
        ToolButton::paintEvent(event); QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        auto* t=ThemeTokens::instance(); p.setPen(QPen(isEnabled()?(copied?t->success():t->textSecondary()):t->textDisabled(),1.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        p.setBrush(Qt::NoBrush); const QPointF c=QRectF(rect()).center();
        if(copied) { QPolygonF tick; tick<<c+QPointF(-6,0)<<c+QPointF(-2,4)<<c+QPointF(6,-4); p.drawPolyline(tick); }
        else { p.drawRoundedRect(QRectF(c.x()-3,c.y()-4,9,11),1,1); QPainterPath rear; rear.moveTo(c+QPointF(-6,3)); rear.lineTo(c+QPointF(-6,-7)); rear.lineTo(c+QPointF(2,-7)); p.drawPath(rear); }
    }
};
}
CopyField::CopyField(const QString& text,QWidget* parent) : StyledWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    auto* row=new QHBoxLayout(this); row->setContentsMargins(0,0,0,0); row->setSpacing(4);
    m_edit=new LineEdit(this); m_edit->setReadOnly(true); m_edit->setMaxLength(std::numeric_limits<int>::max()); m_edit->setObjectName("CopyFieldText"); m_edit->setMinimumWidth(0);
    m_copy=new CopyAction(this); m_copy->setObjectName("CopyFieldButton"); row->addWidget(m_edit,1); row->addWidget(m_copy);
    setFocusProxy(m_edit); m_feedback=new QTimer(this); m_feedback->setSingleShot(true); m_feedback->setInterval(1500);
    connect(m_feedback,&QTimer::timeout,this,&CopyField::resetFeedback);
    connect(m_copy,&QToolButton::clicked,this,&CopyField::copy);
    connect(m_edit,&QLineEdit::textChanged,this,[this](const QString& text) {
        resetFeedback(); m_copy->setEnabled(!text.isEmpty()); m_edit->setToolTip(text); m_edit->setAccessibleDescription(text); emit textChanged(text);
    });
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { m_copy->update(); });
    resetFeedback(); m_copy->setEnabled(false); setText(text);
}
QString CopyField::text() const { return m_edit->text(); }
void CopyField::setText(const QString& text) { m_edit->setText(text); }
void CopyField::setPlaceholderText(const QString& text) { m_edit->setPlaceholderText(text); }
void CopyField::resetFeedback()
{
    m_feedback->stop(); static_cast<CopyAction*>(m_copy)->copied=false;
    m_copy->setToolTip(tr("复制全部内容")); m_copy->setAccessibleName(tr("复制全部内容")); m_copy->update();
}
void CopyField::copy()
{
    const QString value=text(); if(!isEnabled()||value.isEmpty())return;
    const QPointer<CopyField> guard(this);
    QApplication::clipboard()->setText(value,QClipboard::Clipboard);
    if(!guard)return;
    static_cast<CopyAction*>(m_copy)->copied=true; m_copy->setToolTip(tr("已复制")); m_copy->setAccessibleName(tr("已复制")); m_copy->update(); m_feedback->start();
    emit copied(value); // Receivers may delete the field; never touch members after emitting.
}
}
