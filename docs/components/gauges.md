# 通用数值、航向与姿态仪表

[文档首页](../README.md)

`CircularGauge`、`CompassGauge`、`AttitudeMeter` 直接使用 QPainter，统一由 ThemeTokens 适配明暗主题。它们只展示调用方数值，不创建内部采样/动画定时器。

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

## 转速仪表示例

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

## 设计与示例

弧形适合完成量/单指标概览，指针适合量程内读数。阈值由业务显式提供；完成量默认使用主色，高完成率不自动表示危险。目标、阈值和当前值分别绘制，避免相互覆盖。

信息层级参考资料（没有引入对应运行库）：

- [Grafana Gauge](https://grafana.com/docs/grafana/latest/panels-visualizations/visualizations/gauge/)
- [Highcharts Solid Gauge](https://www.highcharts.com/demo/gauges/solid)
- [Highcharts 示例源](https://github.com/highcharts/highcharts/blob/master/samples/highcharts/demo/gauge-solid/demo.js)

`examples/widgets_demo/pages/GaugesPage.cpp` 展示完成量/目标、温度阈值、转速、航向、姿态、无数据与禁用状态。自动演示默认关闭，隐藏时停计时器，显示时遵从用户开关。

容量、网速和业务进度类仪表见[数据管理仪表](monitoring.md)。
