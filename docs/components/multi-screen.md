# 主控与子屏工作区

[文档首页](../README.md)

工作区使用两个公开类：

- `ScreenWindow`：继承 FramelessWidgetWindow，分为 Primary/Secondary 角色。
- `MultiScreenWorkspace`：管理一个主控窗口和多个真正的子屏顶层窗口。

| 区域/行为 | 主控窗口 | 子屏窗口 |
|---|---|---|
| 标题栏 | MainWindow profile，48 基础高度，可配全局搜索/设置/用户 | SecondaryWindow profile，40 基础高度，默认不带全局入口和应用 Logo |
| 窗口按钮 | 最小化/最大化/关闭 | 最小化/最大化/关闭；不是对话框 |
| 菜单、工具栏 | 应用配置全局导航与操作 | 默认没有，可独立添加业务操作 |
| 状态栏 | 全局会话/任务状态与“主控”标记 | 自己的业务状态、“子屏”标记、“返回主屏”入口 |
| 内容 | 主业务导航和控制 | 每个子屏可完全不同；支持共享业务模型 |
| 最小化 | 不带着子屏一起最小化 | 独立窗口状态 |
| 关闭 | 接受关闭后隐藏全部子屏 | 仅隐藏自己，重开复用已有内容/缩放状态 |

字体变大时标题高度随字体增加。子屏无 QWidget 父窗口，窗口管理器不会因 transient 父子关系而将它随主控一起最小化。子屏生命周期由 MultiScreenWorkspace 管理，控制器销毁时统一删除。不要把子屏指针的所有权转交给其他窗口。

## 创建两屏或三屏界面

```cpp
#include <basewidgets/Core/ScreenWindow.h>
#include <basewidgets/Core/MultiScreenWorkspace.h>
#include <basewidgets/Core/CustomTitleBar.h>
#include <QGuiApplication>

using namespace basewidgets;
auto* primary = new ScreenWindow(ScreenWindow::Role::Primary);
primary->setTitle("水声数据处理 · 主控");
primary->setContentWidget(createPrimaryPage(sharedSession));
primary->titleBar()->setSearchVisible(true);
primary->titleBar()->setSettingsVisible(true);
primary->titleBar()->setUserVisible(true);
primary->setContextText("采集会话 / 全局任务状态");

auto* workspace = new MultiScreenWorkspace(primary); // 默认由 primary 持有控制器
QObject::connect(workspace, &MultiScreenWorkspace::windowCreated, primary,
    [sharedSession](const QString& id, ScreenWindow* child) {
        if (id == "bearing") {
            child->setContentWidget(createBearingPage(sharedSession));
            child->setContextText("测站 A / 实时测向");
        } else if (id == "storage") {
            child->setContentWidget(createStoragePage(sharedSession));
            child->setContextText("存储池 / 数据入库");
        }
        // 不在此调用 showMaximized（会提前出现在错误屏幕）；只设置待显示状态。
        child->setWindowState(Qt::WindowMaximized);
    });

const auto screens = QGuiApplication::screens();
if (!screens.isEmpty()) {
    workspace->setPrimaryScreen(screens[0]); // 示例；生产程序可由用户选择主控屏
    primary->showMaximized();
    if (screens.size() > 1)
        workspace->openSecondary("bearing", screens[1], "测向分析 · 子屏");
    if (screens.size() > 2)
        workspace->openSecondary("storage", screens[2], "数据管理 · 子屏");
}
```

上例 `create*Page/sharedSession` 是应用提供的页面工厂和会话模型，不是库内函数。控制器不推测水声算法/数据库权限；应用将同一个模型或会话对象传入不同页面。视图的缩放、当前页、选区可以独立维护。

每个业务 ID 只有一个窗口，重复 openSecondary 会移动/显示已有实例，不重新创建内容。windowCreated 只在首次创建时发出，发生在 show 之前；窗口被用户关闭后再次打开，仍复用该实例。主动删除窗口则会从管理器移除，下次打开重新创建。

## 自动分配与断屏行为

```cpp
// 对每个非主屏自动创建一个子屏工作区。主屏可通过 setPrimaryScreen 自由切换。
// 自动 id 为 screen-<硬件/连接口键>，可在 windowCreated 中按业务分配内容。
workspace->setAutomaticSecondaryWindows(true);
workspace->showAllSecondaryScreens(); // 重新打开主动关闭的自动子窗口
workspace->closeSecondary("bearing");
workspace->activatePrimary();
```

