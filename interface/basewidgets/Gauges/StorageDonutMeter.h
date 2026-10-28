#pragma once
#include <basewidgets/Core/StyledWidget.h>
namespace basewidgets {
/** 环形容量指示器。输入为字节；total=0、used=0 表示未知，不显示虚假的 0%。
 * 只展示数据；不轮询设备。读数、单位和占比均由同一快照计算，GUI 线程使用。
 * 不赋予“低占用=故障”等隐含语义；业务告警应由外部提示组件表达。
 */
class ACSS_UI_EXPORT StorageDonutMeter : public StyledWidget {
    Q_OBJECT
public:
    enum class UnitSystem { IEC, SI };
    Q_ENUM(UnitSystem)
    explicit StorageDonutMeter(QWidget* parent=nullptr);
    /** used > total 整组拒绝并保留旧值；支持完整 quint64 范围。 */
    bool setCapacity(quint64 usedBytes,quint64 totalBytes);
    quint64 usedBytes() const { return m_used; }
    quint64 totalBytes() const { return m_total; }
    bool capacityAvailable() const { return m_total!=0; }
    double percent() const; ///< 未知时返回 NaN，不作为 0 使用。
    void setUnitSystem(UnitSystem system); ///< 默认 IEC（KiB/MiB）；SI 对应 kB/MB/TB。
    UnitSystem unitSystem() const { return m_units; }
    QString usedText() const;
    QString totalText() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QString formatBytes(quint64 bytes) const;
    void refreshDescription();
    quint64 m_used=0,m_total=0;
    UnitSystem m_units=UnitSystem::IEC;
};
}
