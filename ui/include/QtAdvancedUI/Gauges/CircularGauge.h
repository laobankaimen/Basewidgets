/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef QTADVANCEDUI_CIRCULARGAUGE_H
#define QTADVANCEDUI_CIRCULARGAUGE_H

#include <QtAdvancedUI/Core/StyledWidget.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {
//角度坐标约定
// - 0° 指向右（3 点钟方向）， 90° 向上（12 点）， 180° 向左（9 点）， 270° 向下（6 点）。
// - 代码内部按顺时针增加值进行绘制，所以“值从最小到最大”会从 startAngle 顺时针扫到 endAngle
// 圆形刻度与指针仪表盘
class ACSS_UI_EXPORT CircularGauge : public StyledWidget {
    Q_OBJECT
public:
    explicit CircularGauge(QWidget* parent = nullptr);
    explicit CircularGauge(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~CircularGauge() override;

    struct GaugeRange {
        double start = 0.0; // 区间起始值
        double end = 0.0;   // 区间结束值
        QColor color = QColor(200,200,200);
        int width = 10;     // 该区间环形粗细
    };

    void setRange(double min, double max);
    void setValue(double v);
    double value() const { return m_value; }

    void setStartAngle(double deg); // 默认 225 度
    void setEndAngle(double deg);   // 默认 -45 度
    void setTickCount(int count);   // 默认 10
    void setUnit(const QString& unit);
    void setTitle(const QString& title);

    void setDialColor(const QColor& c) { m_dialColor = c; }
    void setNeedleColor(const QColor& c) { m_needleColor = c; }
    void setTickColor(const QColor& c) { m_tickColor = c; }

    // 优化能力：分段彩色环、刻度标签、点状次刻度、环厚度
    void setRanges(const QVector<GaugeRange>& ranges); // 设置彩色区间
    void setShowTickLabels(bool on);                   // 显示主刻度数值
    void setMinorTicksAsDots(bool on);                 // 次刻度以点显示
    void setMinorTickDotAtInner(bool on);              // 点显示在内圈（true）或外圈
    void setRingWidth(int w);                          // 基础环厚度
    void setBaseRingColor(const QColor& c) { m_baseRingColor = c; }
    void setValueArcColor(const QColor& c) { m_valueArcColor = c; }
    void setLabelPrecision(int p) { m_labelPrecision = qMax(0, p); }
    void setTickLabelOffset(int px) { m_tickLabelOffset = px; }
    void setShowValueArc(bool on);                      // 是否显示当前值高亮弧线
    void setNeedleWidth(int w);                        // 三角指针宽度
    void setNeedleWrapRing(bool on);                   // 指针大端贴住圆环内侧
    void setTickInset(int px);                         // 刻度靠近圆环内侧的间隙（0=紧贴）
    void setShowLabelInValue(bool on);                 // 下行数值是否包含标题（温度 11℃）
    void setTickLabelFontScale(double s);              // 刻度值字体缩放（默认 1.0）

    // 刻度详细配置
    void setMajorTickStep(double step);                // 主刻度步长（如 10）
    void setMinorTicksPerMajor(int count);             // 每个主刻度之间次刻度数量
    void setTickMajorWidth(int w);                     // 主刻度线宽
    void setTickMinorWidth(int w);                     // 次刻度线宽
    void setTickMajorLength(int len);                  // 主刻度长度
    void setTickMinorLength(int len);                  // 次刻度长度
    void setMinorTickAngleRange(double startDeg, double endDeg); // 限制次刻度显示角度范围（可选）

    // 预设样式：最小化demo代码，直接应用参考风格
    // 完成率（深色仪表盘，0-100，分段彩色环，线型次刻度，楔形指针）
    void useCompletionRatePreset(const QString& title = QString(), const QString& unit = QString("%"));

    // 经典三色百分比预设（参考 GaugeSpeed 绘制逻辑）
    void useClassicThreeColorPercentPreset(const QString& title = QString(), const QString& unit = QString("%"),
                                           int percentStart = 30, int percentMid = 50, int percentEnd = 20,
                                           const QColor& colorStart = QColor(162, 208, 190),
                                           const QColor& colorMid   = QColor(103, 133, 160),
                                           const QColor& colorEnd   = QColor(203, 80, 80));

protected:
    void paintEvent(QPaintEvent* e) override;
    void applyStylesheet();

private:
    double m_min = 0.0;
    double m_max = 100.0;
    double m_value = 0.0;
    double m_startDeg = 225.0;
    double m_endDeg = -45.0;
    int m_tickCount = 10;
    QString m_unit;
    QString m_title;

    QColor m_dialColor = QColor(60,60,60);
    QColor m_needleColor = QColor(220, 0, 0);
    QColor m_tickColor = QColor(160,160,160);

    // 新增配置
    QVector<GaugeRange> m_ranges;
    bool m_showTickLabels = true;
    bool m_minorTicksAsDots = true;
    bool m_minorDotAtInner = true;
    int m_ringWidth = 12;
    QColor m_baseRingColor = QColor(210,210,210);
    QColor m_valueArcColor = QColor(76, 217, 100);
    int m_labelPrecision = 0;
    int m_tickLabelOffset = 12;
    bool m_showValueArc = false;
    int m_needleWidth = 10;
    bool m_needleWrapRing = true;
    int m_tickInset = 2;
    bool m_showLabelInValue = true;
    double m_tickLabelFontScale = 1.0;

    // 刻度样式
    double m_majorStep = 0.0;      // 0 表示按 m_tickCount 计算
    int m_minorPerMajor = 4;
    int m_tickMajorWidth = 3;
    int m_tickMinorWidth = 2;
    int m_tickMajorLength = 12;
    int m_tickMinorLength = 8;
    double m_minorStartDeg = std::numeric_limits<double>::quiet_NaN();
    double m_minorEndDeg   = std::numeric_limits<double>::quiet_NaN();
    // 经典三色百分比模式参数
    bool m_useClassicThreeColor = false;
    int m_percentStart = 30;
    int m_percentMid   = 50;
    int m_percentEnd   = 20; // 三段之和约等于 100
    QColor m_colorStart = QColor(162, 208, 190);
    QColor m_colorMid   = QColor(103, 133, 160);
    QColor m_colorEnd   = QColor(203, 80, 80);
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_CIRCULARGAUGE_H

