# widgets_demo 导览

[文档首页](../README.md)

所有控件演示和可选功能检查集中在 `examples/widgets_demo/`。构建与运行见[快速入门](getting-started.md)，`--verify/--smoke-test` 见[验证指南](../development/validation.md)。普通构建无需 Qt Test。

## 页面与源码入口

| 页面/文件 | 演示重点 | 接口文档 |
|---|---|---|
| `main.cpp` | 初始化、主题、主控/子屏、标题操作、共享录音模型 | [标题栏](../components/title-bar.md)、[多屏](../components/multi-screen.md) |
| 输入控件 / `pages/InputsPage.cpp` | 按钮层级、选择状态、双向绑定、自适应卡片 | [基础控件](../components/basic-widgets.md) |
| 对话框 / `pages/DialogsPage.cpp` | 文件/颜色/字体/消息、简洁标题、拖动反馈 | [基础控件](../components/basic-widgets.md#标准对话框入口) |
| 视图控件 / `pages/ViewsPage.cpp` | 模型所有权、导航 ID、表格与树形聚合 | [基础控件](../components/basic-widgets.md#视图与布局) |
| 文本与容器 / `pages/TextPage.cpp` | 文本编辑、容器和长标题省略 | [基础控件](../components/basic-widgets.md) |
| 水声分析 / `pages/AcousticPage.cpp` | 波形、PSD、缺帧、时间线、信号质量、采集状态 | [水声分析](../components/acoustic-analysis.md) |
| 测向/解调 / `pages/DirectionAnalysisPage.cpp` | 多波束、门限、LOFAR、DEMON 和业务选区 | [测向与解调](../components/direction-and-demon.md) |
| 数据监控 / `pages/DataManagementPage.cpp` | 容量/速率、计数重置、任务队列和核验质量 | [监控仪表](../components/monitoring.md) |
| 仪表 / `pages/GaugesPage.cpp` | 完成量、阈值、航向、姿态、未知状态 | [通用仪表](../components/gauges.md) |
| 高级表格 / `pages/PagedTablePage.cpp` | 三种模式、固定记录、跨页勾选、菜单与导出 | [高级表格](../components/advanced-table.md) |

页面自动模拟默认关闭，隐藏时暂停计时器。水声示例为未校准归一化 ADC 数据；不能将 dBFS/Hz 当作实测声压级。控件不读取采集卡、数据库或本机网卡计数；业务接入需遵守数据单位和时间戳约定。

## 标题栏与全局操作

设置菜单可切换标题左/居中、独立标题、图标和搜索显隐，用户入口展示模拟账户。搜索 Enter 后切换到高级表格并筛选录音，任务中心打开数据监控页。窄窗口搜索折叠为图标浮层，次要动作进入“更多”。

## 高级表格与图表

表格默认固定前两条记录；固定记录遵守筛选并重复显示于各页。左侧勾选跨页保留，左上角全选当前页，底部显示勾选数和清空入口。勾选与高亮选择独立。

图表支持右键扩展、Shift 框选放大、双击/Home 复位；还可切换区域选择和坐标拾取模式。演示的业务菜单只展示接线方式，不自动执行生产数据处理。

## 主控与子屏

普通启动后，其他显示器自动创建子屏工作区；“屏幕”菜单可选择主控屏、控制自动创建、重新显示子屏，或在单屏环境打开预览。

子屏有独立标题、业务内容、状态与“返回主屏”入口。录音表格与主控共享源模型，各图表页面仍为独立合成示例。关闭子屏只隐藏，重新打开复用状态；断屏暂停/恢复由 MultiScreenWorkspace 管理。

## 公开示例与包消费示例的区别

widgets_demo 在源码工程中直接链接 BaseWidgets；`examples/package_consumer` 是独立配置的安装树消费示例。后者用于检查安装包，不加入默认的九个展示页，配置方法见[构建与发布](../development/cmake-and-packaging.md#外部工程引用)。
