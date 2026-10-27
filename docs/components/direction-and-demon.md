# 多波束、测向历程与 LOFAR/DEMON

[文档首页](../README.md)

公共头文件：`Charts/BeamPlots.h`、`Charts/LofarDemonPlots.h`、`Charts/HistoryHeatmapPlot.h` 与 `Acoustics/DemonAnalysis.h`。先阅读[单位与校准约定](acoustic-analysis.md#单位与校准)；菜单、选区和视口联动共用[图表交互接口](chart-interaction.md)。

## 控件与计算工具

| 类 | 作用 |
|---|---|
| `BeamPatternPlot` | 多束方位响应，按 ID 更新/移除/显隐，游标与拾取 |
| `BearingHistoryPlot` | 方位–时间–响应历程，固定色标和显示门限 |
| `LofarPlot` | 原始声学 PSD 的窄带频率历程 |
| `DemonSpectrumPlot` | 归一化包络调制谱及峰值保持 |
| `DemonHistoryPlot` | 与载频带/抽取参数绑定的调制频率历程 |
| `HistoryHeatmapPlot` | 上述历程共用的均匀横轴、有界环形历史与缺帧显示基础 |
| `DemonAnalysis` | 可在工作线程运行的有限块参考计算 |

## 多波束与测向历程

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

## LOFAR 与 DEMON

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
[hilbert](https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.hilbert.html)
实现没有依赖 SciPy，使用 Qt 容器与 C++ FFT。

## 示例、线程与验证

`examples/widgets_demo/pages/DirectionAnalysisPage.cpp` 演示三束响应、方位历程、窄带 LOFAR、32/64 Hz AM 调制，提供色标、门限与业务菜单扩展。所有数值是合成数据，页面隐藏时暂停模拟。

DemonAnalysis 可在工作线程计算，图表更新必须切回 GUI 线程。Qt5 queued 信号接线前按实际传输类型注册 `DemonSettings`、`DemonResult`、`AcousticSpectrumFrame`；菜单扩展必须在 GUI 线程直接连接。

检查涵盖原始值不受显示门限影响、多束显隐/拾取、已知调制频率与抗混叠；具体命令和验收边界见[验证指南](../development/validation.md)。
