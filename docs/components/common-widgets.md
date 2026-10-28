# 常用交互、状态与反馈组件

[文档首页](../README.md)

本页八个小控件由 BaseWidgets 库公开提供，展示页位于 `examples/widgets_demo/pages/CommonWidgetsPage.cpp`。沿用 Qt Widgets 的事件和布局体系，使用 QPainter/NativeStyle 与 ThemeTokens，不注入 QSS；支持 dark、light、ocean、sg-blue 和外部主题。

| 控件 | 适用场景 | 业务接入 |
|---|---|---|
| `Inputs/StatusBadge.h` | 在线/核验/告警等只读状态 | `setText()`、`setTone()` |
| `Inputs/SearchEdit.h` | 录音/目录/任务的延迟搜索 | `searchRequested(QString)` |
| `Inputs/CopyField.h` | 路径、编号、地址的选择与复制 | `setText()`、`copied(QString)` |
| `Containers/CollapsibleSection.h` | 高级参数和详情收纳 | `contentLayout()`、`expandedChanged(bool)` |
| `Inputs/SegmentedControl.h` | 列表/网格切换、全部/待核验/已归档 | `currentChanged(int)` |
| `Inputs/FilterChip.h` | 站点、格式、时间范围等筛选条件 | `toggled(bool)`、`removeRequested()` |
| `Containers/MessageBanner.h` | 页面内导入结果、离线提醒、校验告警 | `actionRequested()`、`dismissed()` |
| `Containers/EmptyState.h` | 暂无数据、无搜索结果、未连接 | `actionRequested()`、`secondaryActionRequested()` |

所有 API 均在 GUI 线程调用，父对象管理所有权。提示、动作与移除操作不会直接访问数据源。

## 常用组件页布局

原独立的“视图控件”页已经合并至“常用组件”，基础紧凑分组包括：搜索与复制、状态徽标、参数收纳、导航与列表、基础表格、树形表格、视图与筛选、消息反馈、空状态；另有消息中心与可配置关于窗口示例。宽窗口三列，宽度不足时自动减少为两列或单列；只保留一个页面级滚动区。视图内仍可滚动浏览更多记录，保留模型所有权、导航业务 ID 和分组求和示例。

减少页边距、标题留白与控件间距，不缩小正文；所有分组默认不增加鼠标悬停效果。[SG 主题效果](../images/sg-blue-common-widgets.png)。

## 状态徽标 StatusBadge

只读胶囊标签，不抢焦点，不作为可选按钮。语义圆点与浅底色跟随主题，文字始终用于表达状态；没有整块 hover 高亮。长文字省略显示，`text()`、tooltip、无障碍名称保留全文。

```cpp
#include <basewidgets/Inputs/StatusBadge.h>

auto* state = new basewidgets::StatusBadge("设备在线", parent);
state->setTone(basewidgets::FeedbackTone::Success);
state->setDotVisible(true); // 默认 true；可关闭圆点，仅保留文字
```

默认 `Neutral`；也可选择 Info/Success/Warning/Danger。状态由业务更新，不包含动画或设备轮询。

## 延迟搜索 SearchEdit

继承 LineEdit，可继续使用 completer、validator、选择和输入法。自带主题 SVG 搜索图标和清除按钮。`textChanged`（包括程序 `setText/clear`）在停止输入 300 ms 后合并发出一次请求；Enter 或点击搜索图标立即提交，取消待发的重复请求。

```cpp
#include <basewidgets/Inputs/SearchEdit.h>

auto* search = new basewidgets::SearchEdit(parent);
search->setPlaceholderText("搜索录音名称");
search->setDebounceInterval(300); // 允许 0..60000 ms；0 表示下一事件循环合并
connect(search, &basewidgets::SearchEdit::searchRequested, receiver,
        [](const QString& query) { /* 更新代理模型或提交异步查询。 */ });
```

空字符串可用于清空筛选，不隐式 trim。输入法预编辑不提交；有 validator 时只提交可接受值。Escape 清空可编辑内容，沿用延迟提交规则。禁用会取消待发请求，重新启用不会自动补发；`cancelPendingSearch()` 只取消控件尚未发出的请求，已经发起的异步查询由业务取消/处理乱序结果。`requestSearch()` 可显式提交一次。

## 一键复制 CopyField

只读字段支持选区和 Ctrl+C，旁边按钮复制全部原文；成功操作后短暂显示勾号。输入框水平滚动查看长文本，复制不会截取显示区域。空值或整个控件禁用时不复制，不改正文。

```cpp
#include <basewidgets/Inputs/CopyField.h>

auto* id = new basewidgets::CopyField("REC-2026-1028-001", parent);
connect(id, &basewidgets::CopyField::copied, receiver,
        [](const QString& originalText) { /* 可在状态栏显示已复制提示。 */ });
id->setText("REC-2026-1028-002");
```

`copy()` 写入系统 Clipboard 并发出 `copied`，不表示其他进程已读取。`textChanged` 表示字段值改变；`textEdit()` 返回借用的只读 LineEdit，供设置无障碍名称/选择等，不应删除或改成可编辑。此控件会将内容交给系统剪贴板，业务应自行决定哪些字段允许复制。

## 折叠分组 CollapsibleSection

标题按钮支持点击、Space 和 Enter。默认展开；折叠仅隐藏内容，输入值与 QObject 生命周期保持不变，焦点在内容内部时会移回标题。没有高度动画或持续定时器。

