#pragma once
#include <basewidgets/Charts/StyledPlot.h>
#include <QMetaType>
namespace basewidgets {
/** 录音记录的状态分类；仅展示业务提供的状态，不推测文件质量。 */
enum class RecordingState { Ready, Processing, Flagged };
struct ACSS_UI_EXPORT RecordingInterval {
    QString id,channel,label;
    double startSeconds=0,endSeconds=0;
    RecordingState state=RecordingState::Ready;
};
/** 按通道排列的录音时间线。空白区是真实没有记录的时间，不自动填充。
 * setRecords 原子替换，最多 10000 段/64 通道，ID 唯一；可重叠，点击命中最后绘制的记录。
 * 相对秒和 UTC epoch 秒必须由调用方选择一致基准；数据单位不会由控件转换。
 */
class ACSS_UI_EXPORT RecordingTimelinePlot : public StyledPlot {
    Q_OBJECT
public:
    explicit RecordingTimelinePlot(QWidget* parent=nullptr);
    bool setRecords(const QVector<RecordingInterval>& records);
    const QVector<RecordingInterval>& records() const { return m_records; }
    void setUtcTimeAxis(bool utc);
    bool selectRecord(const QString& id);
    QString selectedRecordId() const { return m_selected; }
signals:
    void recordSelected(const QString& id);
private:
    void colorItems();
    void layoutLabels();
    QVector<RecordingInterval> m_records;
    QVector<QCPItemRect*> m_rectangles;
    QVector<QCPItemText*> m_labels;
    QStringList m_channels;
    QString m_selected;
};
}

Q_DECLARE_METATYPE(basewidgets::RecordingInterval)
Q_DECLARE_METATYPE(QVector<basewidgets::RecordingInterval>)
