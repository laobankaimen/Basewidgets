# 图表通用交互

[文档首页](../README.md)

公共头文件：`Charts/StyledPlot.h`。所有业务图表共用此交互基础，在 GUI 线程调用。

## 导航、复位与流式跟随

默认左键平移、滚轮缩放；Shift+左键拖拽临时框选放大，双击或 Home 恢复。
右键可切换平移、框选缩放、框选区域、坐标拾取。Esc 取消当前框选。

- `setNavigationAxes(Qt::Horizontal | Qt::Vertical)` 指定平移、滚轮和框选缩放的轴。
- 波形默认只导航时间轴，其他图表默认双轴；业务可自行修改。
- `setResetRanges(x,y)` 只更新复位基准，不改变当前视口；通用 StyledPlot 在设置初始数据后也可 `captureResetView()`。
- `setViewRanges(x,y)` 用于外部联动，不修改复位基准。若联动来自人工导航，可先 `notifyUserNavigation()` 暂停目标图跟随。
- `coordinateClicked / coordinateHovered` 给出轴坐标；`selectionMade` 返回排序后的选区上下界。
- `viewRangesChanged` 支持同步视口；不同物理单位的横轴不要直接连在一起。
- `userNavigationStarted / viewReset / interactionModeChanged` 可用于同步工具栏和业务控制状态。
- 流式图表手动导航后暂停跟随，新数据不会抢回视口；复位回到最新缓存并恢复跟随。
- 暂停跟随不停止数据接收，也不永久保留超过容量的旧数据。长历史回放应由业务数据源重新加载。

### 扩展右键菜单

```cpp
connect(plot, &basewidgets::StyledPlot::prepareContextMenu, this,
        [this](QMenu* menu, const QPointF& at) {
    auto* action = menu->addAction(tr("处理此位置"));
    // at 是值，可安全复制；menu 只在本次弹出期间有效，不要缓存它。
    connect(action, &QAction::triggered, this, [this, at] {
        requestBusinessProcessing(at.x(), at.y());
    });
}, Qt::DirectConnection);
```

`prepareContextMenu` 必须在 GUI 线程直接连接。菜单非阻塞弹出，关闭时销毁，图表被删除时也会安全关闭。
子类可覆写 `extendContextMenu` 增加固定功能；公开 `createContextMenu` 返回的菜单由调用者负责生命周期。
`saveSnapshot(path,width,height,scale)` 先同步待绘制历史，再同步导出 PNG，返回保存是否成功。

### 选区与联动

```cpp
plot->setInteractionMode(basewidgets::StyledPlot::InteractionMode::RectangleSelect);
connect(plot, &basewidgets::StyledPlot::selectionMade, this,
        [this](double x0, double x1, double y0, double y1) {
    // 对 LOFAR：x 为 Hz，y 为秒；对测向历程：x 为度，y 为秒。
    requestAnalysisForRegion(x0, x1, y0, y1);
});
```

轴范围互相连接时请加业务侧重入保护，并确保时间基准一致。这里只发出选择/定位信号，不自动启动分析、读取文件或控制设备。

## 通用曲线与主题

通用曲线使用 `StyledPlot::addGraph()`，柱状图直接使用 `QCPBars`。StyledPlot 统一处理标题、坐标轴、图例与色标的主题，默认 queued replot 合并重绘。
业务图表中的内部 graph/item 由控件持有，不要用继承的 removeGraph/removeItem 替换它们；需要完全自定义画布时使用 StyledPlot。

## 业务图表入口

- [波形、频谱、时频与录音时间线](acoustic-analysis.md)
- [波束、测向、LOFAR 与 DEMON](direction-and-demon.md)
- [容量、速率与数据管理仪表](monitoring.md)

菜单、鼠标框选与流式视口检查见[验证指南](../development/validation.md)。
