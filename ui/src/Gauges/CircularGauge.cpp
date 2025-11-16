#include <QtAdvancedUI/Gauges/CircularGauge.h>
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

void CircularGauge::setRanges(const QVector<GaugeRange>& ranges)
{
    m_ranges = ranges; update();
}

void CircularGauge::setShowTickLabels(bool on) { m_showTickLabels = on; update(); }
void CircularGauge::setMinorTicksAsDots(bool on) { m_minorTicksAsDots = on; update(); }
void CircularGauge::setMinorTickDotAtInner(bool on) { m_minorDotAtInner = on; update(); }
void CircularGauge::setRingWidth(int w) { m_ringWidth = qMax(2, w); update(); }
void CircularGauge::setShowValueArc(bool on) { m_showValueArc = on; update(); }
void CircularGauge::setNeedleWidth(int w) { m_needleWidth = qMax(2, w); update(); }
void CircularGauge::setNeedleWrapRing(bool on) { m_needleWrapRing = on; update(); }
void CircularGauge::setTickInset(int px) { m_tickInset = qMax(0, px); update(); }
void CircularGauge::setShowLabelInValue(bool on) { m_showLabelInValue = on; update(); }
void CircularGauge::setTickLabelFontScale(double s) { m_tickLabelFontScale = qMax(0.5, s); update(); }

