#pragma once
#include <basewidgets/QtAdvancedUIExport.h>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QVector>
class QWidget;
class QScreen;
namespace basewidgets {
/** 多屏窗口位置工具，坐标一律使用 Qt 逻辑像素，不按主屏 DPR 缩放全局坐标。
 * 不读写 QSettings；每个业务窗口由调用者分别保存 save() 返回的数据。
 * 普通窗口可主动跨屏；Wayland 的最终位置由合成器决定，返回 CompositorManaged。
 */
class ACSS_UI_EXPORT WindowPlacement : public QObject {
    Q_OBJECT
public:
    enum class MoveResult { Applied, CompositorManaged, InvalidTarget };
    /** 安装热插拔/可用区域变化恢复保护，重复调用返回同一个实例。窗口持有其生命周期。
     * 不在日常拖动时强制限制窗口，也不更改最大化/全屏状态。 */
    static WindowPlacement* watch(QWidget* window);
    void setRecoveryEnabled(bool enabled) { m_enabled=enabled; }
    bool recoveryEnabled() const { return m_enabled; }
    /** 选择交叠面积最大的屏；没有交叠时优先 fallback，否则选最近的有效屏。空列表返回 -1。
     * 可用于验证负坐标、屏幕间空隙等拓扑，不依赖真实显示器。 */
    static int bestScreen(const QRect& rectangle,const QVector<QRect>& available,int fallback=-1);
    /** 缩小并限制矩形到一个屏幕的可用区域；无有效屏时返回原矩形。 */
    static QRect fitToScreens(const QRect& rectangle,const QVector<QRect>& available,int fallback=-1);
    /** 立即恢复普通窗口可见性；最小尺寸大于屏幕时保留其最小尺寸并保证标题栏可达。 */
    static void ensureVisible(QWidget* window);
    static MoveResult moveToScreen(QWidget* window,QScreen* screen);
    /** 对话框按父窗口所在屏幕居中；父窗口不在屏内时回退至可用屏幕。 */
    static void centerOnParent(QWidget* window);
    static QByteArray save(const QWidget* window);
    /** 恢复屏幕身份、普通尺寸和最大化/全屏状态；损坏或版本不符返回 false，保持原窗口不动。
     * 建议在 show() 前调用；已断开的屏幕回退到仍可用的屏，不恢复最小化状态。 */
    static bool restore(QWidget* window,const QByteArray& data);
protected:
    bool eventFilter(QObject* watched,QEvent* event) override;
private:
    explicit WindowPlacement(QWidget* window);
    void observe(QScreen* screen);
    void scheduleRecovery();
    QPointer<QWidget> m_window;
    bool m_enabled=true,m_pending=false;
};
}
