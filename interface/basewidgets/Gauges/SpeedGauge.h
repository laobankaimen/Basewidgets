/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_SPEEDGAUGE_H
#define BASEWIDGETS_SPEEDGAUGE_H

#include <basewidgets/Gauges/QcGaugeWidget.h>
#include <basewidgets/Gauges/CircularGauge.h>

namespace basewidgets {

// 速度表（0-80），参考 SpeedGauge 示例配色与刻度
class ACSS_UI_EXPORT SpeedGauge : public QcGaugeWidget {
    Q_OBJECT
public:
    explicit SpeedGauge(QWidget* parent = nullptr)
        : QcGaugeWidget(parent)
    {
        // 完全按照 test/examples/SpeedGauge/mainwindow.cpp 的组合搭建
        addBackground(99);
        auto* bkg1 = addBackground(92);
        bkg1->clearrColors();
        bkg1->addColor(0.1, Qt::black);
        bkg1->addColor(1.0, Qt::white);

        auto* bkg2 = addBackground(88);
        bkg2->clearrColors();
        bkg2->addColor(0.1, Qt::gray);
        bkg2->addColor(1.0, Qt::darkGray);

        addArc(55);
        addDegrees(65)->setValueRange(0,80);
        addColorBand(50);
        addValues(80)->setValueRange(0,80);
        addLabel(70)->setText("Km/h");
        auto* lab = addLabel(40);
        lab->setText("0");
        m_needle = addNeedle(60);
        m_needle->setLabel(lab);
        m_needle->setColor(Qt::white);
        m_needle->setValueRange(0,80);
        addBackground(7);
        addGlass(88);
    }

    // 为了兼容 widgets_demo 旧接口，提供空实现的占位方法（不改变 Qc 组合）
    Q_DECL_DEPRECATED_X("Use CircularGauge for configurable presets")
    void useClassicThreeColorPercentPreset(const QString&, const QString&, int, int, int,
                                           const QColor&, const QColor&, const QColor&) {}
    Q_DECL_DEPRECATED_X("Use CircularGauge for configurable presets")
    void useCompletionRatePreset(const QString&, const QString&) {}
    Q_DECL_DEPRECATED_X("Use CircularGauge for configurable presets")
    void setRange(int, int) {}
    Q_DECL_DEPRECATED_X("Use CircularGauge for configurable presets")
    void setShowValueArc(bool) {}
    Q_DECL_DEPRECATED_X("Use CircularGauge for configurable presets")
    void setDialColor(const QColor&) {}
    Q_DECL_DEPRECATED_X("Use CircularGauge for configurable presets")
    void setNeedleColor(const QColor&) {}
    Q_DECL_DEPRECATED_X("Use CircularGauge for configurable presets")
    void setUnit(const QString&) {}
    Q_DECL_DEPRECATED_X("Use CircularGauge for configurable presets")
    void setTickCount(int) {}
    Q_DECL_DEPRECATED_X("Use CircularGauge for configurable presets")
    void setShowTickLabels(bool) {}
    Q_DECL_DEPRECATED_X("Use CircularGauge for configurable presets")
    void setRanges(const QVector<CircularGauge::GaugeRange>&) {}
    void setValue(int v) { if (m_needle) { m_needle->setCurrentValue(v); update(); } }

private:
    QcNeedleItem* m_needle = nullptr;
};

} // namespace basewidgets

#endif // BASEWIDGETS_SPEEDGAUGE_H

