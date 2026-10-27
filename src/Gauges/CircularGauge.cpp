#include <basewidgets/Gauges/CircularGauge.h>
#include <basewidgets/Core/ThemeTokens.h>
#include "GaugePainting_p.h"
#include <QPaintEvent>
#include <algorithm>
#include <limits>

namespace basewidgets {
using namespace gaugePainting;
CircularGauge::CircularGauge(QWidget* parent) : CircularGauge(nullptr,parent) {}
CircularGauge::CircularGauge(acss::QtAdvancedStylesheet* manager,QWidget* parent) : StyledWidget(manager,parent)
{
    setMinimumSize(minimumSizeHint());
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); });
    refreshDescription();
}
CircularGauge::~CircularGauge()=default;
bool CircularGauge::setRange(double minimum,double maximum)
{
    if (!std::isfinite(minimum) || !std::isfinite(maximum) || minimum>=maximum || !std::isfinite(maximum-minimum)) return false;
    m_min=minimum; m_max=maximum;
    const double old=m_value; m_value=qBound(m_min,m_value,m_max); m_target=qBound(m_min,m_target,m_max);
    const auto oldRanges=m_ranges; setRanges(oldRanges);
    refreshDescription(); update();
    if (old!=m_value) emit valueChanged(m_value);
    return true;
}
void CircularGauge::setValue(double value)
{
    if (!std::isfinite(value)) { setValueAvailable(false); return; }
    value=qBound(m_min,value,m_max);
    const bool changed=value!=m_value;
    m_value=value; setValueAvailable(true); refreshDescription(); update();
    if (changed) emit valueChanged(m_value);
}
void CircularGauge::setValueAvailable(bool available)
{
    if (m_available==available) return;
    m_available=available; refreshDescription(); update(); emit availabilityChanged(available);
}
QString CircularGauge::number(double value) const
{
    const double magnitude=std::abs(value);
    return locale().toString(value,(magnitude>=1e6 || (magnitude>0 && magnitude<.001)) ? 'g' : 'f',
                             (magnitude>=1e6 || (magnitude>0 && magnitude<.001)) ? qMax(3,m_precision) : m_precision);
}
QString CircularGauge::formattedValue() const { return m_available ? number(m_value) : QString::fromUtf8("—"); }
const CircularGauge::GaugeRange* CircularGauge::activeRange() const
{
    for (const auto& band:m_ranges)
        if (m_value>=band.start && (m_value<band.end || (m_value==m_max && band.end==m_max))) return &band;
    return nullptr;
}
QString CircularGauge::statusText() const
{
    if (!m_available) return tr("无数据");
    if (const auto* range=activeRange()) return range->label;
    return QString();
}
void CircularGauge::refreshDescription()
{
    setAccessibleName(m_title);
    setAccessibleDescription(QString("%1 %2; %3–%4; %5").arg(formattedValue(),m_unit,number(m_min),number(m_max),statusText()));
    setToolTip(QString("%1\n%2 %3\n%4").arg(m_title,formattedValue(),m_unit,statusText()).trimmed());
}
void CircularGauge::setDisplayMode(DisplayMode mode) { m_mode=mode; update(); }
void CircularGauge::setTitle(const QString& title) { m_title=title; refreshDescription(); update(); }
void CircularGauge::setUnit(const QString& unit) { m_unit=unit; refreshDescription(); update(); }
void CircularGauge::setLabelPrecision(int precision) { m_precision=qBound(0,precision,6); refreshDescription(); update(); }
bool CircularGauge::setRanges(const QVector<GaugeRange>& ranges)
{
    if (ranges.size()>64) return false;
    QVector<GaugeRange> normalized;
    for (auto band:ranges) {
        if (!std::isfinite(band.start) || !std::isfinite(band.end) || band.start>=band.end) return false;
        band.start=qMax(m_min,band.start); band.end=qMin(m_max,band.end);
        band.width=qBound(2,band.width,16);
        if (band.start<band.end) normalized.append(band);
    }
    std::sort(normalized.begin(),normalized.end(),[](const GaugeRange& a,const GaugeRange& b) { return a.start<b.start; });
    for (int i=1;i<normalized.size();++i) if (normalized[i].start<normalized[i-1].end) return false;
    m_ranges=normalized; refreshDescription(); update(); return true;
}
void CircularGauge::setTargetValue(double value)
{
    if (!std::isfinite(value)) { clearTargetValue(); return; }
    m_target=qBound(m_min,value,m_max); m_hasTarget=true; update();
}
void CircularGauge::clearTargetValue() { m_hasTarget=false; update(); }
bool CircularGauge::setAngles(double start,double end)
{
    if (!std::isfinite(start) || !std::isfinite(end)) return false;
    const double sweep=start-end;
    if (!std::isfinite(sweep) || sweep<30 || sweep>330) return false;
    m_start=std::fmod(start,360.); m_end=m_start-sweep; update(); return true;
}
void CircularGauge::setStartAngle(double angle) { setAngles(angle,m_end); }
void CircularGauge::setEndAngle(double angle) { setAngles(m_start,angle); }
double CircularGauge::angleForValue(double value) const
{
    if (!std::isfinite(value)) return m_start;
    const double fraction=(qBound(m_min,value,m_max)-m_min)/(m_max-m_min);
    return m_start-(m_start-m_end)*fraction;
}
void CircularGauge::setTickCount(int count) { m_intervals=qBound(1,count,20); m_majorStep=0; update(); }
void CircularGauge::setMajorTickStep(double step) { if (std::isfinite(step) && step>=0) { m_majorStep=step; update(); } }
void CircularGauge::setMinorTicksPerMajor(int count) { m_minorTicks=qBound(0,count,5); update(); }
void CircularGauge::setShowTickLabels(bool visible) { m_showTickLabels=visible; update(); }
void CircularGauge::setRingWidth(int width) { m_ringWidth=qBound(4,width,24); update(); }
void CircularGauge::setShowValueArc(bool visible) { m_showValueArc=visible; update(); }
void CircularGauge::setDialColor(const QColor& color) { m_dialColor=color; update(); }
void CircularGauge::setNeedleColor(const QColor& color) { m_needleColor=color; update(); }
void CircularGauge::setTickColor(const QColor& color) { m_tickColor=color; update(); }
void CircularGauge::setBaseRingColor(const QColor& color) { m_baseRingColor=color; update(); }
void CircularGauge::setValueArcColor(const QColor& color) { m_valueArcColor=color; update(); }

