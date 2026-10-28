#pragma once
#include <basewidgets/Charts/QCustomplot.h>
namespace basewidgets {
/** 按可见像素列抽样的 QCPGraph，GUI 线程使用。
 * 原始数据仍在 data()，只在绘制/命中时抽样；放大后恢复局部细节。
 * 每桶保留首尾、最小和最大样点，按原始时间排序，不平均冲击峰。
 * NaN 缺口保持断线；线性/对数轴、反向轴均按屏幕位置分桶。
 * StyledPlot::addGraph 默认创建此类，仍可使用全部 QCPGraph 数据接口。
 */
class ACSS_UI_EXPORT SampledGraph : public QCPGraph {
    Q_OBJECT
public:
    explicit SampledGraph(QCPAxis* keyAxis,QCPAxis* valueAxis);
    /** 当前可见区的绘制样点（包含两侧邻点与断线点），不改变原始数据。
     * 每像素最多一个桶，总桶数最多 4096；每桶最多 4 个有效点和 5 个断线点。
     * 极密集缺样保守断线，不跨缺口连接；不可用值在显示中按 NaN 处理。
     * 返回拷贝用于诊断/预览，不替代 data() 的业务测量/导出。
     */
    QVector<QCPGraphData> displaySamples() const;
protected:
    void getOptimizedLineData(QVector<QCPGraphData>* output,const QCPGraphDataContainer::const_iterator& begin,
                             const QCPGraphDataContainer::const_iterator& end) const override;
    void getOptimizedScatterData(QVector<QCPGraphData>* output,QCPGraphDataContainer::const_iterator begin,
                                QCPGraphDataContainer::const_iterator end) const override;
};
}
