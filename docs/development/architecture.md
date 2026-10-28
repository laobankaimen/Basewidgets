# 架构与维护约束

[文档首页](../README.md)

## 运行时绘制

目标是运行时不生成、不应用 QSS，同时保留 Qt Widgets 的编辑、布局、信号、键盘和可访问性行为。“原生绘制”指 Qt QStyle/QPainter 路径，不等同于操作系统原生皮肤。

```text
assets/styles/ JSON + XML + SVG + fonts
                 │ 构建期生成
        内嵌 C++ 资源 + 可选外部 RCC
                 │
QtAdvancedStylesheet ── updateTheme / themeChanged
                 │
StyleManagerAdapter ── ThemeTokens ── QPalette
                 │
NativeStyle (QProxyStyle / Fusion)
  ├─ 绘制：drawPrimitive / drawControl / drawComplexControl
  ├─ 尺寸：pixelMetric / sizeFromContents
  └─ 几何：subControlRect / hitTestComplexControl
```

统一样式同时覆盖 basewidgets 包装类和复合控件内部的 Qt 控件。复杂文本、菜单、输入法、选择与编辑器继续由 Qt 处理，不为每个控件重复实现事件系统。

## 长期约束

1. 绘制入口保持单一。新控件使用 NativeStyle/QPalette/ThemeTokens，不拼接局部或应用级 QSS。
2. 主题色来自 JSON/XML 和 ThemeTokens。几何常量集中在相应绘制/布局实现，不散落到业务流程中；字体、尺寸使用逻辑像素和字体度量。
3. 子控件绘制和命中使用同一几何。保留 hover、pressed、checked、partial、disabled、默认按钮、图标、助记符、RTL 和键盘焦点。
4. 动态状态属性使用 `bw*` 前缀，并落实绘制与重绘通知；保留已有 `primary/active` 兼容语义，不新增只对旧 QSS 有效的属性。
5. 基础 Inputs/Views/Containers 不依赖业务页面。DSP/统计放 Acoustics，图表展示放 Charts，状态仪表放 Gauges。
6. 平台窗口适配放 Core，避免散落进每个控件。主/子屏窗口共享平台实现，但内容、状态和角色独立配置。
7. 源模型与业务数据所有权由应用明确持有；公开接口写明借用、接管、线程和失败行为。
8. 未知数据明确展示为空/无数据，不擅自补零、校准、预测、启动设备或执行后台任务。

## 资源依赖

`cmake/resources` 的内部生成器只依赖 Qt 与主题解析器，不链接 BaseWidgets。先生成资源，再构建 UI 库，避免循环依赖。构建产物留在 build 目录，源 SVG/主题/字体和许可证留在 assets。

图标原始矢量在实际绘制目标渲染，不预先按另一块屏幕的 DPR 栅格化再放大；详细来源见[主题与资源](../guides/themes-and-resources.md)。

## 窗口与布局

FramelessWidgetWindow 的 title/menu 位于 QMainWindow::setMenuWidget，工具栏/状态栏使用 QMainWindow 区域，业务内容置于独立 content host。外部使用 setContentWidget，不绕过接口替换内部 centralWidget/menuWidget。

普通窗口使用 WindowPlacement 管理恢复；MultiScreenWorkspace 管理的子屏禁用通用断屏回退，采用暂停隐藏并保留业务实例的策略。两者的职责见[多屏工作区](../components/multi-screen.md)。

Qt5 Wayland 系统拖动、释放丢失清理与 Windows 原生命中集中在 Core；这些路径仍需真实窗口管理器验证。截图正确不代表平台交互已经验收。

## 性能和验证

不以“使用 QPainter”直接宣称性能提升。可量化的验证应固定页面、构建类型、数据量、缩放和硬件，分别比较首次显示、连续重绘、主题切换与动画 CPU。

当前已移除运行时 QSS 工作、对曲线使用按像素保极值抽样、热力图使用有界网格聚合、常规图表更新使用帧率调度与有界历史，并在页面隐藏时暂停演示计时器；这些是实现行为，不是未经测量的性能倍数承诺。验证流程见[验证指南](validation.md)。
