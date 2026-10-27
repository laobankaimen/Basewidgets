#pragma once
#include <basewidgets/Charts/StyledPlot.h>
namespace basewidgets {
/** 均匀 X 网格 × 时间的有界热力图基类，供声谱/LOFAR/DEMON/测向历程共用。
 * 色阶和显示门限只修改绘制；levelAt 始终返回原始值。NaN/缺帧透明，不当作低值。
 * 用户缩放/框选后暂停时间跟随，双击/Home 复位时恢复到最新历史。
 */
class ACSS_UI_EXPORT HistoryHeatmapPlot : public StyledPlot {
    Q_OBJECT
public:
    enum class ColorPreset { Viridis, Gray, Heat };
    Q_ENUM(ColorPreset)
    explicit HistoryHeatmapPlot(QWidget* parent=nullptr);
    bool setLevelRange(double lower,double upper);
    QCPRange levelRange() const { return QCPRange(m_lower,m_upper); }
    bool setDisplayThreshold(double threshold,bool enabled=true);
    void clearDisplayThreshold();
    double displayThreshold() const { return m_threshold; }
    bool displayThresholdEnabled() const { return m_thresholdEnabled; }
    void setColorPreset(ColorPreset preset);
    ColorPreset colorPreset() const { return m_preset; }
    void setFollowLatest(bool enabled);
    bool followsLatest() const { return m_follow; }
    void clearHistory();
    int historyCapacity() const { return m_history.size(); }
    int storedFrameCount() const { return m_present; }
    int binCount() const { return m_bins; }
    double levelAt(int bin,int historyRow) const;
    double timeAt(int historyRow) const;
    double coordinateAt(int bin) const;
    QCPColorMap* colorMap() const { return m_map; } ///< 借用，勿替换内部数据几何。
signals:
    void cellSelected(double timeSeconds,double xCoordinate,double level,bool available);
    void levelRangeChanged(double lower,double upper);
    void displayThresholdChanged(double threshold,bool enabled);
    void followLatestChanged(bool enabled);
protected:
    bool configureGrid(double xMinimum,double xMaximum,int bins,int historyFrames,double periodSeconds,const QString& levelUnit);
    bool appendRow(double centerTimeSeconds,const QVector<double>& levels);
    bool setDisplayedXRange(double minimum,double maximum);
    void extendContextMenu(QMenu* menu,const QPointF& coordinate) override;
private:
    void synchronizeMap();
    void push(const QVector<double>& levels);
    void editColors();
    QVector<QVector<double>> m_history;
    int m_next=0,m_present=0,m_bins=0;
    double m_xMinimum=0,m_xMaximum=1,m_period=0,m_last=0,m_lower=-120,m_upper=0,m_threshold=-80;
    bool m_hasTime=false,m_dirty=false,m_follow=true,m_thresholdEnabled=false;
    ColorPreset m_preset=ColorPreset::Viridis;
    QCPRange m_displayX{0,1};
    QCPColorMap* m_map=nullptr;
    QCPColorScale* m_scale=nullptr;
};
}
