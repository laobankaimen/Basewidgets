#pragma once
#include <basewidgets/Charts/StyledPlot.h>
namespace basewidgets {
/** 不等间隔速率/指标历程。values 与 addSeries 顺序一致，所有曲线必须使用相同单位。
 * 最多 16 序列、每序列 8192 点；时间倒序拒绝，NaN 和超出 gapSeconds 的间隔显示断线。
 */
class ACSS_UI_EXPORT RateHistoryPlot : public StyledPlot {
    Q_OBJECT
public:
    explicit RateHistoryPlot(QWidget* parent=nullptr);
    bool configure(int capacity,double visibleSeconds,const QString& unit,double gapSeconds=5);
    int addSeries(const QString& name,const QColor& color=QColor());
    bool appendValues(double seconds,const QVector<double>& values);
    void clearSamples();
    void setFollowLatest(bool enabled);
    bool followsLatest() const { return m_follow; }
    int storedPointCount() const;
    void setAutoScaleY(bool enabled);
signals:
    void followLatestChanged(bool enabled);
private:
    void updateDataView();
    QVector<QCPGraph*> m_series;
    int m_capacity=300;
    double m_window=60,m_gap=5,m_last=0;
    bool m_hasData=false,m_follow=true,m_autoY=true;
};
}
