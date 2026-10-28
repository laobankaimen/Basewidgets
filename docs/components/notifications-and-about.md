# 消息中心与关于页面

[文档首页](../README.md)

这些组件使用 Qt Widgets、QPainter 与主题色，支持嵌入页面或独立弹出。所有接口在 GUI 线程调用；网络、持久化、消息路由和检查更新属于应用逻辑。

| 类型 | 公开头文件 | 用途 |
|---|---|---|
| NotificationModel | `Views/NotificationModel.h` | 有界消息列表与共享已读状态 |
| NotificationBell | `Inputs/NotificationBell.h` | 矢量铃铛、未读数字或红点 |
| NotificationPanel | `Containers/NotificationPanel.h` | 消息列表、未读筛选、详情、操作 |
| NotificationPopup | `Dialogs/NotificationPopup.h` | 根据锚点屏幕放置的弹出包装 |
| AboutWidget / AboutInfo | `Containers/AboutWidget.h` | 可嵌入的产品信息页与配置 |
| AboutDialog | `Dialogs/AboutDialog.h` | 简洁关于弹窗 |

## 铃铛与共享模型

```cpp
#include <basewidgets/Views/NotificationModel.h>
#include <basewidgets/Inputs/NotificationBell.h>
#include <basewidgets/Dialogs/NotificationPopup.h>

// window 为应用提供的 QWidget；一个模型可同时绑定主窗口、子屏和嵌入面板。
auto* messages = new basewidgets::NotificationModel(window);
messages->setMaximumCount(200);
auto* bell = new basewidgets::NotificationBell(window);
bell->setModel(messages); // 借用，不转移模型所有权

auto* popup = new basewidgets::NotificationPopup(window);
popup->setModel(messages);
connect(bell, &QToolButton::clicked, popup, [popup, bell] {
    if (popup->isVisible()) popup->hide();
    else popup->popup(bell);
});

basewidgets::NotificationMessage item;
item.id = "import-001"; // 稳定 ID，由业务生成
item.title = "录音导入完成";
item.body = "12 个录音文件已导入，等待核验。";
item.timestamp = QDateTime::currentDateTime(); // 也可传实际服务器时间；空值不显示时间
item.tone = basewidgets::FeedbackTone::Success;
item.actionText = "查看录音";
messages->addMessage(item);
```

- 新 ID 插入列表最前；相同 ID 原位替换，包括 `read` 状态，不自动重新排序。业务若只更新正文，应先取得 `message(id)` 再修改，保留既有 read 状态。
- 默认最多 200 条，可设 1..10000；超过容量淘汰末尾旧消息。缩小容量也立即裁掉末尾，未读数同步更新。需要更长审计历史时由业务存储保存。
- `markRead(id, bool)`、`markAllRead()`、`removeMessage(id)`、`clear()` 更新所有绑定视图；相同已读状态不重复发送数量信号。
- 列表遵循 QAbstractItemModel 规则，不要在模型变更信号回调内重入修改/删除同一模型。外部消息线程可通过 queued signal/GUI 上下文回调提交快照。
- 铃铛默认数字角标，大于 99 显示 `99+`；`setDotOnly(true)` 改为红点，tooltip 和无障碍名称保留完整数量。未读 0 隐藏角标。
- `setUnreadCount(n)` 可用于独立铃铛；有模型绑定时，下次模型数量变化会覆盖手动值。`setModel(nullptr)` 解绑并清零，模型销毁也会清零。

## 消息面板与业务动作

NotificationPanel 使用 Model/View 委托绘制行，列表只展示摘要，详情显示完整纯文本。打开面板不自动标记已读；点击消息或按 Enter 查看详情时标记该条已读。默认也有“仅未读”“全部已读”“清空”入口。

```cpp
#include <basewidgets/Containers/NotificationPanel.h>

// 可直接加入业务布局，或者使用 popup->panel()。
auto* panel = new basewidgets::NotificationPanel(parent);
panel->setModel(messages);
panel->setTitle("任务与系统消息");
panel->setUnreadOnly(true);
connect(panel, &basewidgets::NotificationPanel::actionRequested,
        receiver, [](const QString& id) { /* 根据消息 ID 跳转页面/执行授权业务。 */ });
```

`messageActivated(id)` 表示已进入详情；`actionRequested(id)` 仅在详情动作按钮被点击时发出，不隐式执行任务、打开 URL 或删除消息。空 `actionText` 隐藏动作按钮。更新中的消息会刷新当前详情，删除当前消息后返回列表。面板不接管外部模型；模型销毁后恢复自己的空模型。

NotificationPopup 使用 Qt::Popup，点击外部或 Escape 关闭，关闭后可复用。`popup(anchor)` 根据锚点中心所在屏幕的 `availableGeometry` 放置，空间不足时改到上方并夹取位置；不是固定使用主屏坐标。空/隐藏锚点不弹出。列表的模型数量、字体与屏幕尺寸仍由应用合理配置。

