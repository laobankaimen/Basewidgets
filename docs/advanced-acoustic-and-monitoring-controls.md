# 测向、LOFAR/DEMON 与数据监控控件

## 1. 控件清单

| 类 / 头文件 | 作用 |
|---|---|
| `StyledPlot` | 所有业务图表的菜单、缩放、拾取、选区、复位、联动和导出基础 |
| `HistoryHeatmapPlot` | 均匀横轴 × 时间的有界环形历程，固定色标、显示门限、缺帧透明 |
| `BeamPatternPlot` / `Charts/BeamPlots.h` | 多束方位响应曲线，按 ID 更新/移除/显隐，图例点击、方位游标和拾取 |
| `BearingHistoryPlot` / `Charts/BeamPlots.h` | 方位–时间–响应值历程，支持色标范围、显示门限和灰度/热力色阶 |
| `LofarPlot` / `Charts/LofarDemonPlots.h` | 原始声学信号窄带 PSD 历程，显式配置显示频带 |
| `DemonSpectrumPlot` / `Charts/LofarDemonPlots.h` | 归一化包络调制谱、峰值保持与频带显示 |
| `DemonHistoryPlot` / `Charts/LofarDemonPlots.h` | 与解调设置绑定的调制频率历程，拒绝混合不同载频带/抽取参数 |
| `DemonAnalysis` / `Acoustics/DemonAnalysis.h` | 单块参考 DEMON 流程，可在工作线程运行 |
| `RateHistoryPlot` | 不等间隔的速率/处理指标历程，缓存上限、缺口和暂停跟随 |
| `StorageCapacityMeter` | 存储分类/剩余容量、警戒线与色块点击 |
| `TransferRateMeter` | 直接速率或累计字节计数计算 RX/TX，重置/过期状态及 Mbps/MiB/s 显示 |

现有波形、频谱、声谱和录音时间线均继承共用交互；无需分别安装右键菜单处理器。
图表保持 QCustomPlot 2.1.1，容量/速率指示采用 QPainter。底层 QCustomPlot 源码没有修改。

## 2. 统一交互

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

## 3. 多波束与测向历程

`BeamPatternPlot` 接收业务已经算好的响应，不实现阵列波束形成：

```cpp
beamPlot->setBearingRange(0, 360);
beamPlot->setLevelRange(-60, 0);
beamPlot->setLevelUnit("归一化波束响应 (dB)");
beamPlot->setBeam("array-A", "阵列 A", angles, responseA);
beamPlot->setBeam("array-B", "阵列 B", angles, responseB);
```

最多 32 束，每束 2–16384 点；角度严格递增，NaN 响应断线。默认角度文字为北=0°、顺时针，
若业务采用正侧视等约定，应修改 `xAxis->setLabel(...)` 并保持所有曲线一致，控件不暗中旋转数据。
修改角度范围前先清空已有曲线，避免旧数据被重新解释。

```cpp
history->configure(0, 360, 361, 256, 0.2, "波束响应 (dB)");
history->setLevelRange(-55, -5);        // 颜色映射上下限
history->setDisplayThreshold(-30);     // 隐藏低于 -30 的值
history->appendFrame(frameCenterTime, angularResponse);
```

色标和门限均可通过右键菜单或外部 UI 调整。`levelAt` 保持原始值；门限不是检测判决，也不做目标跟踪。
`cellSelected/bearingSelected` 的 available 表示原始单元格有数据，不表示它一定通过显示门限。
缺帧/NaN 与被门限隐藏的数据都透明，但可通过原始数据和门限状态区分。

## 4. LOFAR 与 DEMON 的边界

### LOFAR

接受未解调的声学 PSD 帧，显示指定窄带频率范围。与普通 STFT 共用频谱计算，但有明确的 PSD 和原始声学单位约束；不接受归一化包络帧。
颜色、门限、时间缓存与交互来自 HistoryHeatmapPlot。

### DEMON

参考实现执行：

1. 对有限样本块 FFT，保留指定正频载频带并加倍，负频置零，逆变换得到解析信号。
2. 取包络幅度 `abs(analytic)`，按块均值归一化后减 1。
3. 对归一化包络低通，去除超过 maximumModulationHz 的分量，然后按 decimation 抽取。
4. 对抽取后的包络波动计算窗归一化单边 PSD。

输出单位是 **包络 PSD（dB re 1/Hz）**，不是 dBFS、声压级或载频谱。
归一化去掉了载频带整体增益，不能再用这个结果推断原始声压幅值。

