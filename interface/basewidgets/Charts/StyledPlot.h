#pragma once
#include <basewidgets/Charts/QCustomplot.h>
#include <QMenu>
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
    void replot(QCustomPlot::RefreshPriority priority=QCustomPlot::rpQueuedReplot);
    void setPlotTitle(const QString& title);
    QString plotTitle() const;
    void refreshTheme();
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
    void applyMode();
    void acceptRectangle(const QRect& rectangle);
    InteractionMode m_mode=InteractionMode::Pan;
    Qt::Orientations m_navigationAxes=Qt::Horizontal|Qt::Vertical;
    QCPRange m_homeX{0,1},m_homeY{0,1};
    bool m_hasHome=false,m_temporaryZoom=false,m_pressed=false;
    QPoint m_pressPixel;
};
}
