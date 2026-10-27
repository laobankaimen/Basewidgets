# 高级表格

[文档首页](../README.md)

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

## 固定记录的约定

- 固定的是业务记录及其显示顺序，**不是滚动视口冻结窗格**。同一表格内仍一起滚动。
- 顶部/底部顺序按调用者传入顺序保持；升序、降序和源模型插入不把固定状态转给另一条记录。
- 固定记录遵守筛选；被过滤的记录暂时隐藏，清空筛选后重新出现。
- 固定记录在每页重复，`pageSize` 只计算普通记录。例如每页 50 条普通记录加 3 条固定记录，视图显示 53 条。
- 最多固定 64 条，越界、重复或不同模型的索引使原子设置返回 `false`，保留原配置。
- 删除固定记录后自动移除其显示；源模型 `reset` / 替换会失去持久索引，调用者应按自己的稳定业务 ID 重新查找并设置固定行。库不猜测哪列是主键。
- `pinRows(selectedSourceRows(), Top/Bottom)` 可增量固定；`unpinRows` / `clearPinnedRows` 解除。行头的 ↑ / ↓ 表示固定位置；`PagedProxyModel::PinPositionRole` 便于自定义委托识别。
- 无普通记录但存在固定记录时仍有一页；完全空结果为 0 页；删行导致页码越界时自动收敛。

## 多选勾选与当前页全选

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

## 排序、选择、委托与菜单

数值排序应给源模型存数值类型，或通过 `setSortRole` 指定一个数值 role；录音 ID 等字符串不自动强制转数值。`view()`、`header()`、`sortFilterProxy()` 支持 Qt 既有委托、表头调整、排序 role、单元格编辑等扩展。纯单元格数据更新只转发 `dataChanged`，不再整表 reset；结构/排序变化后恢复仍在当前页的源记录选择。

内置右键菜单提供复制单元格/选中行 CSV、选择当前页、升降序、顶部/底部固定、解除固定、列宽自适应、显隐列与清空筛选。空白处也能打开菜单。`setDefaultContextMenuEnabled(false)` 可只保留业务菜单。

```cpp
#include <QMenu>
#include <QPersistentModelIndex>

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

迁移已有菜单槽时，注意[源模型索引语义](../development/migration.md#表格与标题栏)。

菜单非阻塞，关闭后销毁；同步添加动作，不保留裸 `QMenu*`。`contextMenuRequested` / `sourceActivated` 输出源索引，`mapToSource` / `mapFromSource` 显式跨两层代理映射。普通记录不在当前页或已过滤时，`mapFromSource` 返回无效索引。

无效正则通过 `filterRejected` 报错并保留上次有效结果。CSV 的选区导出支持单元格选择，以所属行去重；过滤结果导出每条记录仅一次，当前页导出包含该页固定记录。使用 `QSaveFile` 提交，写入/提交失败返回 `false`，避免留下半个目标文件。

`AdvancedTable` 是可组合的平面表格入口，树表用 `TreeTable`；数据库服务端排序/分页、千万级虚拟数据、Excel 公式引擎不由这个 UI 类隐式实现，应用仍应提供适合业务的数据模型。

## 示例与验证

示例：`examples/widgets_demo/pages/PagedTablePage.cpp`。检查覆盖模型一致性、固定记录、跨页勾选、排序/筛选/删行、CSV 与菜单生命周期；运行方式见[验证指南](../development/validation.md)。
