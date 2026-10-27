# 水声处理与数据管理控件（2026-10-27）

## 本轮范围

新增四种 QCustomPlot 业务图表、两种原生仪表和一个可在线程中使用的计算工具。
底层仍为仓库内的 QCustomPlot 2.1.1，没有引入 WebView、QML、额外 FFT 依赖或 QSS。
未接入真实采集卡/数据库/文件格式解析；demo 的信号和记录元数据全部为可复现的模拟输入。

| 公开类 | 用途 | 主要 API |
|---|---|---|
| AcousticWaveformPlot | 多通道波形、缺口、双游标 | configure / addChannel / appendSamples / setCursors / setFollowLatest |
| AcousticSpectrumPlot | 单边频谱或 PSD、峰值保持、频带积分 | setFrame / setLogFrequency / setPeakHoldEnabled / bandPowerDb |
| AcousticSpectrogramPlot | 有界滚动时频图、缺帧空行、固定色标 | configure / appendFrame / setLevelRange / cellSelected |
| RecordingTimelinePlot | 多通道录音片段、状态、选中记录 | setRecords / setUtcTimeAxis / selectRecord / recordSelected |
| SignalQualityMeter | ADC RMS/Peak、DC、峰均比、削波、缺样 | setSamples / setStatistics / setDisplayRange |
| AcquisitionStatusPanel | 采集状态、样点丢失、存储余量、预计可录时长 | setStatus / lossPercent / remainingSeconds |
| AcousticAnalysis | 单帧 FFT、统计、线性校准及 PSD 积分 | spectrum / statistics / scaleToMicroPascals / bandPowerDb |

## 单位必须由业务明确提供

- `AcousticUnit::FullScale`：归一化采集值，参考幅值为 1。这里的 RMS dBFS 定义为 `20 log10(rms/1)`；满幅正弦为 -3.0103 dBFS，绝不将它自动标记为水声 SPL。
- `AcousticUnit::Volt`：样本实际单位为 V。功率谱参考 1 V²，PSD 参考 1 V²/Hz。
- `AcousticUnit::MicroPascal`：样本实际单位为 μPa。功率谱读数等价于该频点 RMS 相对 1 μPa；PSD 参考 1 μPa²/Hz。**声明单位不执行校准。**
- `scaleToMicroPascals` 只应用调用方给定的正线性倍率（μPa/输入单位）；不猜测水听器灵敏度、前置增益、频响或 ADC 位宽。
- 需要频率相关校准时，应在业务 DSP 层处理，再把正确单位的数据或频谱交给控件。
- `SignalQualityMeter` 专用于归一化 ADC 数据。整数 ADC 正负满幅端点不同的情况，可用自定义 `clippingLevel` 调用 `statistics`，再传入 `setStatistics`。

参考定义与实现对照（未引入这些库为运行依赖）：

- NOAA 海洋声学资料中的水下声压参考值：
  https://www.fisheries.noaa.gov/new-england-mid-atlantic/science-data/understanding-sound-ocean
- SciPy 官方 `periodogram` 中 spectrum/density 与单边谱定义：
  https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.periodogram.html

## 频谱与统计约定

`AcousticAnalysis::spectrum` 使用迭代 radix-2 FFT，输入长度为 16–65536 内的 2 次幂。
支持 rectangular / 周期 Hann，可选去均值。不会隐式重采样、截断、填零或自动补缺样。

令窗后 FFT 为 X，采样率为 fs：

- PSD = `c × |X[k]|² / (fs × Σw²)`。
- Power = `c × |X[k]|² / (Σw)²`。
- DC/Nyquist 的 c=1，其余单边频点 c=2；频率为 `k × fs/N`。
- Power 适合读取对齐频点的正弦均方量；不能把 Hann 窗的 Power 各频点简单相加当作总均方。
- 时域统计 RMS 包含 DC，默认频谱先去均值。Hann 的全频带积分对应窗加权均方；非平稳短记录不应期待它严格等于未加窗时域 RMS²。
- `linearPower` 不施加显示下限；`levelsDb = max(floorDb, 10 log10(linearPower))`。
- `bandPowerDb` **只接受 PSD**，按频点中心落在闭区间内进行 `ΣPSD × Δf` 后再取 dB。
  这是离散频带积分，不是带部分 bin 加权的精确滤波器响应。
