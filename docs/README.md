# BaseWidgets 文档

[项目首页](../README.md)

本文档按使用任务分类。当前接口用法与历史设计记录分开维护；函数签名以 `interface/basewidgets/` 为准，示例和检查集中在 `examples/widgets_demo/`。

## 阅读路线

- 初次接入：[快速入门](guides/getting-started.md) → [Demo 导览](guides/demo.md) → 对应控件文档。
- 水声业务：[图表交互](components/chart-interaction.md) → [数据单位与频谱](components/acoustic-analysis.md) → [测向与解调](components/direction-and-demon.md)。
- 数据管理：[高级表格](components/advanced-table.md) → [监控仪表](components/monitoring.md) → [多屏工作区](components/multi-screen.md)。
- 维护与发布：[架构约束](development/architecture.md) → [构建发布](development/cmake-and-packaging.md) → [验证](development/validation.md)。
- 升级旧项目：[迁移指南](development/migration.md)。

## 使用指南 · guides

| 文档 | 内容 |
|---|---|
| [快速入门](guides/getting-started.md) | 环境、构建运行、最小应用、GUI 线程与所有权 |
| [主题与资源](guides/themes-and-resources.md) | NativeStyle、主题切换、图标来源、DPI、资源生成和外部覆盖 |
| [Demo 导览](guides/demo.md) | 十个页面、标题栏操作、主/子屏预览和模拟数据边界 |

## 控件与接口 · components

| 文档 | 内容 |
|---|---|
| [基础控件](components/basic-widgets.md) | 按钮、输入、日期范围、视图、布局和 DialogUtils |
| [常用交互与反馈](components/common-widgets.md) | 分段切换、筛选标签、行内消息和空状态 |
| [高级表格](components/advanced-table.md) | 三种模式、固定记录、勾选全选、模型映射、右键与 CSV |
| [图表通用交互](components/chart-interaction.md) | 导航、框选、复位、坐标/选区信号、菜单扩展与导出 |
| [水声分析](components/acoustic-analysis.md) | 波形、功率谱/PSD、时频缓存、校准、录音时间线 |
| [测向与解调](components/direction-and-demon.md) | 多波束、方位历程、LOFAR、DEMON 和算法边界 |
| [数据管理仪表](components/monitoring.md) | 容量、双向速率、任务、核验质量与采集状态 |
| [通用仪表](components/gauges.md) | CircularGauge、CompassGauge、AttitudeMeter |
| [标题栏](components/title-bar.md) | 三种 profile、标题/图标覆盖、搜索、用户与动作扩展 |
| [多屏工作区](components/multi-screen.md) | 主/子屏职责、共享模型、分配与断屏、WindowPlacement |

## 开发与维护 · development

| 文档 | 内容 |
|---|---|
| [构建与发布](development/cmake-and-packaging.md) | Qt 选择、选项、资源构建、安装组件、find_package 与 CPack |
| [验证指南](development/validation.md) | demo 检查、截图、最近验证记录与尚需实机验收的范围 |
| [架构约束](development/architecture.md) | 绘制、模块分层、资源依赖和平台代码边界 |
| [迁移指南](development/migration.md) | QSS 兼容层、移除接口与新接入方式 |

## 历史资料 · archive

[原生绘制迁移记录](archive/native-style-history.md) 保存方案修正原因和阶段记录，不作为当前使用说明。旧文档路径到新位置的对应表也在该文件中。

## 文档维护规则

1. 一个主题只维护一份完整说明；其他页面用相对链接引用，不继续追加“本轮新增”段落。
2. 构建/安装命令集中在构建文档；测试结果集中在验证文档，记录环境和日期。
3. API 示例区分可独立运行的程序与需业务提供变量的片段；不要把内部实现类写成公开接口。
4. 历史结论保留时间和范围，不把 offscreen 检查扩写为 Windows、真实硬件或多屏已验收。
5. 截图统一放在 `images/`，资源许可证继续跟随 `assets/` 中的原文件。
6. 新文档加入本索引；移动文件后同步更新源码树与安装树中的相对链接。
