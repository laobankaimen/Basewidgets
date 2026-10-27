# 原生绘制迁移历史

[文档首页](../README.md)

这是 2026-10-26 至 2026-10-27 的方案修正与阶段记录，不是当前使用指南。当前架构见[架构约束](../development/architecture.md)，接口升级见[迁移指南](../development/migration.md)，功能验收只维护在[验证指南](../development/validation.md)。

## 方案修正依据

| 初始思路 | 审查发现 | 最终方向 |
|---|---|---|
| 薄包装控件各自重写 paintEvent | 尺寸、命中、菜单、RTL、快捷键仍依赖 QStyle | 共享 NativeStyle，保留 Qt 交互 |
| 自绘与应用级 QSS 并存 | 背景、padding、子控件几何会由两套机制接管 | 删除运行时 QSS 生成与应用 |
| ToolButton 直接复用 PushButton | autoRaise、分离菜单、箭头/标签布局不同 | 共用面板，分别处理工具按钮几何 |
| 删除焦点外框 | 键盘导航丢失反馈 | 内部焦点线/halo，保留键盘可达性 |
| QPainter 必然更快 | 无法据此量化收益或解释透明度/字体问题 | 只报告已移除的工作，性能单独测量 |
| 第三方示例可证明跨平台效果 | 不能替代本库实际控件与桌面验收 | demo 检查、图集与实机分开 |

## 实现演进

1. NativeStyle/ThemeTokens 建立统一绘制、尺寸、命中与调色板；删除 CSS 模板解析、缓存和导出，保留部分主题兼容入口。
2. 资源工具归入主 CMake 构建，主题源归入 assets；生成器不链接 UI 库，资源随库内嵌。
3. Core 的标题/菜单/工具栏/状态栏职责整理；普通窗口位置保存交给应用，布局加入字体度量和响应式卡片。
4. 按钮视觉层级与弹窗边界调整；默认关闭 Tab hover；Wayland 拖动释放丢失做 Qt 层恢复，未宣称实机问题已完全验证。
5. 删除重复/空实现仪表和旧 Qc 引擎，CircularGauge 统一 Arc/Needle，航向与姿态直接绘制。
6. 新增水声波形、谱、时频、时间线，随后扩展共用交互、多波束、LOFAR/DEMON 和数据监控。
7. 高级表格统一模式、固定源记录、菜单、CSV 和跨页勾选；位置恢复与业务多屏职责逐步分开。
8. 标题栏加入 profile、独立标题/图标、搜索/账户/动作；最终加入 ScreenWindow/MultiScreenWorkspace，主控与子屏的界面和生命周期独立。
9. 标题功能图标归入 SVG 资源，删除重复 DPR 换算和工具按钮预栅格化放大，调整选择指示器比例。

## 历史验证记录

以下计数属于当时阶段，不应引用为当前全库验收结果：

| 阶段 | 当时记录 |
|---|---|
| 初始原生绘制 | 31 项；7 页 × 两主题 × 两宽度 |
| 按钮与资源整合 | 33 项 |
| 弹窗与拖动恢复 | 37 项 |
| 通用仪表 | 42 项 |
| 水声基础控件 | 50 项 |
| 测向、DEMON 与监控 | 59 项；demo 扩为九页 |
| 高级表格/多屏几何/业务仪表 | 68 项 |
| 表格勾选 | 70 项 |
| 标题栏扩展 | 75 项 |

初始阶段记录过 AddressSanitizer 未报告越界/释放后访问，LeakSanitizer 未开启；不据此推断后续全部代码或内存泄漏已经验证。水声阶段曾用 64 点混合信号的独立 O(N²) DFT 对照 radix-2 结果，四种窗/scaling 组合最大绝对功率误差记录为 3.47×10⁻¹⁷；这不替代生产算法验证。

## 旧文档归并位置

| 旧文件（已归并） | 当前内容位置 |
|---|---|
| `DEVELOPMENT.md` | [入门](../guides/getting-started.md)、[构建发布](../development/cmake-and-packaging.md)、[基础控件](../components/basic-widgets.md)、[架构](../development/architecture.md) |
| `qpainter-native-style-plan.md` | [架构](../development/architecture.md)、[迁移](../development/migration.md)、本文 |
| `gauges-native-redesign.md` | [通用仪表](../components/gauges.md)、[迁移](../development/migration.md) |
| `underwater-acoustics-controls.md` | [水声分析](../components/acoustic-analysis.md)、[图表交互](../components/chart-interaction.md) |
| `advanced-acoustic-and-monitoring-controls.md` | [测向/DEMON](../components/direction-and-demon.md)、[监控仪表](../components/monitoring.md)、[图表交互](../components/chart-interaction.md) |
| `tables-multiscreen-and-data-dashboards.md` | [表格](../components/advanced-table.md)、[多屏](../components/multi-screen.md)、[监控仪表](../components/monitoring.md) |
| `titlebar-design-and-integration.md` | [标题栏](../components/title-bar.md) |
| `multi-screen-workspace-and-icons.md` | [多屏](../components/multi-screen.md)、[主题/图标资源](../guides/themes-and-resources.md) |
| `docs/dark.png`、`docs/light.png` | `docs/images/`，保留原始图片内容 |

旧文档中“独立监控窗口按屏幕自动持久化”“颜色历史跨会话保存”“直接 add_subdirectory(src)”“公开 FilePickerWidget/ColorPickerWidget”等说明与当前代码不符，已从活动使用指南中修正。
