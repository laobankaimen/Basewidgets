#ifndef QTADVANCEDUI_ATTITUDEMETER_H
#define QTADVANCEDUI_ATTITUDEMETER_H

#include <QtAdvancedUI/Gauges/QcGaugeWidget.h>

namespace QtAdvancedUI {

// 简化版姿态仪：绘制天空/地面与地平线，支持 pitch/roll
class ACSS_UI_EXPORT AttitudeMeter : public QcGaugeWidget {
    Q_OBJECT
public:
    explicit AttitudeMeter(QWidget* parent = nullptr)
        : QcGaugeWidget(parent)
    {
        addBackground(99);
        auto* bkg = addBackground(92);
        bkg->clearrColors();
        bkg->addColor(0.1, Qt::black);
        bkg->addColor(1.0, Qt::white);
        m_attMeter = addAttitudeMeter(88);

        auto* needle = addNeedle(70);
        needle->setMinDegree(0);
        needle->setMaxDegree(180);
        needle->setValueRange(0,180);
        needle->setCurrentValue(90);
        needle->setColor(Qt::white);
        needle->setNeedle(QcNeedleItem::AttitudeMeterNeedle);
        addGlass(80);
    }

    void setCurrentPitch(float pitch) { if (m_attMeter) { m_attMeter->setCurrentPitch(pitch); update(); } }
    void setCurrentRoll(float roll)   { if (m_attMeter) { m_attMeter->setCurrentRoll(roll);   update(); } }

private:
    QcAttitudeMeter* m_attMeter = nullptr;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_ATTITUDEMETER_H
