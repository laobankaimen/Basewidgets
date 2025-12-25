#ifndef QTADVANCEDUI_COMPASSGAUGE_H
#define QTADVANCEDUI_COMPASSGAUGE_H

#include <QtAdvancedUI/Gauges/QcGaugeWidget.h>

namespace QtAdvancedUI {

// 罗盘（0-360），在基础环上绘制四向字符
class ACSS_UI_EXPORT CompassGauge : public QcGaugeWidget {
    Q_OBJECT
public:
    explicit CompassGauge(QWidget* parent = nullptr)
        : QcGaugeWidget(parent)
    {
        // 完全按照 test/examples/Compass/mainwindow.cpp 的组合搭建
        addBackground(99);
        auto* bkg1 = addBackground(92);
        bkg1->clearrColors();
        bkg1->addColor(0.1, Qt::black);
        bkg1->addColor(1.0, Qt::white);

        auto* bkg2 = addBackground(88);
        bkg2->clearrColors();
        bkg2->addColor(0.1, Qt::white);
        bkg2->addColor(1.0, Qt::black);

        auto* w = addLabel(80); w->setText("W"); w->setAngle(0);   w->setColor(Qt::white);
        auto* n = addLabel(80); n->setText("N"); n->setAngle(90);  n->setColor(Qt::white);
        auto* e = addLabel(80); e->setText("E"); e->setAngle(180); e->setColor(Qt::white);
        auto* s = addLabel(80); s->setText("S"); s->setAngle(270); s->setColor(Qt::white);
        Q_UNUSED(w); Q_UNUSED(n); Q_UNUSED(e); Q_UNUSED(s);

        auto* deg = addDegrees(70);
        deg->setStep(5);
        deg->setMaxDegree(270);
        deg->setMinDegree(-75);
        deg->setColor(Qt::white);

        m_needle = addNeedle(60);
        m_needle->setNeedle(QcNeedleItem::CompassNeedle);
        m_needle->setValueRange(0,360);
        m_needle->setMaxDegree(360);
        m_needle->setMinDegree(0);
        addBackground(7);
        addGlass(88);
    }

    void setValue(int v) { if (m_needle) { m_needle->setCurrentValue(v); update(); } }

private:
    QcNeedleItem* m_needle = nullptr;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_COMPASSGAUGE_H