```cpp
#include <basewidgets/Containers/CollapsibleSection.h>
#include <basewidgets/Inputs/SpinBox.h>
#include <QVBoxLayout>

auto* section = new basewidgets::CollapsibleSection("高级参数", parent);
auto* overlap = new basewidgets::SpinBox(section->contentWidget());
overlap->setRange(0, 90); overlap->setValue(50); overlap->setSuffix(" %");
section->contentLayout()->addWidget(overlap);
section->setExpanded(false);
```

`contentLayout()` 接受 addWidget/addLayout；内容控件归容器所有，不要替换或删除借用的布局。`setExpanded` 幂等，状态变化才发 `expandedChanged`。支持 RTL、长标题省略和完整提示。适用于表单中的参数收纳；折叠不自动停止内部业务任务。

## 分段切换

SegmentedControl 继承 QTabBar，保留禁用项、左右键、RTL、助记符、长文本省略和窄宽度滚动。适合水平顶部的少量短选项，不用于可关闭/拖动的文档标签；文档页面继续用 TabWidget。选中项采用独立底色，默认无鼠标悬停底色，键盘焦点保留。

```cpp
#include <basewidgets/Inputs/SegmentedControl.h>

auto* mode = new basewidgets::SegmentedControl(parent);
mode->setAccessibleName("录音视图");
mode->addTab("全部录音");
mode->addTab("待核验");
mode->setTabData(0, "all");
mode->setTabData(1, "pending");
QObject::connect(mode, &QTabBar::currentChanged, parent, [mode](int index) {
    if (index < 0) return;
    const QString businessId = mode->tabData(index).toString();
    // 按 businessId 更新查询，或连接 QStackedWidget::setCurrentIndex。
});
```

索引变化才发 currentChanged；调用 setCurrentIndex 也会发信号，状态回显时可使用 QSignalBlocker。删除全部选项后索引为 -1。

## 筛选标签

默认可选中、未选中、不可移除，文字过长时仅绘制省略，text() 保留完整文字。可设置 tooltip 显示全文。setCheckable(false) 可用作仅能点击/移除的条件标签。

```cpp
#include <basewidgets/Inputs/FilterChip.h>

auto* chip = new basewidgets::FilterChip("格式 · WAV", parent);
chip->setRemovable(true);
chip->setChecked(true);
chip->setToolTip(chip->text());
QObject::connect(chip, &QAbstractButton::toggled, parent, [](bool enabled) {
    // 同步业务筛选条件的启用状态。
});
QObject::connect(chip, &basewidgets::FilterChip::removeRequested, parent, [chip] {
    // 先同步业务模型；本例隐藏，以便后续恢复。
    chip->hide();
});
```

Space/Enter 切换，Delete 请求移除。右侧 × 可独立 Tab 聚焦，不会顺带切换标签。removeRequested 只通知业务，不自动隐藏或删除；接收者可以删除对象。启用/隐藏/移除后，业务应把焦点移到适当的相邻条件或筛选入口。

`setTone(FeedbackTone::Success)` 等语义色来自主题。正文保持可读颜色，选中线、勾与浅底色表达状态；不靠大面积实色区分条件。

## 行内消息

```cpp
#include <basewidgets/Containers/MessageBanner.h>

auto* message = new basewidgets::MessageBanner(parent);
message->setTone(basewidgets::FeedbackTone::Warning);
message->setTitle("部分记录需要核验");
message->setMessage("可以继续浏览已完成的数据，稍后处理剩余记录。");
message->setActionText("查看记录");
QObject::connect(message, &basewidgets::MessageBanner::actionRequested,
                 parent, [] { /* 打开业务记录页。 */ });
```

FeedbackTone 提供 Neutral/Info/Success/Warning/Danger。消息带语义图形、文字和细侧线，正文自动换行；动作单独成行避免窄窗口挤压。所有文字按纯文本显示，不解释业务返回的 HTML。

默认可关闭，关闭时 hide 并发 dismissed，可再次 show；重复关闭不会重复发信号。setDismissible(false) 隐藏关闭按钮并禁止 dismiss；setActionText("") 隐藏动作。没有自动消失计时器、模态事件循环或焦点抢占。

## 空状态

```cpp
#include <basewidgets/Containers/EmptyState.h>

auto* empty = new basewidgets::EmptyState(parent);
empty->setTitle("没有符合条件的录音");
empty->setDescription("清除筛选条件，或选择其他数据目录。");
empty->setActionText("清除筛选");
empty->setSecondaryActionText("选择目录");
QObject::connect(empty, &basewidgets::EmptyState::actionRequested,
                 parent, [] { /* 清除业务筛选并重新查询。 */ });
```

默认托盘图形由 QPainter 绘制，无额外图片依赖；setIcon(QIcon) 可传业务图标，空 QIcon 恢复默认图形。标题与说明可换行，两个动作分行排列；空字符串隐藏对应动作。控件不创建或导入数据，业务应在结果变为非空时切换回实际列表。

## 消息铃铛与关于页

新增铃铛、消息模型/面板与关于页面的完整接入说明见[消息中心与关于页](notifications-and-about.md)。常用组件页提供模拟消息、共享未读数、名称/版本编辑和关于弹窗预览。

## 演示与验收

主 demo 的“常用组件”标签演示八个小控件及基础视图的组合：搜索框筛选任务列表，复制字段展示完整编号，状态选项更新徽标，折叠参数保留值；分段切换更新当前视图说明，勾选和移除更新条件计数，空状态主动作取消全部筛选，次动作展示提示，恢复按钮重建示例状态。

[海洋色预览](../images/common-widgets-ocean.png) · [亮色预览](../images/common-widgets-light.png)。键盘、RTL、禁用、主题与安装消费检查见[验证指南](../development/validation.md)。
