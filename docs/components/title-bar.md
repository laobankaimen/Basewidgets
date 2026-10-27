# 主窗口、子屏与弹窗标题栏

[文档首页](../README.md)

`CustomTitleBar` 是可独立组合的控件，主窗口通过 `FramelessWidgetWindow::titleBar()` 获取；弹窗通过 `FramelessDialog::titleBar()` 获取。返回借用指针，窗口持有其生命周期。

布局采用应用标识、标题、业务操作、窗口控制四个区域：主窗口可放搜索、设置、用户和任务入口；对话框默认专注标题与关闭。窗口控制始终在右侧，普通操作使用轻量图标或文字，不常驻强调色；关闭悬停仍使用危险色。

| 项目 | MainWindow | SecondaryWindow | Dialog |
|---|---|---|---|
| 基础高度 | 48 逻辑像素 | 40 逻辑像素 | 40 逻辑像素 |
| 大字体 | 自动增加高度 | 自动增加高度 | 自动增加高度 |
| 默认标题 | 窗口标题，空时应用名 | 窗口标题，空时应用名 | 窗口标题，空时应用名 |
| 默认图标 | 窗口 → 应用 → 内嵌图标 | 不显示，可显式设置 | 不显示，可显式设置 |
| 窗口按钮 | 最小化、最大化、关闭 | 最小化、最大化、关闭 | 关闭 |
| 默认双击 | 最大化/还原 | 最大化/还原 | 不切换状态 |
| 搜索/设置/用户/业务动作 | 按需开启，默认关闭 | 隐藏扩展区，保留配置 | 隐藏扩展区，保留配置 |

窗口角色和生命周期见[多屏工作区](multi-screen.md)。

`setProfile` 切换配置时会重设窗口按钮和双击偏好；按需随后调用 `setButtonsVisible` 等。主窗口和弹窗初始化时已经选好对应 profile，不需要重复调用。`FramelessDialog::setResizeable(true)` 仍可显式开启最大化等能力。

标题栏不做网络请求、不执行登录、不持有业务权限逻辑，也不直接操作文件。它通过 Qt 信号和 QAction 交给应用处理业务。

## 标题、图标与对齐分别配置

```cpp
#include <basewidgets/Core/FramelessWidgetWindow.h>
#include <basewidgets/Core/CustomTitleBar.h>

window->setTitle("水声数据处理软件");     // 系统窗口标题，同时是标题栏默认标题
window->setWindowIcon(applicationIcon);    // 系统图标，同时是主标题栏默认图标

auto* bar = window->titleBar();
bar->setTitleAlignment(basewidgets::CustomTitleBar::TitleAlignment::Center);
bar->setTitle("项目 / 航次 2026-10");      // 仅覆盖标题栏，不改系统窗口标题
bar->setIcon(projectIcon);                 // 单独覆盖图标，换主题不覆盖用户设置
bar->setIconVisible(false);                // 临时隐藏，不清空图标配置
bar->setIconVisible(true);

bar->resetTitle(); // 恢复跟随窗口/应用标题
bar->resetIcon();  // 独立恢复图标默认规则
```

- `Left` 使用物理左侧对齐；`Center` 优先以**整个窗口**的中心为基准，给两侧留对称空间，不是对剩余区域简单居中。
- 窄窗口优先折叠操作与搜索，标题省略显示、tooltip 保留全文。极窄且无法留下至少 72 像素的对称标题区时，退让到剩余标题区域，确保不盖住关闭等控件。
- 对于 RTL 内容，文本使用 Qt 的双向文字处理；系统窗口按钮和业务区仍保持当前物理位置，不随文本方向换边。
- `setTitle("")` 是明确的空标题；`setIcon(QIcon())` 是明确无图标；恢复默认应使用 `resetTitle/resetIcon`，而不是传空值。
- `attachToWindow` 不转移窗口所有权。重绑定会断开旧窗口同步，保留显式覆盖设置。可独立创建标题栏并绑定其他 QWidget 顶层窗口。
- 主/弹窗都提供 `titleBar()`；不要通过替换主窗口的 menuWidget 或标题栏子控件破坏内部布局。

## 搜索、用户与设置

