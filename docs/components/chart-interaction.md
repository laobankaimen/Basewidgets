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

通用曲线使用 `StyledPlot::addGraph()`，柱状图直接使用 `QCPBars`。StyledPlot 统一处理标题、坐标轴、图例与色标的主题，默认按 30 FPS 合并流式数据更新和绘图区内的滚轮/拖动重绘。
业务图表中的内部 graph/item 由控件持有，不要用继承的 removeGraph/removeItem 替换它们；需要完全自定义画布时使用 StyledPlot。

## 大数据绘制与刷新调度

`StyledPlot::addGraph()` 创建公开的 `SampledGraph`（兼容 QCPGraph 接口）。波形、频谱、DEMON 谱、波束和速率曲线统一使用它，不需要各页面重新抽样。`data()` 仍持有容量内原始数据，显示抽样不改变 FFT、峰值保持、频带积分或原始数据导出。

- 只抽取当前可见区和两侧邻点，按横轴像素分桶；线性、对数、反向轴适用。每像素最多一个桶，总桶数最多 4096。
- 每桶保留首、尾、最小、最大样点，并按原始时间排序。使用极值保留冲击峰，不用均值抹平尖峰。
- NaN/非有限值作为断线处理；极密缺口保守断开，不跨缺口制造连接。每桶最多四个有限点和五个断线标记，输出量受像素宽度限制。
- 放大后重新从原始数据取样，恢复局部细节。散点同样抽样；`scatterSkip` 在抽样后进一步稀疏显示。`SampledGraph::displaySamples()` 可查看显示样点；它不是业务计算的输入。此类始终启用像素抽样，不依赖 QCPGraph 的 `adaptiveSampling` 开关。
- 启用 QCustomPlot 的快速线段路径，避免密集、非整数线宽折线进入 Qt 的昂贵整段描边路径。不改变用户设置的颜色与线宽，也不修改内置 QCustomPlot 源码。

```cpp
#include <basewidgets/Charts/StyledPlot.h>
#include <basewidgets/Charts/SampledGraph.h>

plot->setMaximumFrameRate(30); // 默认 30，允许 1..120；非法值返回 false
// 多通道/多窗口可以按实际预算设为 20；不影响输入的数据采样率。
plot->replot();               // 合并到下一显示帧，不为每次更新排队重绘
plot->replot(basewidgets::QCustomPlot::rpImmediateRefresh); // 仅同步截图/检查等需要立即完成时使用
```

常规更新应使用 `StyledPlot*` 接口；直接经 `QCustomPlot*` 调用其非虚 `replot`、自行创建普通 QCPGraph，或强制立即绘制，会绕过相应优化。轴刻度区、色标等 QCustomPlot 原有交互与窗口 resize/DPI 刷新仍可能触发立即绘制，`maximumFrameRate` 是常规数据/导航调度上限，不是所有 Qt 绘制事件的硬上限。

抽样仍需遍历可见的原始样点，数据输入量、通道数及可见图表数量应与设备预算匹配。大块 FFT/文件读取放工作线程，通过批量 queued 信号传到 GUI；不要逐样点发送 GUI 信号。绘制层节流不承担 DSP 或采集队列背压。

声谱/LOFAR/DEMON/测向历程采用有界显示网格和最大值聚合，见[时频图](acoustic-analysis.md#时频图)。

## 业务图表入口

- [波形、频谱、时频与录音时间线](acoustic-analysis.md)
- [波束、测向、LOFAR 与 DEMON](direction-and-demon.md)
- [容量、速率与数据管理仪表](monitoring.md)

## 主题序列色与业务颜色

波形、频谱、测向波束和速率历程的默认曲线使用主题 `seriesColor1` 至 `seriesColor6`，亮暗切换时实时更新。API 中显式传入有效 QColor 时保持业务颜色。

```cpp
// 将本图的 QCPGraph 绑定到第 1 个主题色，保留线型、线宽与已有数据。
plot->setSeriesColor(graph, 0);
// 固定为业务颜色；第三参数传 QColor() 可恢复主题跟随。
plot->setSeriesColor(graph, 0, QColor("#A84874"));
```

`paletteIndex` 从 0 开始，超出 5 循环取色；负数、空指针和其他图的曲线返回 false。直接 setPen 设置其他颜色会退出跟随。删除曲线后主题回调不会继续访问该对象。热力图色标由数据含义决定，保持现有 ColorPreset/门限 API。完整配置步骤见[主题指南](../guides/themes-and-resources.md#新增配色模式只改配置)。
