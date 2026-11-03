# widgets_demo 导览

[文档首页](../README.md) · [构建运行](getting-started.md)

所有页面位于 `examples/widgets_demo/`。普通构建无需 Qt Test，可选自检命令见[安装指南](installation.md#可选自检)。

## 页面入口

| 页面 / 源文件 | 主要用法 |
|---|---|
| 输入控件 / `pages/InputsPage.cpp` | [按钮、输入、日期与布局](../components/basic-widgets.md) |
| 对话框 / `pages/DialogsPage.cpp` | [文件、颜色、字体和消息窗口](../components/basic-widgets.md#标准对话框入口) |
| 常用组件 / `pages/CommonWidgetsPage.cpp` | [搜索、复制、筛选、反馈、登录和状态](../components/common-widgets.md)；[消息与关于](../components/notifications-and-about.md) |
| 文本与容器 / `pages/TextPage.cpp` | 文本编辑、容器和长标题 |
| 水声分析 / `pages/AcousticPage.cpp` | [波形、PSD、时频、时间线和信号质量](../components/acoustic-analysis.md) |
| 测向/解调 / `pages/DirectionAnalysisPage.cpp` | [多波束、LOFAR/DEMON 和业务选区](../components/direction-and-demon.md) |
| 数据监控 / `pages/DataManagementPage.cpp` | [容量、速率、任务和核验质量](../components/monitoring.md) |
| 仪表 / `pages/GaugesPage.cpp` | [数值、航向、姿态与阈值](../components/gauges.md) |
| 高级表格 / `pages/PagedTablePage.cpp` | [排序、分页、固定记录、勾选与 CSV](../components/advanced-table.md) |
| 数据首页 / `pages/DashboardPage.cpp` | [导航、指标卡、容量环与监控面板](../components/monitoring.md#数据管理首页与面板) |

`main.cpp` 负责主题、全局命令和主/子屏接线，`DemoPageIds.h` 统一页面编号。示例数据不代表设备测量；自动模拟默认关闭，页面隐藏时暂停计时器。水声数据为未校准归一化 ADC 信号，不把 dBFS/Hz 当作声压级。

## 启动方式

```bash
./build/bin/widgets_demo --theme sg-blue --dashboard
./build/bin/widgets_demo --theme sg-blue --centered-title
./build/bin/widgets_demo --styles-dir /path/to/styles --theme ocean
```

`--dashboard` 打开数据首页，`--centered-title` 使用居中品牌标题。`--styles-dir` 支持外部 styles 目录或 RCC，`--theme` 为不含扩展名的 XML 文件名；省略时使用内嵌默认主题。

## 全局交互

- “打开录音”选择文件并进入录音列表，不隐式解析文件；“导出 CSV”保存当前筛选结果。
- “录音 / 监控 / 测向”与标签页同步；窄窗口通过工具栏溢出菜单和标签滚动保留入口。
- 搜索 Enter 后切换到高级表格并筛选录音，任务中心打开监控页。设置可切换标题对齐、图标/搜索显隐和左侧动作。
- 标题铃铛与常用组件页共享未读模型；帮助菜单打开 AboutDialog。登录示例只展示输入/等待/错误反馈，不接认证服务。
- 状态栏显示“演示·未连接设备”、版本和本机时间；本机时钟不表示数据更新时间。
- 图表支持右键、Shift 框选、双击/Home 复位；表格勾选跨页保留，固定记录遵守筛选。业务处理由调用方接线。

## 多屏与安装消费

“屏幕”菜单可选择主控屏、自动创建/恢复子屏，或在单屏打开预览。子屏有独立标题、内容和状态栏，录音源模型与主控共享；关闭只隐藏，重开复用状态。详见[多屏工作区](../components/multi-screen.md)。

`widgets_demo` 联编源码树，`examples/package_consumer` 演示安装包的 `find_package` 接入。Linux desktop 入口的可选安装见[安装指南](installation.md#linux-桌面入口)。
