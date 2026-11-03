# 数据管理与采集监控仪表

[文档首页](../README.md)

下列代码为接线片段，假定已创建控件，并由业务提供 parent、时间戳与快照变量。

这些控件展示业务提供的容量、速率、任务和核验快照，不访问本机磁盘、网卡或数据库，也不自动执行任务。仪表使用 QPainter/ThemeTokens，历程图使用 QCustomPlot。所有 QWidget 更新在 GUI 线程调用。

| 类 | 公开头文件 | 业务含义 |
|---|---|---|
| DashboardPanel | `Containers/DashboardPanel.h` | 渐变面板、纯文本标题及内容布局 |
| StorageDonutMeter | `Gauges/StorageDonutMeter.h` | 已用 / 总量的整环容量占比，IEC 或 SI 单位 |
| StorageCapacityMeter | `Gauges/StorageCapacityMeter.h` | 容量分类、可用量与警戒线 |
| TransferRateMeter | `Gauges/TransferRateMeter.h` | 接收/发送速率与累计计数基准 |
| RateHistoryPlot | `Charts/RateHistoryPlot.h` | 网络、磁盘、索引等指标历程 |
| TaskQueueDashboard | `Gauges/DataManagementDashboards.h` | 各处理阶段的任务状态 |
| DataQualityDashboard | `Gauges/DataManagementDashboards.h` | 核验覆盖率与通过率 |
| AcquisitionStatusPanel | `Gauges/AcquisitionStatusPanel.h` | 采集状态、丢样率与剩余可录时长 |

## 数据管理首页与面板

`widgets_demo --theme sg-blue --dashboard` 打开“数据首页”，参考 SG 首页的左侧导航、四张指标卡、丢包率图、双存储集群、数据类型分布及任务/最近操作分区。宽度不足时，卡片和图表会逐级减少列数，内容纵向滚动；不靠缩小文字适配。

`DashboardPanel` 提供轻量渐变背景、分隔线和内容布局，默认无整框鼠标悬停效果。示例中的统计卡由该公开容器、标签与 SVG 图标组合，没有使用整图背景，也没有要求独立图片素材。所有数据均为合成快照；左侧导航、查看记录和图表扩展菜单已接入原有 demo 页面。丢包率仍继承 `StyledPlot` 的右键扩展、Shift 框选和双击/Home 复位。

```cpp
#include <basewidgets/Containers/DashboardPanel.h>
#include <basewidgets/Gauges/StorageDonutMeter.h>
#include <QVBoxLayout>

// parent 由调用者提供。布局拥有加入的子控件，contentLayout() 返回借用指针。
auto* panel = new basewidgets::DashboardPanel("实时存储集群", parent);
auto* ring = new basewidgets::StorageDonutMeter;
ring->setUnitSystem(basewidgets::StorageDonutMeter::UnitSystem::SI);
ring->setCapacity(200000000000000ULL, 2000000000000000ULL); // 200 TB / 2 PB = 10%
panel->contentLayout()->addWidget(ring);
```

## StorageDonutMeter

- `setCapacity(usedBytes, totalBytes)` 输入字节，先已用、后总量；与分类条形表的接口顺序不同。`used > total` 返回 false，完整保留上一快照。
- `(0,0)` 表示未知，显示破折号；`capacityAvailable()` 为 false，`percent()` 返回 NaN。`(0,正数)` 是有效的 0%。
- 默认 IEC（1024，KiB/MiB/TiB）；SI 使用 1000（kB/MB/TB）。单位切换只改变格式，不修改原始字节值；`usedText()/totalText()` 可供外部标签复用。
- 最大支持 `quint64`，比值先转换再计算，不做可能溢出的整数乘法。控件没有隐式报警、动画、轮询或业务写入。
- 首页“数据类型分布”的百分比以现有数据总量为分母，三类合计 100% 不表示存储空间已满；磁盘容量占比由上方容量环单独表达。