```cpp
#include <basewidgets/Inputs/ToolButton.h>
#include <QMenu>

bar->setSearchVisible(true);
bar->setSearchPlaceholderText("搜索录音 / 任务");
bar->setSearchShortcut(QKeySequence("Ctrl+K")); // 默认 Ctrl+K；空序列禁用快捷键
connect(bar, &basewidgets::CustomTitleBar::searchSubmitted, receiver,
        [](const QString& query) { /* 调用查询服务或筛选业务模型 */ });
connect(bar, &basewidgets::CustomTitleBar::searchTextChanged, receiver,
        [](const QString& text) { /* 可在业务层做 debounce / 补全 */ });

bar->setSettingsVisible(true);
connect(bar, &basewidgets::CustomTitleBar::settingsRequested, receiver,
        [] { /* 打开应用设置页 */ });
// 也可直接给入口绑定 QMenu；有菜单时直接打开菜单，不再发送 settingsRequested。
auto* settingsMenu = new QMenu(bar);
settingsMenu->addAction("显示设置");
bar->settingsButton()->setMenu(settingsMenu);

bar->setUserVisible(true);
bar->setUserInfo("演示用户", loadedAvatarPixmap, "数据分析 / 本地账户");
auto* accountMenu = new QMenu(bar);
accountMenu->addAction("个人信息");
bar->userButton()->setMenu(accountMenu);
```

搜索在宽窗口显示输入框，窄窗口变为图标，点击或快捷键打开输入浮层。两处文本通过 `setSearchText/searchText` 与同一对信号同步；Enter 提交，浮层 Escape 关闭。禁用搜索或切换为 SecondaryWindow/Dialog profile 后搜索快捷键也禁用。浮层按当前窗口屏幕工作区限制位置；Wayland 的最终全局位置仍由合成器决定。

`searchEdit()` 返回内嵌输入框，可设置 completer 等。注意窄屏浮层拥有独立输入框：文本通过公共 API 同步，但给内嵌框设置的自定义 completer/validator 不会自动复制到浮层。需要统一业务验证时在 `searchSubmitted` 中处理。

图标来源、尺寸和 DPR 规则统一见[主题与图标资源](../guides/themes-and-resources.md#图标与高-dpi)。

头像按圆形中心裁切，空头像显示姓名首个完整字形（支持 Unicode 组合字符），姓名也为空则显示用户轮廓。窄窗口仅显示头像，完整姓名和 detail 保留在 tooltip/无障碍名称。头像由调用者提供 QPixmap，不根据姓名或账户隐式请求网络。

设置、头像、更多操作的紧凑纯图标按钮不额外挤入下拉箭头，点击区域和键盘菜单行为仍由 QToolButton 管理；宽窗口用户名旁保留下拉箭头。

## 自定义动作与窄窗口

```cpp
auto* tasks = new QAction(QIcon(), "任务中心", window); // QAction 所有权归应用
bar->addTitleAction(tasks);
connect(tasks, &QAction::triggered, receiver, [] { /* 打开任务面板 */ });

// 支持 QAction 的 enabled、visible、checkable 和 menu。
// 可以复用同一 QAction 到应用菜单和工具栏。
bar->removeTitleAction(tasks); // 只从标题栏移除，不删除 QAction
```

业务动作按添加顺序展示。空间不足时自动放进“更多”菜单，复用原 QAction，不复制业务回调。动作被销毁时自动移除对应按钮；重复添加同一个动作不会产生多个入口。设置/用户在更窄窗口中也会进入“更多”，已有菜单作为子菜单保留。

标题栏最小宽度为 220 逻辑像素，优先确保窗口控制可达。主窗口内容的最小宽度仍由业务布局决定。不要对标题栏内部按钮另设固定尺寸；通过 QAction、菜单、公开 setter 配置即可。

## 弹窗接入

```cpp
#include <basewidgets/Core/FramelessDialog.h>

auto* dialog = new basewidgets::FramelessDialog(parent);
dialog->setTitle("录音详情");
dialog->titleBar()->setTitleAlignment(basewidgets::CustomTitleBar::TitleAlignment::Center);
// 默认没有图标；需要项目标识时单独设置。
dialog->titleBar()->setIcon(recordingIcon);
dialog->open();
```

Dialog profile 与主窗口使用同一拖动/窗口控制基础，不带复杂业务区。无需复制平台代码。拖动命中通过 `isDraggableAt` 统一判断：交互控件不拖动，标题/图标/空白可拖动。Windows 原生 WM_NCHITTEST 和非 Windows 手动/系统拖动共用这条规则；关闭状态、拖动释放丢失等原有处理保留。

## 示例与验证

`examples/widgets_demo/main.cpp` 演示标题对齐、独立标题、图标/搜索显隐、主题切换、账户菜单和任务中心；搜索提交后切换到录音表格并筛选。“对话框”页提供简洁居中标题示例。

覆盖默认/覆盖/重绑定、profile、居中、窄屏、不重叠布局、菜单生命周期、搜索和头像的检查见[验证指南](../development/validation.md)。旧 setTitle/setIcon 跟随行为迁移见[迁移指南](../development/migration.md#表格与标题栏)。
