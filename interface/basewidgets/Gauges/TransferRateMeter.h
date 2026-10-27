#pragma once
#include <basewidgets/Core/StyledWidget.h>
#include <limits>
namespace basewidgets {
/** 双向速率仪表：可由累计字节计数计算实际区间平均速率，也可直接输入 bytes/s。
 * 首帧只建立基准；计数器下降或超长间隔重建基准，不产生负值/尖峰。计数回绕不猜测位宽。
 * NetworkMbps 使用十进制 bit/s；BinaryMiBPerSecond 使用 IEC bytes/s，二者不可混淆。
 */
class ACSS_UI_EXPORT TransferRateMeter : public StyledWidget {
    Q_OBJECT
public:
    enum class DisplayUnit { NetworkMbps, BinaryMiBPerSecond };
    Q_ENUM(DisplayUnit)
    explicit TransferRateMeter(QWidget* parent=nullptr);
    bool addCounters(double timeSeconds,quint64 receivedBytes,quint64 sentBytes);
    bool setRates(double timeSeconds,double receivedBytesPerSecond,double sentBytesPerSecond);
    void reset();
    void setStale(bool stale); ///< 无更新时由业务监控显式标记过期；控件不猜测时间基准。
    bool hasRates() const { return m_hasRates&&!m_stale; }
    double receiveBytesPerSecond() const;
    double sendBytesPerSecond() const;
    bool setMaximumSampleGap(double seconds);
    bool setLinkCapacity(double bitsPerSecond); ///< 每个方向的配置容量；0=未知，不合并全双工两方向。
    void setDisplayUnit(DisplayUnit unit);
    DisplayUnit displayUnit() const { return m_unit; }
    QSize sizeHint() const override { return QSize(380,230); }
    QSize minimumSizeHint() const override { return QSize(260,210); }
signals:
    void ratesUpdated(double timeSeconds,double receiveBytesPerSecond,double sendBytesPerSecond);
    void baselineReset(const QString& reason);
    void rateValidityChanged(bool valid);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    bool m_hasCounters=false,m_hasRates=false,m_stale=false;
    quint64 m_received=0,m_sent=0;
    double m_last=0,m_rx=0,m_tx=0,m_gap=30,m_capacityBits=0;
    DisplayUnit m_unit=DisplayUnit::NetworkMbps;
};
}
