# 高级表格、多屏窗口与数据管理仪表

## 1. AdvancedTable：同一组件按需配置

模型链保持 `业务模型 → QSortFilterProxyModel → PagedProxyModel → TableView`。控件不拥有业务模型，模型应有明确的应用/页面父对象。

```cpp
#include <basewidgets/Views/AdvancedTable.h>
#include <basewidgets/Views/TableView.h>

using namespace basewidgets;
auto* table = new AdvancedTable(parent);
table->setSourceModel(recordingModel);
table->setPresentationMode(AdvancedTable::PresentationMode::Paged);
table->setPageSize(50);
table->setSelectionMode(QAbstractItemView::ExtendedSelection);
table->setSelectionBehavior(QAbstractItemView::SelectRows);
table->setEditTriggers(QAbstractItemView::NoEditTriggers);
table->view()->sortByColumn(0, Qt::AscendingOrder);
// 源模型第 0、3 行放顶部，第 10 行放底部；不是当前页面行号。
table->setPinnedRows({0, 3}, {10});
```

三种预设：

| 模式 | 工具区 | 分页 |
|---|---|---|
| `Basic` | 仅表格 | 关闭，显示全部筛选结果 |
| `Standard` | 筛选、列管理、统计、导出 | 关闭 |
| `Paged`（默认） | 完整工具区 | 开启 |

预设不会清除现有排序、筛选和固定记录；需要完全不排序时另调用 `setSortingEnabled(false)`。单独的可见性 setter 只改变显示，不改变模型行为。`setPageSize` 拒绝非正数，自定义有效页大小自动出现在选择器中。

### 固定记录的约定

- 固定的是业务记录及其显示顺序，**不是滚动视口冻结窗格**。同一表格内仍一起滚动。
- 顶部/底部顺序按调用者传入顺序保持；升序、降序和源模型插入不把固定状态转给另一条记录。
- 固定记录遵守筛选；被过滤的记录暂时隐藏，清空筛选后重新出现。
- 固定记录在每页重复，`pageSize` 只计算普通记录。例如每页 50 条普通记录加 3 条固定记录，视图显示 53 条。
- 最多固定 64 条，越界、重复或不同模型的索引使原子设置返回 `false`，保留原配置。
- 删除固定记录后自动移除其显示；源模型 `reset` / 替换会失去持久索引，调用者应按自己的稳定业务 ID 重新查找并设置固定行。库不猜测哪列是主键。
- `pinRows(selectedSourceRows(), Top/Bottom)` 可增量固定；`unpinRows` / `clearPinnedRows` 解除。行头的 ↑ / ↓ 表示固定位置；`PagedProxyModel::PinPositionRole` 便于自定义委托识别。
- 无普通记录但存在固定记录时仍有一页；完全空结果为 0 页；删行导致页码越界时自动收敛。

### 多选勾选与当前页全选

左侧行头默认显示复选框，左上角复选框全选/取消**当前页的可用记录**；部分勾选时显示半选，空页禁用全选。无分页模式对应全部当前筛选结果。固定记录在多页重复出现，但只计为一条勾选记录。

勾选状态与 Qt 高亮选择独立，按源记录保留；翻页、排序或筛选不会清空，删行会移除对应勾选，模型 reset/替换清空。不会改变源模型的 `CheckStateRole`、列号、编辑委托或 CSV 内容。业务批量操作请使用勾选接口，不要读取 `selectedSourceRows()`：

```cpp
table->setRowCheckBoxesVisible(true); // 默认 true；false 隐藏，不清空
connect(table, &AdvancedTable::checkedRowsChanged, table, [table] {
    const QModelIndexList records = table->checkedSourceRows(); // 跨页、包括筛选隐藏项
    // 按业务 ID 批量导出/归档/索引；异步保留时转 QPersistentModelIndex 或业务主键。
});
table->setRowChecked(sourceModel->index(5, 0), true);
table->setCurrentPageChecked(true);
table->clearCheckedRows();
```

行复选框点击切换，聚焦单元格后 `Ctrl+Space` 切换该记录；表头全选框可 Tab 聚焦并用 Space 操作。批量全选/清空只发送一次 `checkedRowsChanged`。Demo 显示跨页勾选数量并提供“清空勾选”。现有高亮选择、固定选中与 CSV Selection 的语义不变。

### 排序、选择、委托与菜单

数值排序应给源模型存数值类型，或通过 `setSortRole` 指定一个数值 role；录音 ID 等字符串不自动强制转数值。`view()`、`header()`、`sortFilterProxy()` 支持 Qt 既有委托、表头调整、排序 role、单元格编辑等扩展。纯单元格数据更新只转发 `dataChanged`，不再整表 reset；结构/排序变化后恢复仍在当前页的源记录选择。

内置右键菜单提供复制单元格/选中行 CSV、选择当前页、升降序、顶部/底部固定、解除固定、列宽自适应、显隐列与清空筛选。空白处也能打开菜单。`setDefaultContextMenuEnabled(false)` 可只保留业务菜单。

```cpp
connect(table, &AdvancedTable::prepareContextMenu, table,
        [table](QMenu* menu, const QModelIndexList& sourceRows) {
    // sourceRows 属于 setSourceModel 传入的模型，单元格选中也能得到所属源记录。
    const QPersistentModelIndex record = sourceRows.size() == 1
        ? sourceRows.first() : QModelIndex();
    auto* action = menu->addAction("查看录音详情");
    action->setEnabled(record.isValid());
    QObject::connect(action, &QAction::triggered, table, [record] {
        if (!record.isValid()) return; // 菜单打开期间可能发生删行/重置
        // 用 record 对应的业务 ID 打开详情。
    });
});
```

