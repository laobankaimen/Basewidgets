// Private QPainter helpers shared by small feedback components, not installed.
#pragma once
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Core/FeedbackTone.h>
#include <QAbstractButton>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionButton>
#include <QStyle>
namespace basewidgets { namespace feedbackPainting {
inline QColor toneColor(FeedbackTone tone)
{
    auto* t=ThemeTokens::instance();
    switch(tone) {
    case FeedbackTone::Info: return t->primary();
    case FeedbackTone::Success: return t->success();
    case FeedbackTone::Warning: return t->warning();
    case FeedbackTone::Danger: return t->danger();
    default: return t->textSecondary();
    }
}
inline QColor blend(const QColor& base,const QColor& accent,qreal amount)
{
    return QColor::fromRgbF(base.redF()*(1-amount)+accent.redF()*amount,
                           base.greenF()*(1-amount)+accent.greenF()*amount,
                           base.blueF()*(1-amount)+accent.blueF()*amount);
}
class CloseButton : public QAbstractButton {
public:
    explicit CloseButton(QWidget* parent) : QAbstractButton(parent) {
        setFocusPolicy(Qt::StrongFocus); setCursor(Qt::PointingHandCursor);
        setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    }
    QSize sizeHint() const override { const int side=qMax(24,fontMetrics().height()+8); return QSize(side,side); }
protected:
    void paintEvent(QPaintEvent*) override {
        auto* t=ThemeTokens::instance(); QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        const QRectF area=QRectF(rect()).adjusted(1,1,-1,-1);
        p.setPen(Qt::NoPen);
        if(isEnabled()&&(underMouse()||isDown())) { p.setBrush(isDown()?t->bgPressed():t->bgHover()); p.drawRoundedRect(area,4,4); }
        p.setPen(QPen(isEnabled()?t->textSecondary():t->textDisabled(),1.5,Qt::SolidLine,Qt::RoundCap));
        const QPointF c=area.center(); const qreal r=4;
        p.drawLine(c+QPointF(-r,-r),c+QPointF(r,r)); p.drawLine(c+QPointF(r,-r),c+QPointF(-r,r));
        QStyleOptionButton option; option.initFrom(this);
        if(isEnabled()&&hasFocus()&&(option.state&QStyle::State_KeyboardFocusChange)) {
            p.setBrush(Qt::NoBrush); p.setPen(QPen(t->primary(),1)); p.drawRoundedRect(area,4,4);
        }
    }
};
class ToneSymbol : public QWidget {
public:
    explicit ToneSymbol(QWidget* parent) : QWidget(parent) { setFixedSize(28,28); }
    FeedbackTone tone=FeedbackTone::Info;
protected:
    void paintEvent(QPaintEvent*) override {
        auto* t=ThemeTokens::instance(); const QColor ink=isEnabled()?toneColor(tone):t->textDisabled();
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.setPen(Qt::NoPen);
        p.setBrush(blend(t->bg1(),ink,.12)); p.drawEllipse(QRectF(rect()).adjusted(1,1,-1,-1));
        p.setPen(QPen(ink,1.7,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        if(tone==FeedbackTone::Success) { QPolygonF tick; tick<<QPointF(8,14)<<QPointF(12,18)<<QPointF(20,10); p.drawPolyline(tick); }
        else if(tone==FeedbackTone::Danger) { p.drawLine(10,10,18,18); p.drawLine(18,10,10,18); }
        else if(tone==FeedbackTone::Warning) { p.drawLine(14,8,14,15); p.drawPoint(14,20); }
        else { p.drawPoint(14,8); p.drawLine(14,13,14,20); }
    }
};
} }