- `AcousticSpectrumFrame` 保存 fs、N、窗、去均值、单位和 scaling。接收端验证频率网格、数组长度和线性/dB 数据一致性。
- 频谱输入含 NaN/Inf 时返回 false，不改写已有 output。统计允许忽略这些值，并明确计入 invalidCount。
- 零信号统计的 RMS/Peak 是 -Inf，全部无效时为 NaN；只有图上显示时才采用下限/“—”等表示。
- 当前辅助计算不是 Welch、多窗、波束形成或滤波后 SNR 算法。已有算法可直接提供合法的 frame。

## 流式数据与内存

### 波形

- `configure(fs, capacity, unit)` 原子配置并清空历史，最多 16 通道，每通道容量 16–1048576 点。
- `appendSamples(channel, startSeconds, samples)` 显式提供块首时间，时间基准由应用统一。
- 重叠/倒序块、Inf、时间精度不足或超容量块被拒绝；NaN 样本保留为断线。
- 跨块时间缺口插入 NaN 断线点；容量包含这些断线点。不会将长缺口画成一条连接线。
- 跟随模式只调整横轴；滚轮/拖动停止跟随。游标模式左键依次放 A/B，关闭平移保留缩放。
- 自适应采样只用于 QCustomPlot 渲染，不对容量内原始数据做隐式重采样。

### 时频图

- `configure(prototype, historyFrames, framePeriod)` 固定时间间隔和谱配置，容量 2–2048 帧，总单元格不超过 2097152。
- `appendFrame(centerTime, frame)` 时间是窗中心时间，频谱设置必须一致；换通道也应清空/重新配置，避免混合不同来源。
- 非整数 hop 时间戳、重复/倒序帧被拒绝；整 hop 缺帧用透明空行表达。长时间间隙只保留容量内历史，不遍历无限空帧。
- 环形队列在 append 时只保存新行；`beforeReplot` 合并更新色图，多帧输入可共用一次 queued replot。
- 频率横轴为线性 Hz，纵轴为帧中心秒，最新帧在上方。色标固定，不按每一帧自动拉伸。
- 默认关闭插值，避免对缺口和离散频点制造平滑的假象。空行与低电平有明确区分。

### 记录与状态

- 时间线最多 10000 段/64 通道；记录 ID 唯一，区间严格递增。显示的空白就是没有记录。
- 可选择相对秒或 UTC epoch 秒；UTC 只改变刻度格式，不转换输入数据本身。
- 点击发出记录 ID，业务可据此关联文件、元数据列表或播放器；控件不会自动读文件或播放。
- 状态面板的 receivedSamples 不含 lostSamples，丢样率为 lost/(received+lost)。分母为 0 时显示未知。
- 存储容量为 0 表示未知；预计可录时间按 freeBytes/实际写入 B/s 估算，速率为 0 时不显示无穷或伪造的可录时间。
- 这些都是调用方提供的快照；面板不访问磁盘、不控制采集设备，也不推测压缩码率。

## 接入示例

以下为 API 示例，48000、FFT 长度等应改为真实数据元信息；`samples` 是业务提供的数据。

```cpp
using namespace basewidgets;
auto* wave = new AcousticWaveformPlot(parent);
wave->configure(48000, 96000, AcousticUnit::FullScale);
int ch = wave->addChannel("Hydrophone 1");
wave->appendSamples(ch, blockStartSeconds, samples);

// 建议在工作线程计算；函数不访问 QWidget/QApplication。
AcousticSpectrumFrame frame;
QString error;
if (AcousticAnalysis::spectrum(fftWindow, 48000, &frame,
        AcousticUnit::FullScale, SpectrumScaling::PowerSpectralDensity,
        AcousticWindow::Hann, true, -160, &error)) {
    // 这两句必须在 GUI 线程执行。
    spectrumPlot->setFrame(frame);
    // 只在首次输入或处理参数/通道改变时 configure，不能每帧都清空历史。
    if (spectrogramPlot->historyCapacity() == 0) {
        spectrogramPlot->configure(frame, 128, double(hopSamples)/48000);
        spectrogramPlot->setLevelRange(-120, -20);
    }
    spectrogramPlot->appendFrame(windowCenterSeconds, frame);
}
```