- 主控角色不等于操作系统的 primaryScreen。用户选择任意在线屏幕作为应用主屏。
- 自动模式跳过主控所在屏幕、已有手动业务窗口占用的屏幕；已创建的自动窗口被占用时暂停显示。
- 断开的子屏隐藏并保留窗口/业务状态，避免两个或三个窗口都被 Qt 的通用回退逻辑挤到主屏。
- 同一屏幕键重新出现，恢复原本应显示的窗口；用户主动关闭的窗口不会因热插拔自动重开。
- 主控所在屏幕丢失时回退到系统当前可用主屏。自动子屏和正常手动子屏不会覆盖新的主控屏。
- 显式 openSecondary 到当前主控屏被视为预览，允许共屏；这让单屏开发环境也能检查不同窗口角色。
- 硬件键包含厂商、型号、序列号和屏幕名称，不依赖屏幕枚举顺序。换接连接口后屏幕名称可能改变；跨连接口绑定或跨应用重启的业务视图持久化由应用按稳定业务 ID 管理。
- availableGeometry 变化时重新收敛子屏的位置。窗口自身仍支持最小/最大尺寸和正常缩放。
- `secondaryAvailabilityChanged(id, available)` 可通知主控当前子屏是否可用。
- Wayland 可能拒绝客户端绝对定位，发出 `compositorPlacementRequested`；窗口角色/独立实例仍有效，最终物理位置需桌面合成器配合。Windows/X11 也应做实际设备验证。

库不读写全局 QSettings。主窗口位置仍可用 WindowPlacement 保存/恢复；多屏控制器保存的是运行期角色和实例，不自动持久化业务页面。主控关闭进入停止状态，若应用支持关闭后重新创建会话，应创建新的工作区控制器。

## 普通窗口的位置保存与恢复

普通独立窗口使用 WindowPlacement 恢复到可见区域；工作区管理的子屏由 MultiScreenWorkspace 在断屏时隐藏，两套策略不要同时启用。管理器已经自动关闭子屏的通用位置恢复保护。

`Core/WindowPlacement.h` 不假定屏幕数和主屏原点，使用各 `QScreen::availableGeometry()` 的逻辑坐标。选择实际可用屏，不把整个虚拟桌面包围盒（可能包含屏幕间空隙）视为有效区域。

```cpp
#include <basewidgets/Core/WindowPlacement.h>
#include <QSettings>
using basewidgets::WindowPlacement;
QSettings settings;

// 推荐在 show 前恢复。每个窗口独立 key，不由库写入全局 QSettings。
WindowPlacement::restore(window, settings.value("analysisWindow/placement").toByteArray());
window->show();
// 退出时：
settings.setValue("analysisWindow/placement", WindowPlacement::save(window));

// 主窗口/FramelessDialog 已自动安装保护；普通 QWidget 顶层窗口可主动安装：
WindowPlacement::watch(auxiliaryWindow);
// screen 必须来自当前 QGuiApplication::screens()；不要长期保存无保护裸指针。
auto result = WindowPlacement::moveToScreen(auxiliaryWindow, screen);
```

- 保存屏幕名称/厂商/型号/序列号、普通尺寸、逻辑位置、最大化/全屏状态；恢复不把窗口带回最小化。
- 屏幕身份唯一时优先恢复同一屏，并适配其位置变化；断开时选剩余有效屏，夹取尺寸与位置。
- 观察屏幕增减、分辨率/工作区变化；信号合并后恢复普通窗口。最大化/全屏交给 Qt/窗口管理器，不在日常拖动中强制吸回某一屏。
- 同时考虑原生边框和 QWidget 客户区；业务最小尺寸大于屏幕时保留最小尺寸并保证标题可达。复杂业务页面应使用可滚动/自适应布局。
- 对话框按父窗口所在屏幕居中，避免总在主屏显示。
- Windows 标题栏命中改为 `ScreenToClient → 局部 DPR 换算`，边缘命中宽度按当前窗口 DPR 换算；不再将整个桌面全局原点除以单个屏幕 DPR。
- Wayland 由合成器控制绝对位置：跨屏请求返回 `CompositorManaged`，不承诺强制移动成功。窗口尺寸仍会收敛；弹窗仍保留父子关系。
- `watch(window)->setRecoveryEnabled(false)` 可禁用自动保护；`ensureVisible` 手动恢复，`bestScreen` / `fitToScreens` 可对纯矩形拓扑做验证。

## 示例

`examples/widgets_demo/main.cpp` 在普通启动时为其他屏幕创建子屏工作区；主控和每个子屏可展示不同页面。子屏录音表格直接引用主控源模型，图表仍为各页面的模拟数据。

“屏幕”菜单提供主控屏选择、自动创建开关、重新显示子屏和单屏预览；子屏状态栏的“返回主屏”只激活主控，不关闭自己。