void CircularGauge::paintEvent(QPaintEvent*)
{
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
    const auto* tokens=ThemeTokens::instance();
    const QColor foreground=isEnabled() ? tokens->textPrimary() : tokens->textDisabled();
    const QColor secondary=isEnabled() ? tokens->textSecondary() : tokens->textDisabled();
    QColor accent=m_valueArcColor.isValid() ? m_valueArcColor : tokens->primary();
    if (const auto* band=activeRange()) if (band->color.isValid()) accent=band->color;
    if (!isEnabled() || !m_available) accent=tokens->textDisabled();
    const qreal diameter=qMax(1,qMin(width()-24,height()-80));
    const qreal outer=diameter/2;
    // At compact sizes dense labels/needles obscure the reading. Keep the same data
    // as an arc + number + explicit endpoints, instead of painting overlapping text.
    const bool compact=diameter<180;
    const bool needleMode=m_mode==DisplayMode::Needle && !compact;
    const QPointF center(width()/2.,36+outer);
    const QFont caption=sizedFont(font(),qBound(10.,QFontMetricsF(font()).height()*.9,16.));
    text(painter,QRectF(12,8,width()-24,22),m_title,caption,secondary);
    if (m_dialColor.isValid()) { painter.setBrush(m_dialColor); painter.setPen(Qt::NoPen); painter.drawEllipse(center,outer,outer); }
    const qreal radius=qMax(1.,outer-9);
    const qreal ringWidth=qMin(qreal(m_ringWidth),qMax(4.,radius*.12));
    const qreal bandRadius=radius;
    const qreal trackRadius=m_ranges.isEmpty() ? radius : qMax(1.,radius-12);
    const QColor track=m_baseRingColor.isValid() ? m_baseRingColor : tokens->borderLight();
    arc(painter,center,trackRadius,m_start,m_end,track,ringWidth,Qt::RoundCap);
    // Threshold context has its own outer band; the value arc never overwrites it.
    for (const auto& band:m_ranges) {
        const QColor color=isEnabled() ? (band.color.isValid() ? band.color : tokens->primary()) : tokens->border();
        arc(painter,center,bandRadius,angleForValue(band.start),angleForValue(band.end),color,qMin(qreal(band.width),qMax(2.,radius*.09)));
    }
    if (m_available && m_showValueArc && !needleMode)
        arc(painter,center,trackRadius,m_start,angleForValue(m_value),accent,ringWidth,Qt::RoundCap);
    const qreal tickRadius=trackRadius-ringWidth/2-5;
    QVector<double> ticks;
    if (m_majorStep>0) {
        const double raw=(m_max-m_min)/m_majorStep;
        if (std::isfinite(raw) && raw<=20) {
            for (int i=0;i<=int(std::floor(raw));++i) ticks.append(m_min+i*m_majorStep);
        } else {
            // Overdense/underflowing steps switch to bounded uniform intervals.
            for (int i=0;i<=m_intervals;++i) ticks.append(m_min+(m_max-m_min)*(double(i)/m_intervals));
        }
        if (ticks.isEmpty() || ticks.last()<m_max) ticks.append(m_max);
    } else for (int i=0;i<=m_intervals;++i) ticks.append(m_min+(m_max-m_min)*(double(i)/m_intervals));
    const QColor tickColor=isEnabled() && m_tickColor.isValid() ? m_tickColor : secondary;
    const QFont tickFont=sizedFont(font(),qBound(9.,diameter*.047,13.));
    const QFontMetricsF tickMetrics(tickFont);
    const qreal labelRadius=qMax(1.,tickRadius-17);
    const qreal valueWidth=qMax(40.,compact ? radius*1.5 : labelRadius*(needleMode ? .95 : 1.5));
    const qreal readingY=needleMode ? center.y()+outer*.22 : center.y()-18;
    const qreal readingHeight=needleMode ? 32 : 42;
    const QRectF readingRect(center.x()-valueWidth/2,readingY,valueWidth,readingHeight);
    const QRectF unitRect(center.x()-valueWidth/2,readingY+readingHeight-3,valueWidth,18);
    QVector<QRectF> occupied{readingRect,unitRect};
    // Prioritize endpoints, then remaining labels. Reject collisions instead of shrinking forever.
    QVector<int> labelOrder{0,ticks.size()-1};
    for (int i=1;i+1<ticks.size();++i) labelOrder.append(i);
    for (int i=0;i<ticks.size();++i) {
        const double angle=angleForValue(ticks[i]);
        painter.setPen(QPen(tickColor,1.2));
        painter.drawLine(polar(center,tickRadius,angle),polar(center,qMax(1.,tickRadius-5),angle));
        if (diameter>=180 && i+1<ticks.size()) for (int j=1;j<=m_minorTicks;++j) {
            const double value=ticks[i]+(ticks[i+1]-ticks[i])*double(j)/(m_minorTicks+1);
            const double minor=angleForValue(value);
            painter.setPen(QPen(tokens->border(),1));
            painter.drawLine(polar(center,tickRadius,minor),polar(center,qMax(1.,tickRadius-2.5),minor));
        }
    }
    bool firstLabel=false,lastLabel=false;
    if (m_showTickLabels && !compact) for (int i:labelOrder) {
        const QString label=number(ticks[i]); const QPointF position=polar(center,labelRadius,angleForValue(ticks[i]));
        const qreal w=tickMetrics.horizontalAdvance(label)+4,h=tickMetrics.height()+2;
        const QRectF box(position.x()-w/2,position.y()-h/2,w,h);
        if (!QRectF(4,32,width()-8,diameter+8).contains(box)) continue;
        bool overlaps=false; for (const QRectF& prior:occupied) if (prior.adjusted(-3,-2,3,2).intersects(box)) overlaps=true;
        if (overlaps) continue;
        occupied.append(box); text(painter,box,label,tickFont,tickColor);
        if (i==0) firstLabel=true;
        if (i==ticks.size()-1) lastLabel=true;
    }
    if (m_hasTarget) {
        const double angle=angleForValue(m_target);
        painter.setPen(QPen(foreground,2));
        painter.drawLine(polar(center,bandRadius+5,angle),polar(center,trackRadius-ringWidth/2-3,angle));
    }
    if (m_available && needleMode) {
        const qreal angle=angleForValue(m_value);
        const QPointF tip=polar(center,qMax(1.,tickRadius-10),angle);
        const QPointF side=polar(QPointF(),3,angle+90);
        painter.setPen(Qt::NoPen); painter.setBrush(isEnabled() && m_needleColor.isValid() ? m_needleColor : accent);
        painter.drawPolygon(QPolygonF(QVector<QPointF>{center+side,tip,center-side,polar(center,8,angle+180)}));
        painter.setBrush(tokens->bg1()); painter.setPen(QPen(accent,2)); painter.drawEllipse(center,4,4);
    }
    const QString reading=formattedValue();
    const QFont valueFont=fittedFont(font(),reading,qBound(18.,diameter*(needleMode ? .12 : .15),needleMode ? 32. : 42.),valueWidth);
    text(painter,readingRect,reading,valueFont,foreground);
    text(painter,unitRect,m_unit,caption,secondary);
    if (m_showTickLabels && (!firstLabel || !lastLabel)) {
        text(painter,QRectF(12,height()-64,width()-24,18),number(m_min)+" – "+number(m_max),sizedFont(font(),10),secondary);
    }
    const QString status=statusText();
    text(painter,QRectF(12,height()-42,width()-24,20),status,caption,m_available ? foreground : secondary);
    const QString target=m_hasTarget ? tr("目标 %1 %2").arg(number(m_target),m_unit) : QString();
    text(painter,QRectF(12,height()-22,width()-24,18),target,sizedFont(font(),11),secondary);
}
void CircularGauge::useCompletionRatePreset(const QString& title,const QString& unit)
{
    setRange(0,100); setRanges({}); setAngles(225,-45); setDisplayMode(DisplayMode::Arc);
    setTickCount(5); setMinorTicksPerMajor(0); setShowValueArc(true); setShowTickLabels(true);
    setRingWidth(12); setDialColor(QColor()); setBaseRingColor(QColor()); setNeedleColor(QColor());
    setTickColor(QColor()); setValueArcColor(QColor()); clearTargetValue(); setTitle(title); setUnit(unit); setLabelPrecision(0);
}
void CircularGauge::useClassicThreeColorPercentPreset(const QString& title,const QString& unit,
    int first,int middle,int last,const QColor& firstColor,const QColor& middleColor,const QColor& lastColor)
{
    if (first<0 || middle<0 || last<0 || double(first)+middle+last<=0) return;
    useCompletionRatePreset(title,unit); setDisplayMode(DisplayMode::Needle);
    const double total=double(first)+middle+last;
    const double a=100.*first/total,b=100.*(double(first)+middle)/total;
    QVector<GaugeRange> bands;
    if (first) bands.append({0,a,firstColor,6,tr("区间一")});
    if (middle) bands.append({a,b,middleColor,6,tr("区间二")});
    if (last) bands.append({b,100,lastColor,6,tr("区间三")});
    setRanges(bands);
}
}
