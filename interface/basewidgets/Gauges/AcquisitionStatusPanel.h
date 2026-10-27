#pragma once
#include <basewidgets/Core/StyledWidget.h>
#include <QMetaType>
namespace basewidgets {
enum class AcquisitionState { Idle, Recording, Paused, Disconnected };
/** 累计样点计数的单位由业务统一（例如全部通道样点总数）；received 不包括 lost。
 * writeBytesPerSecond 为实际写入速率，不由 fs/channel 推算压缩格式。
 * totalBytes=0 表示磁盘容量未知；预计可录时间只按 freeBytes / 实际写入速率估算。
 */
struct ACSS_UI_EXPORT AcquisitionStatus {
    AcquisitionState state=AcquisitionState::Idle;
    double sampleRate=0,writeBytesPerSecond=0;
    int channels=0;
    quint64 receivedSamples=0,lostSamples=0,freeBytes=0,totalBytes=0;
};
class ACSS_UI_EXPORT AcquisitionStatusPanel : public StyledWidget {
    Q_OBJECT
public:
    explicit AcquisitionStatusPanel(QWidget* parent=nullptr);
    bool setStatus(const AcquisitionStatus& status); ///< 无效快照整体拒绝，不将未知数据填为正常状态。
    const AcquisitionStatus& status() const { return m_status; }
    double lossPercent() const; ///< 尚无计数返回 NaN。
    double remainingSeconds() const; ///< 容量/速率未知返回 NaN；不含额外预留空间策略。
    QSize sizeHint() const override { return QSize(360,230); }
    QSize minimumSizeHint() const override { return QSize(260,210); }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    AcquisitionStatus m_status;
    bool m_valid=false;
};
}

Q_DECLARE_METATYPE(basewidgets::AcquisitionStatus)