void CircularGauge::setMajorTickStep(double step) { m_majorStep = qMax(0.0, step); update(); }
void CircularGauge::setMinorTicksPerMajor(int count) { m_minorPerMajor = qMax(0, count); update(); }
void CircularGauge::setTickMajorWidth(int w) { m_tickMajorWidth = qMax(1, w); update(); }
void CircularGauge::setTickMinorWidth(int w) { m_tickMinorWidth = qMax(1, w); update(); }
void CircularGauge::setTickMajorLength(int len) { m_tickMajorLength = qMax(1, len); update(); }
void CircularGauge::setTickMinorLength(int len) { m_tickMinorLength = qMax(1, len); update(); }
void CircularGauge::setMinorTickAngleRange(double startDeg, double endDeg) { m_minorStartDeg = startDeg; m_minorEndDeg = endDeg; update(); }

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
    // 暗色表盘背景（径向渐变），作为基础底层，提高对比度
    {
        QRadialGradient g(c, outerR, c + QPointF(outerR*0.35, outerR*0.35));
        QColor base = m_dialColor.isValid() ? m_dialColor : QColor(60,60,60);
        QColor light = base.lighter(120);
        QColor dark  = base.darker(140);
        g.setColorAt(0.0, light);
        g.setColorAt(0.6, base);
        g.setColorAt(1.0, dark);
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        QRectF dialRect(c.x()-outerR+1, c.y()-outerR+1, outerR*2-2, outerR*2-2);
        p.drawEllipse(dialRect);
    }
    const qreal tickOuterR = outerR - m_ringWidth + m_tickInset;
    const qreal tickInnerRMajor = outerR - m_tickMajorLength;
    const qreal tickInnerRMinor = outerR - m_tickMinorLength;
    const qreal needleLen = outerR - 22;

    // 基础量程角度
    const qreal spanDeg = m_startDeg - m_endDeg; // e.g. 240
    QRectF arcRect(c.x()-outerR, c.y()-outerR, outerR*2, outerR*2);
    // 边缘阴影：模拟 SpeedGauge 外圈的暗亮过渡，提升立体感
    {
        // 外侧阴影（深色半透明，线宽随半径缩放）
        QPen outerShadowPen(QColor(0,0,0,90));
        outerShadowPen.setWidthF(qMax<qreal>(2.0, outerR/35.0));
        outerShadowPen.setCapStyle(Qt::FlatCap);
        p.setPen(outerShadowPen);
        p.setBrush(Qt::NoBrush);
        p.drawArc(arcRect, int(m_startDeg*16), int((m_endDeg - m_startDeg)*16));

        // 内侧高光（白色半透明，略细，靠内圈）
        QPen innerHighlightPen(QColor(255,255,255,80));
        innerHighlightPen.setWidthF(qMax<qreal>(1.5, outerR/60.0));
        innerHighlightPen.setCapStyle(Qt::FlatCap);
        p.setPen(innerHighlightPen);
        // 通过缩小矩形，让高光更靠近内圈位置
        const qreal shrink = qMax<qreal>(1.0, m_ringWidth*0.35);
        QRectF innerRect = arcRect.adjusted(shrink, shrink, -shrink, -shrink);
        p.drawArc(innerRect, int(m_startDeg*16), int((m_endDeg - m_startDeg)*16));
    }
    // 如果采用经典三色百分比模式：按百分比绘制三段环；否则绘制基础环与自定义区间
    if (m_useClassicThreeColor) {
        // 完全按参考代码方式绘制三段环：使用本地坐标和 270 度参考系
        p.save();
        p.translate(c);
        int r = int(outerR);
        QPen pen; pen.setCapStyle(Qt::FlatCap); pen.setWidth(qMax(2, m_ringWidth));
        r = r - m_ringWidth;
        QRectF rect(-r, -r, r*2, r*2);
        double angleAll = spanDeg;
        double angleStart = angleAll * (double)m_percentStart / 100.0;
        double angleMid   = angleAll * (double)m_percentMid   / 100.0;
        double angleEnd   = angleAll * (double)m_percentEnd   / 100.0;
        // 第一段
        pen.setColor(isEnabled() ? m_colorStart : m_tickColor); p.setPen(pen);
        p.drawArc(rect, int((270 - m_startDeg - angleStart) * 16), int(angleStart * 16));
        // 第二段
        pen.setColor(isEnabled() ? m_colorMid : m_tickColor); p.setPen(pen);
        p.drawArc(rect, int((270 - m_startDeg - angleStart - angleMid) * 16), int(angleMid * 16));
        // 第三段
        pen.setColor(isEnabled() ? m_colorEnd : m_tickColor); p.setPen(pen);
        p.drawArc(rect, int((270 - m_startDeg - angleStart - angleMid - angleEnd) * 16), int(angleEnd * 16));
        p.restore();
    } else {
        QPen basePen(m_baseRingColor, m_ringWidth, Qt::SolidLine, Qt::FlatCap);
        p.setPen(basePen);
        p.setBrush(Qt::NoBrush);
        p.drawArc(arcRect, int(m_startDeg*16), int((m_endDeg - m_startDeg)*16));
    }

    // 计算角度
    const qreal valueRatio = (m_value - m_min) / (m_max - m_min + 0.0001);
    const qreal valueDeg = m_startDeg - spanDeg * valueRatio;

    // 非经典模式下，按 ranges 提供的区间绘制彩色环
    if (!m_useClassicThreeColor && !m_ranges.isEmpty()) {
        for (const auto& r : m_ranges) {
            double sRatio = (r.start - m_min) / (m_max - m_min + 0.0001);
            double eRatio = (r.end   - m_min) / (m_max - m_min + 0.0001);
            qreal sDeg = m_startDeg - spanDeg * sRatio;
            qreal eDeg = m_startDeg - spanDeg * eRatio;
            QPen pen(r.color, qMax(2, r.width), Qt::SolidLine, Qt::FlatCap);
            p.setPen(pen);
            p.drawArc(arcRect, int(sDeg*16), int((eDeg - sDeg)*16));
        }
    }

    // 值弧线（当前值高亮，可关闭）
    if (m_showValueArc) {
        QPen valPen(m_valueArcColor, qMax(2, m_ringWidth-2), Qt::SolidLine, Qt::FlatCap);
        p.setPen(valPen);
        p.drawArc(arcRect, int(m_startDeg*16), int((valueDeg - m_startDeg)*16));
    }

    // 刻度（经典模式：主刻度按段变色；次刻度线型并圆头）
    int majorCount = qMax(2, m_tickCount);
    // 若设置了步长，则根据步长计算主刻度数量
    if (m_majorStep > 0.0) {
        majorCount = qMax(1, int((m_max - m_min) / m_majorStep));
    }
    const int minorPerMajor = m_minorPerMajor;
    if (m_useClassicThreeColor) {
        // 采用与你提供的算法一致的“旋转+步进”绘制，并按常量比例缩放
        int steps = majorCount * minorPerMajor;
        double angleStep = spanDeg / steps;
        // 将半径常量（100/94/70/62）按外半径比例缩放
        double k = outerR / 100.0;          // 缩放系数
        double scaleRadius = 94.0 * k;      // 刻度半径
        double majorInner = (94.0 - 13.0) * k; // 主刻度内端
        double minorInner = (94.0 - 5.0) * k;  // 次刻度内端

        // 计算三段范围对应索引
        int indexStart = int(steps * (double)m_percentStart / 100.0) + 1;
        int indexMid   = int(steps * (double)m_percentMid   / 100.0) - 1;
        int indexEnd   = int(steps * (double)m_percentEnd   / 100.0) + 1;
        int index = 0;

        p.save();
        p.translate(c);
        p.rotate(m_startDeg);
        QPen pen; pen.setCapStyle(Qt::RoundCap);

        for (int i = 0; i <= steps; ++i) {
            if (i % minorPerMajor == 0) {
                // 主刻度颜色随分段变化
                QColor col = m_colorEnd;
                if (index < indexStart) col = m_colorStart;
                else if (index < (indexStart + indexMid)) col = m_colorMid;
                index++;
                if (!isEnabled()) col = m_tickColor;
                pen.setColor(col); pen.setWidthF(1.5);
                p.setPen(pen);
                p.drawLine(QPointF(0, majorInner), QPointF(0, scaleRadius));
            } else {
                pen.setColor(m_tickColor); pen.setWidthF(0.9);
                p.setPen(pen);
                p.drawLine(QPointF(0, minorInner), QPointF(0, scaleRadius));
            }
            p.rotate(angleStep);
        }
        p.restore();

        // 主刻度数字（半径 70，经比例缩放）
        if (m_showTickLabels) {
            double labelR = 68.0 * k; // 标签半径略缩小，避免覆盖圆环
            double startRad = (360 - m_startDeg - 90) * (M_PI / 180.0);
            double deltaRad = spanDeg * (M_PI / 180.0) / majorCount;
            QFont lf = font(); qreal lb = lf.pointSizeF(); if (lb <= 0) lb = 12.0; lf.setPointSizeF(lb * m_tickLabelFontScale);
            p.setFont(lf); p.setPen(QPen(m_tickColor));
            for (int i = 0; i <= majorCount; ++i) {
                double sina = qSin(startRad - i * deltaRad);
                double cosa = qCos(startRad - i * deltaRad);
                double vMajor = m_min + i * ((m_max - m_min) / majorCount);
                QString txt = QString("%1%2").arg(vMajor, 0, 'f', m_labelPrecision).arg(m_unit);
#if (QT_VERSION >= QT_VERSION_CHECK(5,11,0))
                double textWidth = p.fontMetrics().horizontalAdvance(txt);
#else
                double textWidth = p.fontMetrics().width(txt);
#endif
                double textHeight = p.fontMetrics().height();
                int x = int(c.x() + labelR * cosa - textWidth / 2.0);
                int y = int(c.y() - labelR * sina + textHeight / 4.0);
                p.drawText(x, y, txt);
            }
        }
    } else {
        // 原有余弦/正弦算法（非经典模式）
        for (int i=0; i<=majorCount; ++i) {
            qreal tMajor = i / qreal(majorCount);
            qreal degMajor = m_startDeg - spanDeg * tMajor;
            qreal radMajor = toRadians(-degMajor);
            QPointF m1 = c + QPointF(qCos(radMajor)*tickInnerRMajor, qSin(radMajor)*tickInnerRMajor);
            QPointF m2 = c + QPointF(qCos(radMajor)*tickOuterR,      qSin(radMajor)*tickOuterR);
            p.setPen(QPen(m_tickColor, m_tickMajorWidth));
            p.drawLine(m1, m2);
            if (m_showTickLabels) {
                double vMajor = m_min + tMajor * (m_max - m_min);
                QString txt = QString::number(vMajor, 'f', m_labelPrecision);
                QPointF lbl = c + QPointF(qCos(radMajor)*(tickOuterR+m_tickLabelOffset), qSin(radMajor)*(tickOuterR+m_tickLabelOffset));
                QFont lf = font(); qreal lb = lf.pointSizeF(); if (lb <= 0) lb = 10.0; lf.setPointSizeF(lb * m_tickLabelFontScale);
                p.setFont(lf); p.setPen(QPen(m_tickColor));
                qreal rectW = 28.0 * m_tickLabelFontScale; qreal rectH = 20.0 * m_tickLabelFontScale;
                p.drawText(QRectF(lbl.x()-rectW/2.0, lbl.y()-rectH/2.0, rectW, rectH), Qt::AlignCenter, txt);
            }
            if (i < majorCount) {
                for (int j=1; j<minorPerMajor; ++j) {
                    qreal tMinor = (i + j/qreal(minorPerMajor)) / qreal(majorCount);
                    qreal degMinor = m_startDeg - spanDeg * tMinor;
                    qreal radMinor = toRadians(-degMinor);
                    if (!std::isnan(m_minorStartDeg) && !std::isnan(m_minorEndDeg)) {
                        if (!(degMinor <= m_minorStartDeg && degMinor >= m_minorEndDeg)) continue;
                    }
                    QPointF s1 = c + QPointF(qCos(radMinor)*tickInnerRMinor, qSin(radMinor)*tickInnerRMinor);
                    QPointF s2 = c + QPointF(qCos(radMinor)*tickOuterR,      qSin(radMinor)*tickOuterR);
                    if (m_minorTicksAsDots) { p.setPen(Qt::NoPen); p.setBrush(m_tickColor); QPointF dotPos = m_minorDotAtInner ? s1 : s2; p.drawEllipse(dotPos, 2.2, 2.2); }
                    else { p.setPen(QPen(m_tickColor, m_tickMinorWidth)); p.drawLine(s1, s2); }
                }
            }
        }
    }

    // 玻璃质感盖层（与 SpeedGauge 的 QcGlassItem 一致）：
    // 顶部半圆线性渐变 + 次级半圆高光，覆盖完整圆形，增强“玻璃感”
    {
        // 盖层相对外圈略微内缩，避免越界
        const qreal glassInset = qMax<qreal>(m_ringWidth*0.2, 4.0);
        QRectF glassRect1 = arcRect.adjusted(glassInset, glassInset, -glassInset, -glassInset);
        QRectF glassRect2 = glassRect1;
        qreal r = qMin(glassRect1.width(), glassRect1.height())/2.0;
        glassRect2.setHeight(r/2.0);
        glassRect2.moveCenter(arcRect.center());

        p.setPen(Qt::NoPen);
        QColor clr1 = Qt::gray;  clr1.setAlphaF(0.20);
        QColor clr2 = Qt::white; clr2.setAlphaF(0.40);

        // 线性渐变方向与 QcGlassItem 保持一致（左上到右下）
        QLinearGradient g1(glassRect1.topLeft(), glassRect1.bottomRight());
        g1.setColorAt(0.10, clr1);
        g1.setColorAt(0.50, clr2);
        p.setBrush(g1);
        // 顶部半圆（完整圆形区域上的半圆光泽）
        p.drawPie(glassRect1, 0, 16*180);
        // 次级高光（更窄的半圆，增强顶部亮度层次）
        p.drawPie(glassRect2, 0, -16*180);
    }

    // 指针
    // 保持与圆环绘制一致的角度方向
    qreal radVal = toRadians(-valueDeg);
    QPointF dir(qCos(radVal), qSin(radVal));
    QPointF ortho(-dir.y(), dir.x());
    if (m_useClassicThreeColor) {
        // 经典三角指针
        double k = outerR / 100.0;
        int radiusPtr = int(62.0 * k);
        int w1 = int(5.0 * k);
        int w2 = int(8.0 * k);
        QPolygon pts; pts.setPoints(4, -w1, 0, 0, -w2, w1, 0, 0, radiusPtr);
        p.save(); p.translate(c); p.rotate(m_startDeg);
        double degRotate = spanDeg / (m_max - m_min) * (m_value - m_min);
        p.rotate(degRotate);
        p.setPen(Qt::NoPen); p.setBrush(m_needleColor);
        p.drawConvexPolygon(pts);
        p.restore();
    } else {
        // 非对称楔形
        QPointF baseCenter = m_needleWrapRing ? (c + dir * (outerR - m_ringWidth/2.0)) : c;
        QPointF tip = c + QPointF(qCos(radVal)*(outerR - m_ringWidth + 1), qSin(radVal)*(outerR - m_ringWidth + 1));
        qreal baseHalf = m_needleWidth/2.0;
        qreal tipHalf  = qMax(1.5, m_needleWidth*0.22/2.0);
        QPointF baseL = baseCenter + ortho * baseHalf;
        QPointF baseR = baseCenter - ortho * baseHalf;
        QPointF tipL  = tip + ortho * (-tipHalf);
        QPointF tipR  = tip + ortho * ( tipHalf);
        QPolygonF needle; needle << baseL << tipL << tip << tipR << baseR;
        p.setPen(Qt::NoPen); p.setBrush(m_needleColor);
        p.drawPolygon(needle);
        // 圆孔
        qreal holeR = 3.5;
        QPointF holeCenter = baseCenter + dir * 8.0;
        p.setBrush(QColor(245,245,245)); p.setPen(Qt::NoPen);
        p.drawEllipse(holeCenter, holeR, holeR);
    }

    // 文本（标题与数值）
    p.setPen(QPen(QColor(220,220,220)));
    QFont f = font();
    // 如果 pointSizeF 不可用（可能为 -1），给一个安全默认值
    qreal base = f.pointSizeF(); if (base <= 0) base = 12.0;
    // 数值（经典分支下继续缩小三分之一，并进一步下移）
    qreal valInc = m_useClassicThreeColor ? 2.0 : 2.0;
    qreal valYOffset = m_useClassicThreeColor ? 22.0 : 16.0; // 继续下移
    qreal valSize = base + valInc;
    if (m_useClassicThreeColor) valSize *= (2.0/3.0); // 缩小三分之一
    QFont valF = f; valF.setPointSizeF(valSize); valF.setBold(true);
    p.setFont(valF);
    QString valText = QString::number(m_value, 'f', 0) + (m_unit.isEmpty()?"":" "+m_unit);
    QString valueLine = m_showLabelInValue && !m_title.isEmpty() ? (m_title + " " + valText) : valText;
    p.drawText(QRectF(c.x()-outerR, c.y()+valYOffset, outerR*2, 40), Qt::AlignCenter, valueLine);
    // 标题在数值下方：根据数值下移同步下移，保持间距
    if (!m_title.isEmpty()) {
        qreal titleInc = m_useClassicThreeColor ? -2.0 : -2.0; // 经典分支下略微缩小字号
        qreal titleYOffset = m_useClassicThreeColor ? (valYOffset + 40.0) : 22.0; // 跟随数值下移
        QFont tF = f; tF.setPointSizeF(base + titleInc);
        p.setFont(tF);
        p.drawText(QRectF(c.x()-outerR, c.y()+titleYOffset, outerR*2, 24), Qt::AlignCenter, m_title);
    }
}