迁移注意：旧版 `prepareContextMenu` 传回分页视图索引；本版明确传回 `sourceModel()` 索引，已有槽不要再次调用分页映射。

菜单非阻塞，关闭后销毁；同步添加动作，不保留裸 `QMenu*`。`contextMenuRequested` / `sourceActivated` 输出源索引，`mapToSource` / `mapFromSource` 显式跨两层代理映射。普通记录不在当前页或已过滤时，`mapFromSource` 返回无效索引。

无效正则通过 `filterRejected` 报错并保留上次有效结果。CSV 的选区导出支持单元格选择，以所属行去重；过滤结果导出每条记录仅一次，当前页导出包含该页固定记录。使用 `QSaveFile` 提交，写入/提交失败返回 `false`，避免留下半个目标文件。

本轮移除了顶部隐藏的重复分页控件、重复同步信号和无效分页区间描述。`AdvancedTable` 是可组合的平面表格入口，树表用 `TreeTable`；数据库服务端排序/分页、千万级虚拟数据、Excel 公式引擎不由这个 UI 类隐式实现，应用仍应提供适合业务的数据模型。

## 2. 双屏/三屏与窗口恢复

`Core/WindowPlacement.h` 不假定屏幕数和主屏原点，使用各 `QScreen::availableGeometry()` 的逻辑坐标。选择实际可用屏，不把整个虚拟桌面包围盒（可能包含屏幕间空隙）视为有效区域。

```cpp
#include <basewidgets/Core/WindowPlacement.h>

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

Demo 新增“屏幕”菜单：移动主窗口到指定屏，或在指定屏打开独立数据监控窗口。可同时打开第二、第三屏监控窗口；每个屏幕身份独立持久化，重复打开会复用窗口。页面继续使用模拟数据，生产应用应接入共享业务数据源。

## 3. 数据管理仪表

公共头文件：`Gauges/DataManagementDashboards.h`。两个控件都是 QPainter + ThemeTokens，不注入 QSS、不自行读磁盘/数据库或启动后台任务。快照 setter 应在 GUI 线程调用；工作线程可先产出值对象，再通过 queued connection 切回 GUI。

### TaskQueueDashboard

```cpp
auto* tasks = new TaskQueueDashboard(parent);
tasks->setQueues({
    {"import", "录音导入", 12, 3, 280, 5},
    {"index", "索引构建", 24, 4, 240, 2},
    {"archive", "归档迁移", 60, 2, 120, 0}
});
// 状态顺序：排队、运行、成功、失败。id 用于业务联动，不用标题作为主键。
connect(tasks, &TaskQueueDashboard::stateActivated, receiver,
        [](const QString& id, TaskQueueDashboard::State state) { /* 筛选任务列表 */ });
```

最多 12 个队列，ID 必须非空、唯一，队列名非空，四种计数求和不得溢出。更新失败保留原快照。每队列完成率为 `(成功 + 失败) / 总任务数`，失败单列红色显示。不同处理阶段可能涉及同一录音，因此不把多个队列相加虚构全局进度。空队列显示“暂无任务”，不显示 100%。

支持鼠标点击、左右/上下方向键、Enter/Space 激活，焦点在快照重排后仍按稳定 ID 保留。内置复制快照菜单，`prepareContextMenu` 可追加重试、详情等业务动作；库不自动重试/删除真实任务。

### DataQualityDashboard

```cpp
auto* quality = new DataQualityDashboard(parent);
DataQualitySnapshot snapshot;
snapshot.total = 10000;
snapshot.valid = 7800;
snapshot.missing = 12;
snapshot.corrupt = 5;
snapshot.duplicate = 80;
snapshot.observedAt = backendSnapshotTime; // 不填则明确显示“未提供采样时间”
quality->setSnapshot(snapshot);
```

类别互斥：有效、缺失、损坏、重复；未核验量由总量扣除这些类别得到。业务后端需要先定义分类优先级，不能把同一条记录同时算入有效和重复。

- 覆盖率 = 已核验 / 总量。
- 通过率 = 有效 / 已核验。
- 无总量、未开始核验等场景返回 NaN、显示“—”；`clear()` 是“尚未采样”，合法全零快照是“空数据集”。
- 拒绝类别和超总量及整数溢出；支持计数点击/键盘分类选择和 `prepareContextMenu`，可联动文件列表或校验任务。
- 完整快照可通过 tooltip、无障碍描述及复制菜单读取；窄宽度绘制省略过长文本，保持比例和分类准确。

示例统一在 `examples/widgets_demo/`，`DataManagementPage` 组合容量、双向网速、任务队列、核验质量以及 QCustomPlot 速率历程；隐藏页面暂停模拟定时器。

## 4. 验证与边界

`widgets_demo --verify` 覆盖 Qt 模型检查器、固定记录与筛选/排序/插入/删除、选择映射、CSV、业务菜单、负坐标三屏/屏幕空隙/断屏回退、窗口持久化、仪表溢出/空数据/键盘鼠标交互。`--smoke-test` 检查九页、明暗主题和两种窗口宽度。

本轮 `--verify` 68 项通过，1.5 倍缩放同样通过；36 组页面截图、独立监控窗口打开/复用/关闭和安装后外部消费工程通过。

本环境为 Linux Qt 5.15 offscreen，可验证模型、逻辑、绘制及 1.5 倍缩放；无法代替真实双/三屏插拔、Windows 混合 DPI、Wayland 合成器定位的实机验收。请在实际设备上验证主屏切换、150%/100% 混合缩放、最大化跨屏、拔掉副屏、对话框父屏定位。
