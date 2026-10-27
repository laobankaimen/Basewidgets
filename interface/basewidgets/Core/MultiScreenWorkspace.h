#pragma once
#include <basewidgets/QtAdvancedUIExport.h>
#include <basewidgets/Core/ScreenWindow.h>
#include <QObject>
#include <QPointer>
#include <QMap>
#include <QStringList>
class QScreen;
namespace basewidgets {
/** 一个主控窗口 + 多个独立子屏窗口。GUI 线程使用，主控关闭后统一隐藏子屏。
 * 控制器不拥有 primary；控制器拥有创建的子屏，销毁控制器会删除子屏。
 * 子屏使用独立标题/状态栏且无 QWidget 父窗口，不随主屏最小化。
 * 未指定 controllerParent 时由 primary 持有控制器；应用也可自行指定 QObject 生命周期。
 */
class ACSS_UI_EXPORT MultiScreenWorkspace : public QObject {
    Q_OBJECT
public:
    explicit MultiScreenWorkspace(ScreenWindow* primary,QObject* controllerParent=nullptr);
    ~MultiScreenWorkspace() override;
    ScreenWindow* primaryWindow() const { return m_primary; }
    QScreen* primaryScreen() const;
    /** 主屏是应用角色，不等于操作系统 primaryScreen。手动切换保留各子屏业务实例。 */
    bool setPrimaryScreen(QScreen* screen);
    /** 每个业务 id 最多一个子屏实例；重复调用重新显示/移动已有窗口，不重建内容。
     * screen 必须当前在线。允许明确放到主屏（便于单屏预览/调试）。
     * 第一次创建在 windowCreated 信号中配置内容、标题动作和独立状态栏。 */
    ScreenWindow* openSecondary(const QString& id,QScreen* screen,const QString& title=QString());
    ScreenWindow* secondaryWindow(const QString& id) const;
    QStringList secondaryIds() const;
    QScreen* targetScreen(const QString& id) const;
    bool isSuspended(const QString& id) const;
    void closeSecondary(const QString& id);
    /** 自动为每个非主屏建立一个子窗口；热插拔新增/暂停/恢复。
     * 断开的子屏隐藏并保留内容，避免全部挤到主屏；用户主动关闭的窗口不自动重开。
     * 自动 id 为 screen-<屏幕键>，业务可按 windowCreated 接入不同视图。 */
    void setAutomaticSecondaryWindows(bool enabled);
    bool automaticSecondaryWindows() const { return m_automatic; }
    /** 重新显示被用户关闭的自动子窗口，随后按当前拓扑同步。 */
    void showAllSecondaryScreens();
    /** 当前硬件/连接口身份键（厂商/型号/序列号/屏幕名称），不依赖列表顺序。
     * 换接连接口后名称可能改变；应用需要跨连接口恢复时应使用自己的设备绑定策略。 */
    static QString screenKey(const QScreen* screen);
    /** 纯拓扑决策，返回除主屏之外的有效索引，供应用规划及无硬件测试。 */
    static QList<int> secondaryScreenIndexes(int screenCount,int primaryIndex);
public slots:
    void synchronizeScreens();
    void activatePrimary();
signals:
    /** window 为借用指针；槽在首次 show 前运行。应用在这里提供共享业务模型的页面。 */
    void windowCreated(const QString& id,basewidgets::ScreenWindow* window);
    void secondaryAvailabilityChanged(const QString& id,bool available);
    void primaryScreenChanged(QScreen* screen);
    /** Wayland 发出此提示，最终物理定位由桌面合成器决定，不能保证强制指定屏幕。 */
    void compositorPlacementRequested(const QString& id,QScreen* screen);
private:
    struct Entry {
        QPointer<ScreenWindow> window;
        QString screenKey;
        bool automatic=false,wanted=true,suspended=false,allowPrimary=false;
    };
    ScreenWindow* createSecondary(const QString& id,const QString& title);
    void placeSecondary(const QString& id,QScreen* screen,bool force=false);
    void scheduleSync();
    void observeScreen(QScreen* screen);
    QPointer<ScreenWindow> m_primary;
    QPointer<QScreen> m_primaryScreen;
    QMap<QString,Entry> m_entries;
    bool m_automatic=false,m_syncPending=false,m_syncing=false,m_stopped=false,m_destroying=false,m_reposition=false;
};
}