## StorageCapacityMeter

输入总容量和具名分类字节数，最多 16 类，名称唯一；超总量或累加溢出被拒绝。
剩余量按 total-used 计算；`total=0` 且分类为空表示未知；`total>0` 且分类为空表示全部容量可用。
默认警戒线 90%，可修改；点击色块发出 `segmentSelected(index,name,bytes)`，空闲区 index=-1。

```cpp
meter->setCapacity(totalBytes, {{"原始录音", rawBytes, {}},
                               {"处理结果", resultBytes, {}},
                               {"索引", indexBytes, {}}});
```

## TransferRateMeter

累计计数接口接受字节数和同一基准的秒时间戳：

```cpp
meter->setLinkCapacity(1e9); // 每方向 1 Gbit/s，仅为显式配置，不是自动测得的链路能力
meter->addCounters(t0, rx0, tx0); // 首帧基准，没有速率
meter->addCounters(t1, rx1, tx1); // (counter1-counter0)/(t1-t0)
```

- 首帧、计数器下降/回绕和超过 maximumSampleGap（默认 30 秒）都会重建基准，不显示负值或尖峰。
- 不猜测计数器位宽；重新建立基准后需下一帧才能得到有效速率。
- 数据源只有直接速率时可 `setRates(time,rxBytesPerSecond,txBytesPerSecond)`；切换回计数模式会重新取基准。
- `ratesUpdated` 始终输出 bytes/s。Mbps = bytes/s × 8 / 1e6；MiB/s = bytes/s / 1048576。
- 配置链路容量为每方向容量，接收/发送分别显示占用比例，不将全双工两个方向相加。
- 计时和过期判定由业务负责；没有新数据时可 `setStale(true)`，控件不隐式读取系统时钟/网卡。

## RateHistoryPlot

适合网络吞吐、磁盘读写、索引文件数/s 等不等间隔指标。一个图内所有曲线使用同一单位。
时间戳严格递增、容量 2–8192 点/曲线、最多 16 曲线；NaN 或超 gapSeconds 的间隔显示断线。
曲线单位不会自动随速率仪表的显示单位改变，业务接线时需显式换算。

```cpp
plot->configure(300, 60, "吞吐 (MiB/s)");
plot->addSeries("接收"); plot->addSeries("发送");
connect(meter, &basewidgets::TransferRateMeter::ratesUpdated, plot,
        [plot](double t,double rx,double tx) {
    plot->appendValues(t, {rx/1048576., tx/1048576.});
});
```

## TaskQueueDashboard

```cpp
#include <basewidgets/Gauges/DataManagementDashboards.h>
using namespace basewidgets;

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

## DataQualityDashboard

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

## 采集状态与剩余时长

`AcquisitionStatusPanel::setStatus` 接收 AcquisitionStatus，状态为 Idle/Recording/Paused/Disconnected。无效快照整体拒绝。

- `receivedSamples` 不包含 `lostSamples`，丢样率为 `lost / (received + lost)`；没有计数时返回 NaN。
- 累计计数的统计口径由业务统一，例如所有通道样点总数，不混用帧数和样点数。
- `totalBytes=0` 表示容量未知；预计剩余时间为 `freeBytes / writeBytesPerSecond`，输入实际写入 B/s，不按采样率猜测压缩码率。
- 容量或速率未知时返回 NaN，不显示无穷或伪造可录时间；额外预留空间策略由业务提供。

ADC RMS/Peak、DC、峰均比和削波使用 `SignalQualityMeter`，详见[水声数据约定](acoustic-analysis.md)。

## 示例

`examples/widgets_demo/pages/DataManagementPage.cpp` 演示容量/网速、任务队列、数据质量和三类速率历程；`AcousticPage.cpp` 演示采集状态。示例均为模拟快照。

跨线程按需注册 StorageSegment、AcquisitionStatus、DataQualitySnapshot 等值类型，不从工作线程调用仪表。
