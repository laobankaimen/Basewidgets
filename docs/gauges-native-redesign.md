# 原生仪表盘重构（2026-10-27）

## 设计参考与取舍

参考的是信息组织和交互，不复制第三方实现或贴图：

- Grafana Gauge：量程、当前值和阈值分别配置；状态区间需有语义，而不是所有仪表默认绿黄红。
  https://grafana.com/docs/grafana/latest/panels-visualizations/visualizations/gauge/
- Highcharts Solid Gauge：简洁弧形、突出中央数字、减少装饰，适合进度和单指标概览。
  https://www.highcharts.com/demo/gauges/solid
  对照官方示例源：https://github.com/highcharts/highcharts/blob/master/samples/highcharts/demo/gauge-solid/demo.js

本项目用 QPainter 实现同类信息层级，保持 NativeStyle/ThemeTokens 的明暗语言。没有增加 WebView、QML、图片纹理或 QSS 依赖。完成量默认使用主色，不将高完成率误表达为危险；只有业务提供阈值时才显示相应区间和文字。

## 已确认的旧实现问题

- BasicGauge、SpeedGauge 和 CircularGauge 重复表达速度/标量；SpeedGauge 的多个配置接口是空实现。
- CircularGauge 的经典分支与普通分支采用不同角度公式，预设切换会残留分支状态。
- 零量程、次刻度数为 0、极小刻度步长存在除零或过量循环风险。
- 多个内联颜色 setter 不触发重绘；固定灰色、玻璃高光和小字使明暗主题不一致。
- 标题与数值重复，单位写到每个刻度，阈值环被当前值环覆盖，小尺寸文字重叠。
- Qc 组合层的样式与主 UI 不一致，还公开内部 item 列表与大量低层参数，维护成本高。

## 控件保留与删除

| 控件 | 处理 |
|---|---|
| CircularGauge | 重写为 Arc / Needle 两种模式，共用数据校验、角度映射与主题绘制 |
| CompassGauge | 保留名称，直接自绘；固定方位、航向指针、大号角度与方向文字 |
| AttitudeMeter | 保留名称，直接自绘；天空/地面、俯仰梯、固定机体符号及角度读数 |
| BasicGauge / SpeedGauge | 删除，统一由 CircularGauge::DisplayMode::Needle 替代 |
| QcGaugeWidget / QcItem 系列 | 删除旧组合引擎；上述控件不再依赖它 |

这是公开 API 清理，需要重新编译下游程序。没有保留“调用成功但不做事”的兼容占位。旧安装目录不会自动删除历史头文件，发布时使用干净的安装前缀。

## 数据及绘制约定

### CircularGauge

- `setRange(min,max)`：有限且严格递增，跨度必须可表示。无效设置返回 false 并保持原状态。
- `setValue`：有限值夹取到量程；NaN/Inf 切为无数据，最后有限值保留但不绘制读数指示。
- `setValueAvailable(false)`：显示“— / 无数据”，不伪装成 0；正常值更新恢复可用。
- `GaugeRange` 使用实际业务值；按起点排序、裁剪到量程。重叠或非有限区间整组拒绝；最多 64 段。
- 区间为 `[start,end)`，量程最大值属于最后一段。改变量程会重新裁剪已有区间。
- 阈值带在外圈，当前值轨道在内圈，两者不互相覆盖。状态用文字补充，不仅依赖颜色。
- 目标值为独立标记和“目标”文字；有限输入夹取，非有限输入清除。
- 数值从默认 225° 顺时针到 -45°，针、弧、刻度、目标共用 `angleForValue`。
- `setAngles(start,end)` 为原子设置，扫角为 30–330°；不能用闭合的 360° 混淆最小与最大。
- 小尺寸下采用数值/弧/量程摘要；正常尺寸保留主刻度并避免标签与中央读数碰撞。
- 刻度循环有上限；0 个次刻度合法。过密步长自动退化为均分，避免渲染线程卡死。
- 默认色来自 ThemeTokens，显式颜色传 QColor() 可恢复主题。字体使用逻辑像素，按空间拟合并省略过长文字。
- 不创建内部计时器，不对业务数据做平滑或预测；`valueChanged` 仅在真实有限值变化时发出。

### CompassGauge

- 北=0°、东=90°、顺时针增加；有限角度按 360° 归一化。
- 方位刻度固定，指针表示航向。数字按最近整数显示为 000°–359°，accessibleDescription 保留一位小数。
- `setValue(int)` 保留为 `setHeading(double)` 的兼容入口。无数据隐藏指针并显示“—”。

### AttitudeMeter

- pitch 正值抬头，地平线向下；限制为 [-90°,90°]。
- roll 正值右倾，归一化到 [-180°,180°)。`setAttitude` 原子更新两轴。
- 仅地平线/俯仰梯旋转，机体参考符号和外围刻度固定；蓝/棕为天空/地面语义色。
- 无数据不保留伪造的水平地平线。控件只显示输入姿态，不提供传感器融合。

## 迁移示例

```cpp
#include <basewidgets/Gauges/CircularGauge.h>
auto* gauge = new basewidgets::CircularGauge(parent);
gauge->setDisplayMode(basewidgets::CircularGauge::DisplayMode::Needle);
gauge->setTitle("电机转速");
gauge->setUnit("rpm");
gauge->setRange(0, 8000);
gauge->setTickCount(4);
gauge->setRanges({
    {0, 6000, QColor("#67C23A"), 5, "工作区间"},
    {6000, 7200, QColor("#E6A23C"), 5, "高转速"},
    {7200, 8000, QColor("#F56C6C"), 5, "超限"}
});
gauge->setTargetValue(6000);
gauge->setValue(3200);
```

旧 `setNeedleWrapRing`、`setNeedleWidth`、`setTickMajorWidth/Length`、`setTickMinorWidth/Length`、
`setMinorTicksAsDots`、`setMinorTickDotAtInner`、`setMinorTickAngleRange`、`setTickInset`、
`setTickLabelOffset`、`setTickLabelFontScale`、`setShowLabelInValue` 已移除，避免几何和文字布局互相冲突。
保留基本量程、数值、标题、单位、颜色、阈值、刻度数/步长，以及两个旧预设入口；预设不再绘制玻璃高光。

## Demo 与验证

示例位于 `examples/widgets_demo/pages/GaugesPage.cpp`，全部数字是演示值。

- 完成量/目标、温度阈值、转速指针、航向、姿态、无数据/禁用六种场景。
- 自动演示默认关闭，页面隐藏时停止计时器，回到页面时按用户勾选状态恢复。
- 接口边界、阈值交界、角度映射、非有限数、极小步长、预设切换及隐藏计时器行为纳入同一 demo 的 `--verify`。
- 图集覆盖明暗主题以及 300×300、180×210、420×240 三组尺寸，另验证 1.5 倍缩放。

```bash
cmake -S . -B cmake-build-debug -DUI_DEMO_CHECKS=ON
cmake --build cmake-build-debug --target widgets_demo --parallel 4
QT_QPA_PLATFORM=offscreen BW_NATIVE_CAPTURE_DIR=/tmp/gauge-captures \
  ./cmake-build-debug/bin/widgets_demo --verify
```

验证结果（Linux / Qt 5.15）：42 项检查在 1.0/1.5 倍缩放下均通过；主 demo 七页的明暗/宽窄窗口渲染通过。
六种仪表的明暗三尺寸图集已人工检查。使用干净安装目录确认旧头文件不再安装，并通过外部 CMake consumer 编译和运行。
Windows/Qt6 尚未在本环境构建验证，不据此声称全部平台已验收。
