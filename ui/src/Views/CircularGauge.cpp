#include <QtAdvancedUI/Views/CircularGauge.h>
#include <QPainter>
#include <QtMath>

namespace QtAdvancedUI {

CircularGauge::CircularGauge(QWidget* parent)
    : StyledWidget(parent)
{
    setMinimumSize(160, 160);
}

CircularGauge::CircularGauge(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : StyledWidget(manager, parent)
{
    setMinimumSize(160, 160);
}

CircularGauge::~CircularGauge() = default;

void CircularGauge::setRange(double min, double max)
{
    m_min = min; m_max = max; update();
}

void CircularGauge::setValue(double v)
{
    m_value = qBound(m_min, v, m_max);
    update();
}

void CircularGauge::setStartAngle(double deg) { m_startDeg = deg; update(); }
void CircularGauge::setEndAngle(double deg)   { m_endDeg = deg;   update(); }
void CircularGauge::setTickCount(int count)   { m_tickCount = qMax(1, count); update(); }
void CircularGauge::setUnit(const QString& unit) { m_unit = unit; update(); }
void CircularGauge::setTitle(const QString& title) { m_title = title; update(); }

static qreal toRadians(qreal deg) { return qDegreesToRadians(deg); }

void CircularGauge::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();
    const int size = qMin(w, h);
    const QPointF c(w/2.0, h/2.0);
    const qreal outerR = size/2.0 - 6;
    const qreal tickOuterR = outerR;
    const qreal tickInnerRMajor = outerR - 12;
    const qreal tickInnerRMinor = outerR - 8;
    const qreal needleLen = outerR - 22;

    // 背景圆盘（更柔和）
    p.setPen(QPen(m_dialColor, 4));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(c, outerR, outerR);

    // 计算角度
    const qreal spanDeg = m_startDeg - m_endDeg; // e.g. 270
    const qreal valueRatio = (m_value - m_min) / (m_max - m_min + 0.0001);
    const qreal valueDeg = m_startDeg - spanDeg * valueRatio;

    // 值弧线（绿色渐变更显眼）
    QRectF arcRect(c.x()-outerR, c.y()-outerR, outerR*2, outerR*2);
    QPen valPen(QColor(76, 217, 100), 8, Qt::SolidLine, Qt::FlatCap);
    p.setPen(valPen);
    p.drawArc(arcRect, int(m_startDeg*16), int((valueDeg - m_startDeg)*16));

    // 刻度（主刻度更长更粗，次刻度更密更短）
    const int majorCount = qMax(2, m_tickCount);
    const int minorPerMajor = 4; // 每个主刻度之间画 4 个次刻度
    for (int i=0; i<=majorCount; ++i) {
        qreal tMajor = i / qreal(majorCount);
        qreal degMajor = m_startDeg - spanDeg * tMajor;
        qreal radMajor = toRadians(degMajor);
        QPointF m1 = c + QPointF(qCos(radMajor)*tickInnerRMajor, qSin(radMajor)*tickInnerRMajor);
        QPointF m2 = c + QPointF(qCos(radMajor)*tickOuterR,      qSin(radMajor)*tickOuterR);
        p.setPen(QPen(m_tickColor, 3));
        p.drawLine(m1, m2);

        if (i < majorCount) {
            for (int j=1; j<minorPerMajor; ++j) {
                qreal tMinor = (i + j/qreal(minorPerMajor)) / qreal(majorCount);
                qreal degMinor = m_startDeg - spanDeg * tMinor;
                qreal radMinor = toRadians(degMinor);
                QPointF s1 = c + QPointF(qCos(radMinor)*tickInnerRMinor, qSin(radMinor)*tickInnerRMinor);
                QPointF s2 = c + QPointF(qCos(radMinor)*tickOuterR,      qSin(radMinor)*tickOuterR);
                p.setPen(QPen(m_tickColor, 2));
                p.drawLine(s1, s2);
            }
        }
    }

    // 指针
    qreal radVal = toRadians(valueDeg);
    QPointF tip = c + QPointF(qCos(radVal)*needleLen, qSin(radVal)*needleLen);
    p.setPen(QPen(m_needleColor, 3));
    p.drawLine(c, tip);
    p.setBrush(m_needleColor);
    p.setPen(Qt::NoPen);
    p.drawEllipse(c, 4.5, 4.5);

    // 文本（标题与数值）
    p.setPen(QPen(QColor(220,220,220)));
    QFont f = font();
    // 如果 pointSizeF 不可用（可能为 -1），给一个安全默认值
    qreal base = f.pointSizeF(); if (base <= 0) base = 14.0;
    f.setPointSizeF(base);
    f.setBold(true);
    p.setFont(f);
    QString valText = QString::number(m_value, 'f', 0) + (m_unit.isEmpty()?"":" "+m_unit);
    p.drawText(QRectF(c.x()-outerR, c.y()-20, outerR*2, 40), Qt::AlignCenter, valText);
    if (!m_title.isEmpty()) {
        QFont f2 = font();
        qreal tbase = f2.pointSizeF(); if (tbase <= 0) tbase = 12.0; // 避免 <=0 警告
        f2.setPointSizeF(qMax(10.0, tbase - 1.0));
        p.setFont(f2);
        p.drawText(QRectF(c.x()-outerR, c.y()+outerR-28, outerR*2, 24), Qt::AlignCenter, m_title);
    }
}

void CircularGauge::applyStylesheet()
{
    if (styleManager()) setStyleSheet(styleManager()->styleSheet());
}

} // namespace QtAdvancedUI