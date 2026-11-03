#pragma once
#include <basewidgets/Charts/QCustomplot.h>
#include <QMenu>
#include <QPointer>
#include <QElapsedTimer>
class QTimer;
namespace basewidgets {
/** 全部业务图表的交互基类（GUI 线程）。平移、滚轮、Shift 框选缩放、双击/Home 复位、
 * 菜单扩展、坐标拾取、区域选择及 PNG 导出共用同一入口，不修改 QCustomPlot 源码。
 */
class ACSS_UI_EXPORT StyledPlot : public QCustomPlot {
    Q_OBJECT
public:
    enum class InteractionMode { Pan, RectangleZoom, RectangleSelect, Inspect };
    Q_ENUM(InteractionMode)
    explicit StyledPlot(QWidget* parent=nullptr);
    /** 默认合并数据更新/导航绘制，最多 maximumFrameRate 帧/秒。
     * rpImmediateRefresh 等显式非 queued 请求立即绘制，用于同步导出/检查。
     * 避免通过 QCustomPlot* 调用其同名非虚函数绕过调度。 */
    Q_SLOT void replot(QCustomPlot::RefreshPriority priority=QCustomPlot::rpQueuedReplot);
    /** 默认 30，允许 1..120；仅限制展示帧率，不丢弃接收数据。非法值保持旧设置。 */
    bool setMaximumFrameRate(int framesPerSecond);
    int maximumFrameRate() const { return m_maximumFrameRate; }
    /** 创建 SampledGraph，归本图持有；data() 保留原始数据，显示按像素保极值抽样。
     * 与 QCustomPlot 的同名非虚接口兼容；直接 new QCPGraph 不获得此抽样。 */
    QCPGraph* addGraph(QCPAxis* keyAxis=nullptr,QCPAxis* valueAxis=nullptr);
    void setPlotTitle(const QString& title);
    QString plotTitle() const;
    void refreshTheme();
    /** 绑定本图曲线的默认主题序列色（paletteIndex >= 0，循环取色）。
     * overrideColor 有效时改为固定业务颜色；传无效 QColor 可恢复主题跟随。
     * 保留线宽/线型，不改变数据或视口；空指针、外部图曲线或负下标返回 false。
     * 直接 setPen 改成其他颜色也会退出主题跟随，建议用本接口明确控制。
     */
    bool setSeriesColor(QCPGraph* graph, int paletteIndex, const QColor& overrideColor=QColor());
    void setInteractionMode(InteractionMode mode);
    InteractionMode interactionMode() const { return m_mode; }
    void setNavigationAxes(Qt::Orientations axes); ///< 同时控制拖动、滚轮和框选缩放的轴。
    Qt::Orientations navigationAxes() const { return m_navigationAxes; }
    /** 更新复位基准但不立刻改视口。流式控件更新最新数据边界后调用，复位回到最新历史。 */
    bool setResetRanges(const QCPRange& x,const QCPRange& y);
    void captureResetView();
    QCPRange resetXRange() const { return m_homeX; }
    QCPRange resetYRange() const { return m_homeY; }
    virtual void resetView();
    /** 原子设置视口，用于外部联动；不会改变复位基准。log 轴范围必须严格为正。 */
    bool setViewRanges(const QCPRange& x,const QCPRange& y);
    /** 返回新菜单，所有权交给调用者；prepareContextMenu 必须使用直接连接，不能缓存 menu 指针。 */
    QMenu* createContextMenu(const QPoint& plotPixel,QWidget* parent=nullptr);
    bool saveSnapshot(const QString& fileName,int width=0,int height=0,double scale=1);
    /** 若外部源发起联动，可显式通知用户导航，暂停流式跟随；不自动循环连接多个图。 */
    void notifyUserNavigation();
signals:
    void prepareContextMenu(QMenu* menu,const QPointF& coordinate);
    void coordinateHovered(double x,double y);
    void coordinateClicked(double x,double y,Qt::KeyboardModifiers modifiers);
    void selectionMade(double xMinimum,double xMaximum,double yMinimum,double yMaximum);
    void viewRangesChanged(double xMinimum,double xMaximum,double yMinimum,double yMaximum);
    void userNavigationStarted();
    void viewReset();
    void interactionModeChanged(basewidgets::StyledPlot::InteractionMode mode);
protected:
    virtual void extendContextMenu(QMenu* menu,const QPointF& coordinate);
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    QCPTextElement* m_title=nullptr;
private:
    struct ThemedSeries { QPointer<QCPGraph> graph; int paletteIndex; QColor appliedColor; };
    QVector<ThemedSeries> m_themedSeries;
    QTimer* m_replotTimer=nullptr;
    QElapsedTimer m_lastReplot;
    int m_maximumFrameRate=30;
    void applyMode();
    void acceptRectangle(const QRect& rectangle);
    InteractionMode m_mode=InteractionMode::Pan;
    Qt::Orientations m_navigationAxes=Qt::Orientations(Qt::Horizontal)|Qt::Vertical;
    QCPRange m_homeX{0,1},m_homeY{0,1};
    bool m_hasHome=false,m_temporaryZoom=false,m_pressed=false;
    QPoint m_pressPixel;
};
}
