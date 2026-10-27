#pragma once
#include <basewidgets/Core/StyledWidget.h>
class QTimer; class QCheckBox;
namespace basewidgets { class StorageCapacityMeter; class TransferRateMeter; class RateHistoryPlot; class TaskQueueDashboard; class DataQualityDashboard; }
/** 存储/吞吐示例，所有数字为合成快照，不读取本机磁盘或网卡。 */
class DataManagementPage : public basewidgets::StyledWidget {
public:
    explicit DataManagementPage(QWidget* parent=nullptr);
protected:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;
private:
    void advance();
    basewidgets::TaskQueueDashboard* m_tasks=nullptr;
    basewidgets::DataQualityDashboard* m_quality=nullptr;
    basewidgets::StorageCapacityMeter* m_capacity=nullptr;
    basewidgets::TransferRateMeter* m_rate=nullptr;
    basewidgets::RateHistoryPlot *m_network=nullptr,*m_disk=nullptr,*m_indexing=nullptr;
    QTimer* m_timer=nullptr;
    QCheckBox* m_run=nullptr;
    double m_time=0;
    quint64 m_received=0,m_sent=0;
};