// 应用完成率风格预设，最小化外部设置逻辑
void CircularGauge::useCompletionRatePreset(const QString& title, const QString& unit)
{
    // 基本范围与文本
    setRange(0, 100);
    setUnit(unit.isEmpty() ? "%" : unit);
    setTitle(title);
    setShowLabelInValue(false);

    // 角度范围：在原基础上顺时针平移 30°（缺口中心约 300°）
    setStartAngle(60);
    setEndAngle(-180);

    // 刻度：主刻度 10 步长；每段 5 个次刻度（4 个间隔）
    setShowTickLabels(true);
    setMinorTicksAsDots(false);
    setTickCount(10);
    setMajorTickStep(10);
    setMinorTicksPerMajor(5);
    setTickMajorWidth(3);
    setTickMinorWidth(2);
    setTickMajorLength(16);
    setTickMinorLength(10);
    setTickInset(0);
    setTickLabelOffset(24);        // 标签在刻度线外侧，更接近示例
    setTickLabelFontScale(0.75);
    setLabelPrecision(0);

    // 环与配色
    setRingWidth(16);
    setBaseRingColor(QColor(230,235,242));
    setShowValueArc(false);

    // 分段彩色环
    setRanges({
        {0, 30, QColor(162, 208, 190), 20},   // 绿
        {30, 80, QColor(103, 133, 160), 20},  // 蓝灰
        {80, 100, QColor(203, 80, 80), 20}    // 红
    });

    // 指针形态（非对称楔形，尾端贴环中线）
    setNeedleWidth(12);
    setNeedleWrapRing(true);

    // 表盘暗色基色（用于渐变）
    setDialColor(QColor(60,60,60));
}