跨线程信号在启动前注册：

```cpp
qRegisterMetaType<basewidgets::AcousticSpectrumFrame>();
qRegisterMetaType<basewidgets::SignalStatistics>();
// 将业务 worker 的信号以 Qt::QueuedConnection 连接到图表/面板的更新 lambda。
// 不要从音频回调或 DSP 工作线程直接调用 QWidget/QCustomPlot。
```

如需压力单位：

```cpp
QVector<double> pressure;
// microPaPerUnit 必须来自你的校准资料，不在控件中设置假定的默认灵敏度。
if (AcousticAnalysis::scaleToMicroPascals(rawSamples, microPaPerUnit, &pressure)) {
    AcousticAnalysis::spectrum(pressure, sampleRate, &frame, AcousticUnit::MicroPascal);
    // PSD 单位为 dB re 1 μPa²/Hz；积分结果才是相应频带的均方声压级。
}
```

## 旧图表清理与迁移

删除 `QCPLineChart`、`QCPBarChart`、`QCPWaterfallChart` 三个薄封装：它们只是 plottable，
没有采样时序/滚动/单位语义，却会重复安装菜单和争用画布主题。

- 原始通用折线：使用 `StyledPlot::addGraph()`；柱状图可直接 `new QCPBars(plot->xAxis, plot->yAxis)`。
- 水声时域/谱/时频场景使用上述业务图表，不能把旧 QCPColorMap 演示视作 STFT。
- `StyledPlot` 统一处理标题、全部坐标轴、图例和色标的主题；重绘默认 queued 合并。
- 鼠标信号直接继承 QCustomPlot，不再重复转发同名信号。底层 QCustomPlot 及许可证未修改。
- 业务图表内部曲线/item 归控件持有；不要通过继承的 removeGraph/removeItem 替换其内部对象。
  需要自定义通用场景时直接使用 StyledPlot。
- 此次清理涉及公开头文件删除，下游需迁移后重新编译；安装使用干净前缀以避免残留旧头文件。

## Demo 与验证

`examples/widgets_demo/pages/AcousticPage.cpp` 集中演示：24 kHz 两通道、750 Hz 线谱、扫频、削波通道、
固定 PSD 色标、每 23 帧跳过一帧、录音元数据与存储状态。采样时钟来自样点索引，定时器只控制演示速度。
自动模拟默认关闭，页面隐藏时暂停，不会后台无限积累数据。

验证覆盖：已知幅度正弦的 Power/PSD、Hann/rectangular、DC/Nyquist、不同 bin 宽度下积分一致性、
线性校准的 dB 差、缺样/削波统计、缓存上限/缺口、时频环形滚动/缺帧 alpha、峰值保持、时间线 ID 与选择、
采集丢样率及剩余时间，以及明暗主题图表截图。

## 本轮验证结果

Linux / Qt 5.15：50 项检查在 1.0 与 1.5 倍缩放下通过；七页 × 明暗 × 宽窄窗口的 28 组渲染成功。
四类业务图表另导出明暗图集，已检查波形、PSD、扫频/缺帧声谱和带标签的录音时间线。
干净安装后的外部 consumer 可直接包含并使用新 API，旧三个薄封装头文件不再安装。

单独以 64 点非整周期混合信号，将 C++ radix-2 输出与独立 O(N²) 直接 DFT 对照，
覆盖 rectangular/Hann × Power/PSD 四种组合，最大绝对功率误差为 3.47×10⁻¹⁷。
本环境没有验证真实采集卡、真实录音格式或 Windows/Qt6；demo 不是实测水声数据。

## 后续交互与测向扩展

所有业务图表已统一升级菜单/框选/复位能力，时频缓存提取为 HistoryHeatmapPlot。
新增测向历程、LOFAR/DEMON 和容量/速率监控，详见 [扩展说明](advanced-acoustic-and-monitoring-controls.md)。
DEMON 输出新增 RelativeEnvelope 单位，不解释为 dBFS 或原始声压。