[SG 消息面板预览](../images/notifications-sg-blue.png)

## 主窗口标题栏接入

```cpp
#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Inputs/NotificationBell.h>

auto* title = window->titleBar();
title->setNotificationsVisible(true); // 默认关闭，子屏/弹窗 profile 隐藏扩展项
// 标题栏拥有按钮，业务模型归应用持有。
title->notificationButton()->setModel(messages);
connect(title, &basewidgets::CustomTitleBar::notificationsRequested,
        popup, [title, popup] {
    QWidget* anchor = title->notificationButton()->isVisible()
        ? static_cast<QWidget*>(title->notificationButton()) : title;
    popup->popup(anchor);
});
```

窄窗口把消息入口放入“更多”菜单，标签保留未读数量，并发送同一 `notificationsRequested` 信号。标题仍按既有左右/居中规则布局，铃铛区域不参与拖动。详情见[标题栏](title-bar.md)。

## 配置关于页

AboutWidget 可直接加入设置页；AboutDialog 是相同内容的弹窗包装，默认只有关闭操作。底部“关闭”按钮、标题栏 ×、Esc 和代码 `close()/reject()` 均可关闭；点击窗口外不会自动关闭。底部按钮产生 Accepted，其余产生 Rejected，均发送 finished；设置 `WA_DeleteOnClose` 时关闭后销毁。没有隐式网络、更新检查或浏览器跳转。

```cpp
#include <basewidgets/Dialogs/AboutDialog.h>

basewidgets::AboutInfo info;
info.productName = "水声数据管理中心";
info.version = "2.1.0";
info.tagline = "采集 · 分析 · 存储管理";
info.build = "release-2026.10";
info.organization = "应用所属组织";
info.description = "在这里填写产品说明。";
info.copyright = "在这里填写实际版权信息。";
info.license = "在这里填写实际许可与第三方声明。";
info.details = {{"部署环境", "内网"}};
info.links = {{"guide", "使用指南", QUrl("app:guide")}};
// info.logo = applicationIcon; // 空值沿用 QApplication::windowIcon()

auto* about = new basewidgets::AboutDialog(window);
about->setAttribute(Qt::WA_DeleteOnClose);
if (about->setInfo(info)) about->open();
connect(about->aboutWidget(), &basewidgets::AboutWidget::linkActivated,
        receiver, [](const QString& id, const QUrl& url) { /* 由应用选择打开方式。 */ });
```

产品名/版本/Logo 为空时沿用应用默认，其余空字段隐藏；全部文字作为纯文本，可选择复制。长内容使用纵向滚动，许可区可展开。“复制版本信息”只复制产品、版本和元信息，不复制许可全文或链接，可用 `setCopyButtonVisible(false)` 隐藏。链接 ID 必须非空且唯一、文字非空、URL 有效且非空，否则整个配置失败并保留旧页面。

### JSON 配置

`AboutWidget::setConfiguration` 和 `AboutDialog::setConfiguration` 接受同一完整快照。应用负责读取文件、解析 QJsonDocument，然后传 `document.object()`；组件不监视文件或定时重载。

```json
{
  "productName": "水声数据管理中心",
  "version": "2.1.0",
  "tagline": "采集 · 分析 · 存储管理",
  "organization": "应用所属组织",
  "build": "release-2026.10",
  "description": "在这里填写产品说明。",
  "copyright": "在这里填写版权信息。",
  "license": "在这里填写许可说明。",
  "copyButtonVisible": true,
  "details": [{"label": "部署环境", "value": "内网"}],
  "links": [{"id": "guide", "text": "使用指南", "url": "app:guide"}]
}
```

可选 `logo` 是本地或 qrc 路径，空值沿用应用图标；不是远程图片 URL。相对路径相对于进程工作目录，应用也可先转为绝对路径。字符串字段类型错误、Logo 不存在或链接无效时保持旧配置。缺项恢复默认而不是增量合并；`copyButtonVisible` 缺省 true。C++ 的 `setInfo` 保持现有复制按钮偏好，JSON 则一并配置它。

[SG 关于窗口预览](../images/about-sg-blue.png)

## Demo 与验证

主标题栏和“常用组件 → 消息铃铛与面板”共享同一示例模型；模拟消息、查看详情、全部已读都会同步角标。标题栏详情的动作可跳到录音或监控页。“帮助 → 关于组件示例”使用通用 AboutDialog；“常用组件 → 可配置关于页面”可以修改名称/版本后预览。

回归覆盖模型边界与角色、未读共享、筛选/详情/生命周期、弹窗屏幕边界、标题栏溢出菜单、JSON 原子验证、纯文本/Logo/链接/复制及四主题宽窄渲染。执行方式与平台边界见[验证指南](../development/validation.md)。
