#ifndef BASEWIDGETS_CIRCULARGAUGE_H
#define BASEWIDGETS_CIRCULARGAUGE_H

#include <basewidgets/Core/StyledWidget.h>
#include <QColor>
#include <QVector>

namespace basewidgets {
/** 通用数值仪表：Arc 突出完成量，Needle 展示量程位置；只负责显示，不解释业务告警。
 * 紧凑尺寸自动退化为数值/弧/量程摘要，隐藏会遮挡读数的指针与刻度文字。
 * 所有几何使用逻辑像素。GUI 线程调用；无内部动画/轮询计时器，数据更新时按需重绘。
 * 角度：0°=右，90°=上；数值沿顺时针方向从 startAngle 到 endAngle。
 */
class ACSS_UI_EXPORT CircularGauge : public StyledWidget {
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool valueAvailable READ valueAvailable WRITE setValueAvailable NOTIFY availabilityChanged)
public:
    enum class DisplayMode { Arc, Needle };
    Q_ENUM(DisplayMode)
    struct GaugeRange {
        double start = 0;
        double end = 0;
        QColor color;
        int width = 6; ///< 阈值带宽，限制为 2–16 逻辑像素；不覆盖当前值弧。
        QString label; ///< 阈值文字，例如“注意”；不要仅靠颜色表达状态。
    };
    explicit CircularGauge(QWidget* parent = nullptr);
    explicit CircularGauge(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~CircularGauge() override;

    /** 拒绝非有限、反向或零跨度量程；失败保持原状态。成功时夹取当前值/目标值。 */
    bool setRange(double minimum, double maximum);
    double minimum() const { return m_min; }
    double maximum() const { return m_max; }
    /** 有限输入夹取到量程；NaN/Inf 表示无数据，保留最后有限值但不显示指针/填充。 */
    void setValue(double value);
    double value() const { return m_value; }
    void setValueAvailable(bool available);
    bool valueAvailable() const { return m_available; }
    QString formattedValue() const;
    QString statusText() const;
    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const { return m_mode; }
    void setTitle(const QString& title);
    QString title() const { return m_title; }
    void setUnit(const QString& unit);
    QString unit() const { return m_unit; }
    void setLabelPrecision(int precision); ///< 数字和刻度精度，限制为 0–6。

    /** 阈值为实际量程单位，最多 64 段。排序/夹取到量程；重叠或无效输入整组拒绝。
     * 相邻区间按 [start,end) 解释，量程最大值属于末段；空列表清除阈值。
     */
    bool setRanges(const QVector<GaugeRange>& ranges);
    QVector<GaugeRange> ranges() const { return m_ranges; }
    void setTargetValue(double value); ///< 目标夹取到量程；NaN/Inf 清除目标。
    void clearTargetValue();
    bool hasTargetValue() const { return m_hasTarget; }
    double targetValue() const { return m_target; }

    /** 起止角要求顺时针扫角为 30–330°，默认 225→-45；两个角度原子设置避免中间无效状态。 */
    bool setAngles(double start, double end);
    void setStartAngle(double angle);
    void setEndAngle(double angle);
    double angleForValue(double value) const;
    void setTickCount(int intervals); ///< 主刻度区间数限制为 1–20，拥挤时自动减少文字。
    void setMajorTickStep(double step); ///< 0 恢复均分；正值使用真实步长，过密时自动稀疏。
    void setMinorTicksPerMajor(int count); ///< 每段次刻度数 0–5，0 合法且不除零。
    void setShowTickLabels(bool visible);
    void setRingWidth(int width); ///< 当前值弧宽度，限制为 4–24。
    void setShowValueArc(bool visible);
    /** QColor() 清除显式覆盖，恢复主题色。所有 setter 自动触发重绘。 */
    void setDialColor(const QColor& color);
    void setNeedleColor(const QColor& color);
    void setTickColor(const QColor& color);
    void setBaseRingColor(const QColor& color);
    void setValueArcColor(const QColor& color);
    void useCompletionRatePreset(const QString& title = QString(), const QString& unit = QString("%"));
    /** 兼容预设：非负权重归一化到 100，0 权重段省略。已去除旧玻璃高光和重复角度算法。 */
    void useClassicThreeColorPercentPreset(const QString& title = QString(), const QString& unit = QString("%"),
        int first = 30, int middle = 50, int last = 20,
        const QColor& firstColor = QColor("#67C23A"), const QColor& middleColor = QColor("#E6A23C"),
        const QColor& lastColor = QColor("#F56C6C"));
    QSize sizeHint() const override { return QSize(300,300); }
    QSize minimumSizeHint() const override { return QSize(180,210); }
signals:
    void valueChanged(double value);
    void availabilityChanged(bool available);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    void refreshDescription();
    QString number(double value) const;
    const GaugeRange* activeRange() const;
    double m_min=0, m_max=100, m_value=0, m_target=0;
    double m_start=225, m_end=-45, m_majorStep=0;
    int m_intervals=5, m_minorTicks=3, m_precision=0, m_ringWidth=10;
    bool m_available=true, m_hasTarget=false, m_showTickLabels=true, m_showValueArc=true;
    DisplayMode m_mode=DisplayMode::Arc;
    QString m_title, m_unit;
    QVector<GaugeRange> m_ranges;
    QColor m_dialColor, m_needleColor, m_tickColor, m_baseRingColor, m_valueArcColor;
};
}
#endif
