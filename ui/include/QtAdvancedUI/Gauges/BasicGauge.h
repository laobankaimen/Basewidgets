#ifndef QTADVANCEDUI_BASICGAUGE_H
#define QTADVANCEDUI_BASICGAUGE_H

#include <QtAdvancedUI/Gauges/QcGaugeWidget.h>

namespace QtAdvancedUI {

// 基础速度表（0-100），参考 Basic 示例风格
class ACSS_UI_EXPORT BasicGauge : public QcGaugeWidget {
    Q_OBJECT
public:
    explicit BasicGauge(QWidget* parent = nullptr)
        : QcGaugeWidget(parent)
    {
        // 完全按照 test/examples/Basic/mainwindow.cpp 的组合搭建
        addArc(55);
        addDegrees(65)->setValueRange(0,100);
        auto* clrBand = addColorBand(50);
        clrBand->setValueRange(0,100);
        addValues(80)->setValueRange(0,100);
        addLabel(70)->setText("Km/h");
        auto* lab = addLabel(40);
        lab->setText("0");
        m_needle = addNeedle(60);
        m_needle->setLabel(lab);
        m_needle->setColor(Qt::blue);
        m_needle->setValueRange(0,100);
        addBackground(7);
    }

    void setValue(int v) { if (m_needle) { m_needle->setCurrentValue(v); update(); } }
private:
    QcNeedleItem* m_needle = nullptr;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_BASICGAUGE_H
