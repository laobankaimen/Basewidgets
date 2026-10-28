# 常用交互与反馈组件

[文档首页](../README.md)

四个控件由 BaseWidgets 库公开提供，展示页位于 `examples/widgets_demo/pages/CommonWidgetsPage.cpp`。沿用 Qt Widgets 的事件和布局体系，使用 QPainter/NativeStyle 与 ThemeTokens，不注入 QSS；支持 dark、light、ocean 和外部主题。

| 控件 | 适用场景 | 业务接入 |
|---|---|---|
| `Inputs/SegmentedControl.h` | 列表/网格切换、全部/待核验/已归档 | `currentChanged(int)` |
| `Inputs/FilterChip.h` | 站点、格式、时间范围等筛选条件 | `toggled(bool)`、`removeRequested()` |
| `Containers/MessageBanner.h` | 页面内导入结果、离线提醒、校验告警 | `actionRequested()`、`dismissed()` |
| `Containers/EmptyState.h` | 暂无数据、无搜索结果、未连接 | `actionRequested()`、`secondaryActionRequested()` |

所有 API 均在 GUI 线程调用，父对象管理所有权。提示、动作与移除操作不会直接访问数据源。

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

## 演示与验收

主 demo 最后一个“常用组件”标签演示四个控件的组合：分段切换更新当前视图说明，勾选和移除更新条件计数，空状态主动作取消全部筛选，次动作展示提示，恢复按钮重建示例状态。

[海洋色预览](../images/common-widgets-ocean.png) · [亮色预览](../images/common-widgets-light.png)。键盘、RTL、禁用、主题与安装消费检查见[验证指南](../development/validation.md)。
