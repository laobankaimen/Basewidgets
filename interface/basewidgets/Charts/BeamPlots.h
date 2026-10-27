#pragma once
#include <basewidgets/Charts/StyledPlot.h>
#include <basewidgets/Charts/HistoryHeatmapPlot.h>
namespace basewidgets {
/** 多测向波束曲线：显示业务计算后的方位响应，不假定阵列几何或执行波束形成。
 * 所有曲线使用相同角度约定和电平单位；最多 32 束，每束最多 16384 点。
 * 角度严格递增且在配置范围内，NaN 电平断线。支持图例/菜单显隐与方位游标。
 */
class ACSS_UI_EXPORT BeamPatternPlot : public StyledPlot {
    Q_OBJECT
public:
    explicit BeamPatternPlot(QWidget* parent=nullptr);
    bool setBearingRange(double lowerDegrees,double upperDegrees);
    bool setLevelRange(double lower,double upper);
    void setLevelUnit(const QString& label);
    bool setBeam(const QString& id,const QString& name,const QVector<double>& bearings,const QVector<double>& levels,const QColor& color=QColor());
    bool setBeamVisible(const QString& id,bool visible);
    bool removeBeam(const QString& id);
    void clearBeams();
    int beamCount() const { return m_beams.size(); }
    bool setBearingCursor(double degrees);
signals:
    void beamPicked(const QString& id,double bearingDegrees,double level);
    void bearingCursorChanged(double degrees);
protected:
    void extendContextMenu(QMenu* menu,const QPointF& coordinate) override;
private:
    struct Beam { QString id; QCPGraph* graph=nullptr; };
    QVector<Beam> m_beams;
    QCPItemStraightLine* m_cursor=nullptr;
    double m_lower=0,m_upper=360;
};
/** 测向历程：每行是一帧均匀方位网格的业务响应，色值可为 dB 或业务评分。
 * 默认北=0°/顺时针，请按实际算法输入；不自动判定目标、航向或跟踪关联。
 */
class ACSS_UI_EXPORT BearingHistoryPlot : public HistoryHeatmapPlot {
    Q_OBJECT
public:
    explicit BearingHistoryPlot(QWidget* parent=nullptr);
    bool configure(double lowerDegrees,double upperDegrees,int bearingBins,int frames,double periodSeconds,const QString& levelUnit="波束电平 (dB)");
    bool appendFrame(double centerTime,const QVector<double>& levels);
signals:
    void bearingSelected(double timeSeconds,double degrees,double level,bool available);
};
}
