#ifndef BASEWIDGETS_ATTITUDEMETER_H
#define BASEWIDGETS_ATTITUDEMETER_H
#include <basewidgets/Core/StyledWidget.h>
namespace basewidgets {
/** 姿态可视化：pitch 正值表示抬头（地平线向下），roll 正值表示向右倾侧。
 * pitch 限制为 [-90,90]，roll 归一化到 [-180,180)。只绘制姿态，不做传感器融合。
 * 天空/地面保持蓝/棕语义色；外围刻度、数值与全应用明暗主题一致。
 */
class ACSS_UI_EXPORT AttitudeMeter : public StyledWidget {
    Q_OBJECT
public:
    explicit AttitudeMeter(QWidget* parent=nullptr);
    void setAttitude(double pitch,double roll); ///< 原子更新；非有限值切换为无数据。
    void setCurrentPitch(float pitch) { setAttitude(pitch,m_roll); }
    void setCurrentRoll(float roll) { setAttitude(m_pitch,roll); }
    double pitch() const { return m_pitch; }
    double roll() const { return m_roll; }
    void setTitle(const QString& title);
    void setValueAvailable(bool available);
    bool valueAvailable() const { return m_available; }
    QSize sizeHint() const override { return QSize(300,300); }
    QSize minimumSizeHint() const override { return QSize(180,210); }
signals:
    void attitudeChanged(double pitch,double roll);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    void refreshDescription();
    double m_pitch=0,m_roll=0;
    bool m_available=true;
    QString m_title;
};
}
#endif
