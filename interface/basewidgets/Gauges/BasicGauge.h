/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_BASICGAUGE_H
#define BASEWIDGETS_BASICGAUGE_H

#include <basewidgets/Gauges/QcGaugeWidget.h>
#include <basewidgets/Core/ThemeTokens.h>

namespace basewidgets {

// 基础速度表（0-100），参考 Basic 示例风格
class ACSS_UI_EXPORT BasicGauge : public QcGaugeWidget {
    Q_OBJECT
public:
    explicit BasicGauge(QWidget* parent = nullptr)
        : QcGaugeWidget(parent)
    {
        // 完全按照 test/examples/Basic/mainwindow.cpp 的组合搭建
        auto* arc = addArc(55);
        auto* degrees = addDegrees(65); degrees->setValueRange(0,100);
        auto* clrBand = addColorBand(50);
        clrBand->setValueRange(0,100);
        auto* values = addValues(80); values->setValueRange(0,100);
        auto* unit = addLabel(70); unit->setText("Km/h");
        auto* lab = addLabel(40);
        lab->setText("0");
        m_needle = addNeedle(60);
        m_needle->setLabel(lab);
        m_needle->setColor(Qt::blue);
        m_needle->setValueRange(0,100);
        addBackground(7);
        auto updateTheme = [this,arc,degrees,values,unit,lab] {
            auto* tokens = ThemeTokens::instance();
            arc->setColor(tokens->border()); degrees->setColor(tokens->textSecondary());
            values->setColor(tokens->textPrimary()); unit->setColor(tokens->textSecondary());
            lab->setColor(tokens->textPrimary()); m_needle->setColor(tokens->primary()); update();
        };
        connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,updateTheme);
        updateTheme();
    }

    void setValue(int v) { if (m_needle) { m_needle->setCurrentValue(v); update(); } }
private:
    QcNeedleItem* m_needle = nullptr;
};

} // namespace basewidgets

#endif // BASEWIDGETS_BASICGAUGE_H

