#pragma once
#include <basewidgets/Core/StyledWidget.h>
#include <QDateTime>
#include <QVector>
#include <QMetaType>
class QMenu;
namespace basewidgets {
/** 同一队列内四种状态互斥；不同队列可以是不同处理阶段，不自动相加成“总任务数”。 */
struct ACSS_UI_EXPORT TaskQueueSnapshot {
    QString id;                 ///< 稳定业务键，用于点击/菜单回调。
    QString title;              ///< 展示名称，如“录音导入”“索引构建”“归档迁移”。
    quint64 queued=0,running=0,succeeded=0,failed=0;
};
/** QPainter 任务队列仪表，不启动任务、不访问线程池；业务传入快照。
 * 单队列完成率=(成功+失败)/该队列总数，失败始终单独显示，不伪装为成功。
 * 支持鼠标分类点击、方向键选择、Enter/Space 激活及可扩展右键菜单。 */
class ACSS_UI_EXPORT TaskQueueDashboard : public StyledWidget {
    Q_OBJECT
public:
    enum class State { Queued, Running, Succeeded, Failed };
    Q_ENUM(State)
    explicit TaskQueueDashboard(QWidget* parent=nullptr);
    /** 原子更新；最多 12 个队列，id 非空且唯一，计数求和不得溢出。失败保持原快照。 */
    bool setQueues(const QVector<TaskQueueSnapshot>& queues);
    QVector<TaskQueueSnapshot> queues() const { return m_queues; }
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:
    void queuesChanged();
    void stateActivated(const QString& queueId,basewidgets::TaskQueueDashboard::State state);
    /** menu 归仪表持有，关闭后销毁；同步添加业务 QAction，id 按值传递。 */
    void prepareContextMenu(QMenu* menu,const QString& queueId,basewidgets::TaskQueueDashboard::State state);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;
private:
    QRectF stateRect(int queue,int state) const;
    int hit(const QPoint& point) const;
    void activate();
    QVector<TaskQueueSnapshot> m_queues;
    int m_selected=0;
};
/** 核验结果互斥：valid/missing/corrupt/duplicate 之和 <= total。
 * 未核验=total-各结果之和；duplicate 是已判定重复并单独归类的记录，不包含于 valid。
 * observedAt 可省略；生产数据建议包含采样时刻，控件不伪造当前时间。 */
struct ACSS_UI_EXPORT DataQualitySnapshot {
    quint64 total=0,valid=0,missing=0,corrupt=0,duplicate=0;
    QDateTime observedAt;
};
/** 数据质量概览：核验覆盖率与已核验通过率分开，未知和空数据不显示为 100%。 */
class ACSS_UI_EXPORT DataQualityDashboard : public StyledWidget {
    Q_OBJECT
public:
    enum class Category { Valid, Missing, Corrupt, Duplicate, Unchecked };
    Q_ENUM(Category)
    explicit DataQualityDashboard(QWidget* parent=nullptr);
    /** 原子校验计数，失败保持原快照；total=0 且全部计数为 0 表示空数据集。 */
    bool setSnapshot(const DataQualitySnapshot& snapshot);
    void clear(); ///< 恢复“尚未采样”，与空数据集区分。
    bool hasSnapshot() const { return m_known; }
    DataQualitySnapshot snapshot() const { return m_snapshot; }
    quint64 checkedRecords() const;
    quint64 uncheckedRecords() const;
    double coveragePercent() const; ///< 无快照或 total=0 返回 NaN。
    double passPercent() const;     ///< 尚未核验任何记录时返回 NaN。
    QSize sizeHint() const override { return QSize(420,380); }
    QSize minimumSizeHint() const override { return QSize(280,380); }
signals:
    void snapshotChanged();
    void categoryActivated(basewidgets::DataQualityDashboard::Category category);
    void prepareContextMenu(QMenu* menu,basewidgets::DataQualityDashboard::Category category);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;
private:
    QRectF categoryRect(int category) const;
    int hit(const QPoint& point) const;
    DataQualitySnapshot m_snapshot;
    bool m_known=false;
    int m_selected=0;
};
}
Q_DECLARE_METATYPE(basewidgets::TaskQueueSnapshot)
Q_DECLARE_METATYPE(basewidgets::TaskQueueDashboard::State)
Q_DECLARE_METATYPE(basewidgets::DataQualitySnapshot)
Q_DECLARE_METATYPE(basewidgets::DataQualityDashboard::Category)