```cpp
basewidgets::DemonSettings settings;
settings.carrierLowerHz = 1500;
settings.carrierUpperHz = 2600;
settings.maximumModulationHz = 100;
settings.decimation = 16;
basewidgets::DemonResult result;
QString error;
// samples 示例长度 4096，fs 示例 8192 Hz；设备参数由业务实际提供。
if (basewidgets::DemonAnalysis::analyze(samples, fs, settings, &result, &error)) {
    // 切回 GUI 线程后更新显示。
    spectrum->setResult(result);
    // 第一次/参数改变时才 configure，不能每帧清历史。
    if (!history->historyCapacity()) history->configure(result, 128, hopSeconds);
    history->appendResult(centerTime, result);
}
```

限制和语义：

- 输入块 256–65536 点、长度为 2 的幂；抽取率也是 2 的幂（1–256），输出至少 16 点。
- 调制上限至少覆盖一个可分辨频点，且不高于输出 Nyquist 的 80%；先低通再抽取，避免高频包络折叠。
- 带内无能量或只剩相对输入 RMS 低于 1e-12 的数值残差时拒绝归一化；这个保护不是业务检测阈值。
- 当前是有限块周期延拓的频域带通/低通，端点可能产生边缘效应，**不是有状态的流式 FIR/IIR**。
- 生产环境可用已有滤波/包络/平均后端提供合法 DemonResult；必须保持单位、频率网格和设置元信息一致。只提供谱时 normalizedEnvelope 可留空，meanEnvelope 未知时保持 NaN，不填造假的均值。
- 载频带、抽取率、调制上限、输入采样率/块长改变时，历程需要重新 configure；不会混合不同处理条件。
- 不自动推断轴频、叶片数、转速、目标类别或跟踪关联。

解析包络的定义可对照 SciPy 官方 Hilbert 文档：
https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.hilbert.html
实现没有依赖 SciPy，使用 Qt 容器与 C++ FFT。

## 5. 容量、吞吐与网速

### StorageCapacityMeter

输入总容量和具名分类字节数，最多 16 类，名称唯一；超总量或累加溢出被拒绝。
剩余量按 total-used 计算；total=0/空分类代表未知，不当作一块空磁盘。
默认警戒线 90%，可修改；点击色块发出 `segmentSelected(index,name,bytes)`，空闲区 index=-1。

```cpp
meter->setCapacity(totalBytes, {{"原始录音", rawBytes, {}},
                               {"处理结果", resultBytes, {}},
                               {"索引", indexBytes, {}}});
```

### TransferRateMeter

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

### RateHistoryPlot

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

## 6. Demo、线程和验证

- `DirectionAnalysisPage.cpp`：合成三束响应、方位历程、窄带 LOFAR、32/64 Hz AM 调制示例，带色标/门限控制与业务菜单扩展。
- `DataManagementPage.cpp`：模拟容量分类、网络计数、磁盘 I/O 和索引处理速率，可模拟计数器重置并点击容量分类。
- 原有时域/频域/声谱/录音时间线保留，并升级共用交互；两个新页面的自动演示默认关闭，隐藏时暂停计时器。
- 计算工具可在工作线程运行；所有 QWidget/QCustomPlot 方法在 GUI 线程调用。跨线程传输前注册 DemonResult/AcousticSpectrumFrame 元类型。
- 该 demo 不读取本机设备和数据，也不发送网络请求；所有数值均为合成示例。

验证包括：实际鼠标框选/Shift/双击/Home/Esc、菜单扩展/生命周期、原始值不受门限影响、手动视口不被流式刷新抢回、
多束拾取/显隐、已知调制频率与抗混叠、容量溢出、计数器重置、速率缓存上限，以及明暗主题图集和高 DPI。

跨线程结构还提供了 `DemonSettings`、`StorageSegment`/其 QVector、`AcquisitionStatus`、
`RecordingInterval`/其 QVector 的 Q_DECLARE_METATYPE；Qt5 的 queued 信号接线前请按实际类型调用 qRegisterMetaType。
容量/速率等图表控件本身仍只能在 GUI 线程操作，菜单扩展信号不应 queued。

## 7. 本轮验证结果（Linux / Qt 5.15）

- 59 项检查在正常与 1.5 倍缩放下通过，包含真实鼠标事件的框选/复位/菜单生命周期。
- 已知 32 Hz AM 调制峰位置正确，包络幅度归一化保持增益不变性，320 Hz 分量未在抽取后折叠到 192 Hz。
- 9 页 × 明暗主题 × 2 种窗口宽度的 36 组 smoke 渲染通过，另检查了新图表和指示控件的明暗图集。
- 干净安装后的外部 CMake consumer 完成编译/运行，能使用新图表、DEMON、容量/速率 API 和元类型注册。
- 本轮没有接真实阵列、设备计数器或生产滤波后端；Windows/Qt6 尚未在此环境实测。
