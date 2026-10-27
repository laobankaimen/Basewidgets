# 旧接口与接入方式迁移

[文档首页](../README.md)

公开头文件移除后，下游需要迁移并重新编译。发布时使用干净安装前缀，增量 install 不会自动删掉旧头文件。

## 主题与 QSS

| 旧入口/做法 | 当前方式 |
|---|---|
| QSS 模板、CSS 缓存/导出、生成开关 | 已移除；编辑 assets JSON/XML/SVG，构建时生成并嵌入资源 |
| `processStylesheetTemplate` | 已移除，无替代 QSS 解析入口 |
| `updateStylesheet(bool)` / `stylesheetChanged` | 兼容保留；新代码用 `updateTheme()` / `themeChanged` |
| `styleSheet()` | 兼容空查询，不再返回可应用的 QSS |
| 旧类名 `QtAdvancedStylesheet` | 名称保留，职责是主题色/SVG/资源管理 |
| 手动执行 tools 或 initialBuilder | 普通应用只需 `StyleManagerAdapter::initialize()`，主 CMake 自动生成资源 |
| 可执行文件旁自动加载旧 themes.rcc | 不再隐式加载；外部覆盖通过 initialize 的显式路径传入 |
| `BASEWIDGETS_FORCE_FUSION` | 不控制当前样式；初始化统一安装 NativeStyle |
| 旧 `UI_BUILD_TESTS` 缓存 | 改用 `UI_DEMO_CHECKS`，检查仍编进 widgets_demo |

外部独立 acss 管理器不是强制单例；推荐全应用入口是 StyleManagerAdapter::instance()。不要继续使用旧文档中的 getInstance/loadTheme/setTheme 假定接口，按公开头文件选择 setCurrentTheme + updateTheme。

## 仪表

| 移除内容 | 替代 |
|---|---|
| BasicGauge / SpeedGauge | `CircularGauge::DisplayMode::Needle` |
| QcGaugeWidget / QcItem 系列 | CircularGauge、CompassGauge、AttitudeMeter 直接自绘 |
| 经典玻璃/高光分支 | Arc/Needle 共享量程、角度、阈值和主题绘制 |

CircularGauge 保留基本量程、数值、标题、单位、颜色、阈值、刻度数/步长和两个旧预设入口。下列布局细节接口已删除，不提供无作用占位：

```text
setNeedleWrapRing / setNeedleWidth
setTickMajorWidth / setTickMajorLength / setTickMinorWidth / setTickMinorLength
setMinorTicksAsDots / setMinorTickDotAtInner / setMinorTickAngleRange
setTickInset / setTickLabelOffset / setTickLabelFontScale / setShowLabelInValue
```

改用控件内部统一布局，按业务量程配置阈值、目标与刻度；可用示例见[通用仪表](../components/gauges.md)。

## 图表

QCPLineChart、QCPBarChart、QCPWaterfallChart 三个薄封装已移除。通用折线使用 StyledPlot::addGraph()，柱状图使用 QCPBars；波形、频谱、时频场景改用业务图表。旧 ColorMap 示例不等于 STFT。

StyledPlot 统一主题、菜单、框选与复位；鼠标信号直接使用继承的 QCustomPlot 接口，不重复安装菜单处理器。业务图表内部 graph/item 归控件持有；需要完全自定义对象管理时直接使用 StyledPlot。

频谱/历程数据必须明确单位、网格和时序，参考[水声约定](../components/acoustic-analysis.md)及 [DEMON 边界](../components/direction-and-demon.md)。

## 表格与标题栏

| 项目 | 迁移要求 |
|---|---|
| AdvancedTable::prepareContextMenu | 传回 sourceModel() 索引，旧槽不要再映射一次分页索引 |
| 高亮选择与行头勾选 | 分别使用 selectedSourceRows / checkedSourceRows；勾选保留跨页状态 |
| 固定记录 | 参数属于源模型，固定的是排序位置，非视口冻结窗格；pageSize 不含固定行 |
| CustomTitleBar::setTitle/setIcon | 持续覆盖标题栏展示，不再被后续窗口同步覆盖；resetTitle/resetIcon 恢复默认 |
| 系统窗口标题/图标 | 使用窗口的 setTitle/setWindowTitle、setWindowIcon |
| 固定 40 像素标题高度 | 改用 profile 和字体自适应；MainWindow/SecondaryWindow/Dialog 分别配置 |

详细用法见[高级表格](../components/advanced-table.md)和[标题栏](../components/title-bar.md)。

## 多屏

以前只在某个屏幕打开同款监控窗口的 demo 已被 ScreenWindow + MultiScreenWorkspace 替代。子屏拥有独立标题/状态栏与内容，按业务 ID 复用；断屏隐藏保留，避免全部回退到主屏。

WindowPlacement 仍负责普通独立窗口的位置保存/恢复。库不写全局 QSettings；应用需要跨重启恢复业务视图时自行持久化 ID 与页面配置。见[多屏工作区](../components/multi-screen.md)。

## 对话框与资源

对外推荐使用 DialogUtils。旧文档中以 `Dialogs/FilePicker.h`、FilePickerWidget、ColorPickerWidget 为公开类的片段不适用；文件/颜色选择器的实现类不构成相同的公开 API。

颜色选择取消时返回有效初始色或最近选择色，数值输入取消时返回原输入值；需要单独的接受状态时使用 FramelessDialog。颜色历史只保存在进程内。

图标/字体仍在 assets 中，未因无 QSS 绘制而整体删除；默认 Ai 示例标识已替换为项目波形 Logo。资源安装方式见[构建与发布](cmake-and-packaging.md)。
