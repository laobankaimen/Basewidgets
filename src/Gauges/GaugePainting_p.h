// Private shared painter helpers. Not installed as public API.
#ifndef BASEWIDGETS_GAUGEPAINTING_P_H
#define BASEWIDGETS_GAUGEPAINTING_P_H
#include <QPainter>
#include <QFontMetricsF>
#include <QtMath>
#include <cmath>
namespace basewidgets { namespace gaugePainting {
inline QPointF polar(const QPointF& center,qreal radius,qreal degrees)
{
    const qreal radians=qDegreesToRadians(degrees);
    return center+QPointF(radius*std::cos(radians),-radius*std::sin(radians));
}
inline QFont sizedFont(const QFont& base,qreal pixels,bool bold=false)
{
    QFont font=base; font.setPixelSize(qMax(8,qRound(pixels))); font.setBold(bold); return font;
}
inline void text(QPainter& painter,const QRectF& rect,const QString& value,const QFont& font,
                 const QColor& color,Qt::Alignment alignment=Qt::AlignCenter)
{
    if (rect.width()<=0 || rect.height()<=0) return;
    painter.setFont(font); painter.setPen(color);
    const QFontMetricsF metrics(font);
    painter.drawText(rect,alignment|Qt::TextSingleLine,metrics.elidedText(value,Qt::ElideRight,qMax(0.,rect.width())));
}
inline QFont fittedFont(const QFont& base,const QString& value,qreal maximum,qreal width)
{
    QFont font=sizedFont(base,maximum,true);
    while (font.pixelSize()>12 && QFontMetricsF(font).horizontalAdvance(value)>width) font.setPixelSize(font.pixelSize()-1);
    return font;
}
inline void arc(QPainter& painter,const QPointF& center,qreal radius,qreal start,qreal end,
                const QColor& color,qreal width,Qt::PenCapStyle cap=Qt::FlatCap)
{
    if (radius<=0 || qAbs(end-start)<.001) return;
    painter.setPen(QPen(color,width,Qt::SolidLine,cap)); painter.setBrush(Qt::NoBrush);
    painter.drawArc(QRectF(center.x()-radius,center.y()-radius,radius*2,radius*2),qRound(start*16),qRound((end-start)*16));
}
} }
#endif