void CircularGauge::applyStylesheet()
{
    // 组件级样式应用已统一移除，改为应用级统一管理
}

void CircularGauge::useClassicThreeColorPercentPreset(const QString& title, const QString& unit,
                                                      int percentStart, int percentMid, int percentEnd,
                                                      const QColor& colorStart, const QColor& colorMid, const QColor& colorEnd)
{
    setRange(0, 100);
    setUnit(unit.isEmpty()? "%" : unit);
    setTitle(title);
    setShowLabelInValue(false);
    // 统一顺时针平移 30°（缺口中心约 300°）
    setStartAngle(60);
    setEndAngle(-180);
    setTickCount(10);
    setMajorTickStep(10);
    setMinorTicksPerMajor(5);
    setMinorTicksAsDots(false);
    setTickMajorWidth(1.5); // 与参考代码接近
    setTickMinorWidth(0.8);
    setTickMajorLength(13);
    setTickMinorLength(5);
    setTickInset(0);
    setTickLabelOffset(22);
    setTickLabelFontScale(0.70);
    setLabelPrecision(0);
    setRingWidth(14);
    setBaseRingColor(QColor(230,235,242));
    setShowValueArc(false);
    setDialColor(QColor(60,60,60));
    // 经典模式参数
    m_percentStart = percentStart; m_percentMid = percentMid; m_percentEnd = percentEnd;
    m_colorStart = colorStart; m_colorMid = colorMid; m_colorEnd = colorEnd;
    m_useClassicThreeColor = true;
    update();
}

} // namespace QtAdvancedUI
