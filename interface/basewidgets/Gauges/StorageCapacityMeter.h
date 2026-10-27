#pragma once
#include <basewidgets/Core/StyledWidget.h>
#include <QColor>
#include <QVector>
#include <QMetaType>
namespace basewidgets {
struct ACSS_UI_EXPORT StorageSegment { QString name; quint64 bytes=0; QColor color; };
/** 容量分布仪表，输入都是 bytes，显示使用 IEC KiB/MiB/GiB。
 * total=0/空分段表示未知；各分段和不能超总量。剩余量=total-已分配，不伪造磁盘采样。
 * 点击色块发出索引/名称/字节数，index=-1 表示空闲区；业务可关联目录或文件列表。
 */
class ACSS_UI_EXPORT StorageCapacityMeter : public StyledWidget {
    Q_OBJECT
public:
    explicit StorageCapacityMeter(QWidget* parent=nullptr);
    bool setCapacity(quint64 totalBytes,const QVector<StorageSegment>& segments);
    quint64 totalBytes() const { return m_total; }
    quint64 usedBytes() const { return m_used; }
    quint64 freeBytes() const { return m_total-m_used; }
    bool setWarningPercent(double percent);
    double warningPercent() const { return m_warning; }
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return QSize(260,210); }
signals:
    void segmentSelected(int index,const QString& name,quint64 bytes);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
private:
    QVector<StorageSegment> m_segments;
    quint64 m_total=0,m_used=0;
    double m_warning=90;
};
}

Q_DECLARE_METATYPE(basewidgets::StorageSegment)
Q_DECLARE_METATYPE(QVector<basewidgets::StorageSegment>)
