# widgets_demo 导览

[文档首页](../README.md)

所有控件演示和可选功能检查集中在 `examples/widgets_demo/`。构建与运行见[快速入门](getting-started.md)，`--verify/--smoke-test` 见[验证指南](../development/validation.md)。普通构建无需 Qt Test。

## 页面与源码入口

| 页面/文件 | 演示重点 | 接口文档 |
|---|---|---|
| `main.cpp` | 初始化、主题、主控/子屏、标题操作、共享录音模型 | [标题栏](../components/title-bar.md)、[多屏](../components/multi-screen.md) |
| 输入控件 / `pages/InputsPage.cpp` | 按钮层级、选择状态、双向绑定、自适应卡片 | [基础控件](../components/basic-widgets.md) |
| 对话框 / `pages/DialogsPage.cpp` | 文件/颜色/字体/消息、简洁标题、拖动反馈 | [基础控件](../components/basic-widgets.md#标准对话框入口) |
| 文本与容器 / `pages/TextPage.cpp` | 文本编辑、容器和长标题省略 | [基础控件](../components/basic-widgets.md) |
| 水声分析 / `pages/AcousticPage.cpp` | 波形、PSD、缺帧、时间线、信号质量、采集状态 | [水声分析](../components/acoustic-analysis.md) |
| 测向/解调 / `pages/DirectionAnalysisPage.cpp` | 多波束、门限、LOFAR、DEMON 和业务选区 | [测向与解调](../components/direction-and-demon.md) |
| 数据监控 / `pages/DataManagementPage.cpp` | 容量/速率、计数重置、任务队列和核验质量 | [监控仪表](../components/monitoring.md) |
| 仪表 / `pages/GaugesPage.cpp` | 完成量、阈值、航向、姿态、未知状态 | [通用仪表](../components/gauges.md) |
| 高级表格 / `pages/PagedTablePage.cpp` | 三种模式、固定记录、跨页勾选、菜单与导出 | [高级表格](../components/advanced-table.md) |
| 数据首页 / `pages/DashboardPage.cpp` | 指标卡、丢包率、双集群容量环与首页导航 | [数据管理仪表](../components/monitoring.md#数据管理首页与面板) |
| 常用组件 / `pages/CommonWidgetsPage.cpp` | 搜索/复制/状态/折叠参数、基础视图、筛选与反馈 | [常用交互与反馈](../components/common-widgets.md) |

原“视图控件”页已合并到“常用组件”，采用三列/两列/单列自适应分组；模型所有权、导航 ID、树形汇总示例保留。`DemoPageIds.h` 统一维护页面编号，工具栏和首页导航复用同一组常量。

页面自动模拟默认关闭，隐藏时暂停计时器。水声示例为未校准归一化 ADC 数据；不能将 dBFS/Hz 当作实测声压级。控件不读取采集卡、数据库或本机网卡计数；业务接入需遵守数据单位和时间戳约定。

## 菜单、命令栏与页面导航

顶部菜单保持紧凑；命令栏使用轻微表面底色与统一 SVG，按“文件操作 / 工作区导航 / 主题及更多”分组。

- “打开录音”选择文件并进入录音列表，状态栏明确这是示例接入点，没有隐式解析录音。
- “导出 CSV”导出录音表格的当前筛选结果，路径由用户选择。
- “录音 / 监控 / 测向”切换对应页面，选中态与标签页同步。
- “主题”菜单枚举当前资源包中的全部 XML 配色，新增模式不需要改菜单代码；“更多”也可进入主题菜单。
- “更多”提供保存窗口布局和主题切换。窄窗口下命令由 QToolBar 溢出菜单保留，标签页横向滚动。

标签采用安静底色与短选中线，页面内容没有随鼠标变色的整体描边；输入和键盘导航的焦点仍保留。

## 标题栏与全局操作

设置菜单可切换标题左/居中、图标随标题对齐、左侧业务入口、独立标题及图标/搜索显隐，用户入口展示模拟账户。左侧“分析工作台”进入水声分析页。搜索 Enter 后切换到高级表格并筛选录音，任务中心打开数据监控页。窄窗口搜索折叠为图标浮层，次要动作进入“更多”。

可运行 `./build/bin/widgets_demo --theme sg-blue --dashboard` 查看带侧边导航、统计卡和科技标题栏的数据管理首页；运行 `./build/bin/widgets_demo --theme sg-blue --centered-title` 查看参考设计图的蓝色配色与居中品牌标题。配色和标题布局独立；生产代码见[标题栏接入](../components/title-bar.md#参考图形式中间品牌标题左侧按钮)。

标题栏铃铛和常用组件页共享未读状态；“帮助 → 关于组件示例”打开可配置的 AboutDialog，常用组件页也可编辑名称/版本后预览。组件 API 见[消息与关于](../components/notifications-and-about.md)。

## 高级表格与图表

表格默认固定前两条记录；固定记录遵守筛选并重复显示于各页。左侧勾选跨页保留，左上角全选当前页，底部显示勾选数和清空入口。勾选与高亮选择独立。

图表支持右键扩展、Shift 框选放大、双击/Home 复位；还可切换区域选择和坐标拾取模式。演示的业务菜单只展示接线方式，不自动执行生产数据处理。

## 主控与子屏

普通启动后，其他显示器自动创建子屏工作区；“屏幕”菜单可选择主控屏、控制自动创建、重新显示子屏，或在单屏环境打开预览。

子屏有独立标题、业务内容、状态与“返回主屏”入口。录音表格与主控共享源模型，各图表页面仍为独立合成示例。关闭子屏只隐藏，重新打开复用状态；断屏暂停/恢复由 MultiScreenWorkspace 管理。

## 公开示例与包消费示例的区别

widgets_demo 在源码工程中直接链接 BaseWidgets；`examples/package_consumer` 是独立配置的安装树消费示例。后者用于检查安装包，不加入默认的十个展示页，配置方法见[构建与发布](../development/cmake-and-packaging.md#外部工程引用)。

## 外部主题预览

```bash
./build/bin/widgets_demo --styles-dir /path/to/styles --theme ocean
```

`--styles-dir` 接受完整外部 styles 目录或 RCC 文件，`--theme` 为 XML 文件名且不含扩展名；两项省略时继续使用内嵌默认主题。运行 `--smoke-test` 会遍历当前包的全部主题。新配置准备步骤见[主题指南](themes-and-resources.md#新增配色模式只改配置)。
