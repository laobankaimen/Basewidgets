# QPainter 原生绘制优化方案

## 1. 结论与范围（2026-10-26 修订）

方向合理可行，但原方案不能直接按“每个控件重写 paintEvent”推进。核心目标明确为：**BaseWidgets 运行时不生成、不应用 QSS，由 QPainter / QStyle 和 QPalette 提供绘制。** 保留 Qt 控件的事件处理、编辑器、布局、信号和可访问性。

本轮已实现这一运行时路径，并删除 QSS 模板解析、CSS 缓存、CSS 导出及 `themes.css.template`。内部资源生成器只生成主题色、字体和 SVG，随 UI 库构建自动生成 RCC 和内嵌 C++ 资源，不包含 CSS/QSS 文件。

新增正式入口 `updateTheme()` / `themeChanged`。旧类名 `QtAdvancedStylesheet`、`updateStylesheet(bool)`、`stylesheetChanged` 和空的 `styleSheet()` 查询保留为兼容层；它们不再包含 QSS 逻辑。`processStylesheetTemplate` 与生成开关已移除。调用方自行设置的 QSS 不属于本库迁移范围，不应与 NativeStyle 混用。

## 2. 原方案需要修正的地方

| 原判断/做法 | 代码审查结论 | 修正 |
|---|---|---|
| 控件是薄包装，因此重写绘制风险低 | 继承的尺寸、命中区域、RTL、快捷键、菜单和辅助功能仍依赖 QStyle | 用一个 NativeStyle 同时控制绘制、尺寸和子控件几何；保留 QWidget 子类交互 |
| 各控件自绘，同时保留应用级 QSS | QSS 仍会接管背景、边框、padding 和子控件布局，自绘与命中区域可能冲突 | 停用应用级 QSS；移除 demo 中的局部 QSS |
| 复杂控件保留少量 QSS | 会长期保留第二套样式入口，与核心目标冲突 | 输入框边框交给 QStyle 原语；文本编辑、菜单、视图等使用 QPalette 和 Fusion 原生绘制 |
| ToolButton 复用 PushButton 绘制 | ToolButton 有 autoRaise、箭头、图文排列和分离菜单按钮 | 共享面板绘制，保留 ToolButton 的样式选项、标签和命中区域 |
| 删除焦点外框 | 会损害键盘导航可见性 | 保留控件内部细焦点边框或滑块 halo |
| 已有第一阶段绘制可以直接沿用 | 半选态先进入 checked 分支；图标/RTL/助记符与几何不完整；进度条缺反向、busy 和溢出处理 | 将绘制收敛到 NativeStyle，增加行为和渲染回归测试 |
| 使用 QPainter 必然更快并修复字体/透明度问题 | QPainter 自身不保证更快，透明度效果与非法字体尺寸需要单独归因 | 仅确认移除了运行时 QSS 工作；性能收益、动画 CPU 和效果链另做测量 |
| 参考项目证明可像素级复刻 | 不能替代本项目的跨平台、交互和布局验收 | 用本项目的测试、控件图集及真实 demo 验证 |

## 3. 实现架构

```text
QtAdvancedStylesheet
  ├─ XML/JSON 主题色、SVG 资源与图标
  ├─ updateTheme(): 刷新颜色/资源
  └─ 主构建内部生成器: 打包 JSON/XML/字体/SVG
            │ themeChanged（另保留旧信号兼容）
StyleManagerAdapter
  ├─ 安装 NativeStyle（QProxyStyle，Fusion 为基础）
  ├─ ThemeTokens.reload() → 完整 QPalette → 全部控件重绘
  └─ 不调用 setStyleSheet(css)
            │
NativeStyle
  ├─ drawPrimitive / drawControl / drawComplexControl → QPainter
  ├─ pixelMetric / sizeFromContents → 字体感知的尺寸
  ├─ subControlRect / hitTestComplexControl → 共用滑块几何
  └─ 复杂原生标签、菜单和文本/视图绘制 → Fusion + 主题 QPalette
```

这里的“原生”指 Qt Widgets 的 QStyle/QPainter 绘制路径，不表示操作系统原生皮肤。`QProxyStyle` 不是 QSS。公共样式同时覆盖 `basewidgets::*` 包装和复合控件内部直接创建的 `QPushButton`、`QSpinBox` 等，避免遗漏内部子控件。

`StyleManagerAdapter` 继续采用应用级主题，构造函数中的 manager 参数保留现有 API；本轮不新增同一应用中多个独立主题的承诺。应用初始化会安装 NativeStyle，原先 Linux 的 `BASEWIDGETS_FORCE_FUSION` 环境变量不再控制这一模式。

## 4. 主题与几何约束

- ThemeTokens 提供明暗主题回退色，以及 Active / Inactive / Disabled 的完整 QPalette；不再依赖旧 JSON 中不完整的 palette 映射。
- 在 manager 初始化之前访问 ThemeTokens 不触发递归初始化；显式传入 light 主题不会被 setDefaultTheme 覆盖。
- checked / primary 按钮用 HighlightedText，避免浅色主题中选中态文字与背景混淆。
- 保留 `bwPrimary`、`primary`、`active` 动态属性语义，变化时自动重绘。
- 控件尺寸以 Qt 逻辑像素和字体度量计算，不手动乘 DPI。滑块为可容纳 halo 的 22 逻辑像素命中区域，手柄在区域内部绘制，端点不向外扩张。
- 保留 hover、pressed、checked、partial、disabled、default、flat、键盘焦点、图标、助记符和 RTL。
- 文本编辑光标、选择、输入法、弹出列表、菜单及可访问性继续由 Qt 实现。
- ToggleSwitch 的动画跟随 toggled，覆盖鼠标、键盘和 setChecked；BusyIndicator 默认跟随主题更新，显式 setColor 后保留调用方颜色。

## 5. 分阶段实施与当前状态

### Phase 1：修正主题入口与基础控件 — 已完成

- 安装共享 NativeStyle；运行时关闭 CSS 生成与应用，兼容原 updateStylesheet 调用。
- ThemeTokens 的初始化、明暗回退和完整 palette 已接通。
- PushButton / CheckBox / RadioButton / ProgressBar 回到标准 Qt 绘制分发入口，由 NativeStyle 自绘。
- Checkbox 半选、默认按钮、助记符、图标和 RTL 交给统一样式处理。
- ProgressBar 处理 reset、垂直、RTL、反向、整型极值；busy 动画委托 Fusion，避免新增每控件定时器。

### Phase 2：中等控件 — 已完成公共绘制迁移

- Slider 自绘轨道、填充、手柄和 halo；命中区域与绘制共用几何，保留刻度及方向语义。
- TabBar 自绘背景和选中边线；原生标签保留图标、关闭按钮、助记符和不同 tab 方位支持。
- ToolButton 自绘面板、分离菜单按钮；标签使用原生布局。
- GroupBox / Frame 通过样式原语绘制边框；Label 使用 palette 保留富文本、选择和换行能力。

### Phase 3：复杂控件 — 已完成无 QSS 渲染路径

- ComboBox / SpinBox 自绘框体和箭头，保留弹出列表、内嵌编辑器、步进与 NoButtons / PlusMinus。
- LineEdit 自绘面板和边框；内嵌无边框编辑器不重复画框。
- TextEdit / PlainTextEdit 使用主题 palette 和样式边框，保留文档编辑逻辑。
- ScrollBar 自绘轨道、滑块和箭头，命中区域沿用 Qt 几何。
- Menu / MenuBar / ToolBar / StatusBar / TreeView / TableView 等采用 Fusion 的 QPainter 绘制与统一 palette。它们已脱离 QSS，但没有逐项重制全部设计细节，也不宣称与旧 QSS 像素一致。
- 示例颜色预览改为 palette，移除局部 setStyleSheet。

### Phase 4：验收 — 自动化已接入，平台验收继续保留

验证入口统一放入 `examples/widgets_demo/DemoChecks.cpp`，通过 `UI_DEMO_CHECKS`（默认 OFF）开启 `widgets_demo --verify`。没有独立 tests 目录或测试可执行文件。验证涵盖：

- tokens 先于 manager 创建；显式 light 初始化；明暗往返；应用/manager 的 stylesheet 为空；临时输出目录没有新 CSS。
- Checkbox 三态、RTL 命中、禁用、图标尺寸；RadioButton 排他；PushButton 键盘触发；ToggleSwitch 键盘与程序切换动画。
- Slider 横/竖 × LTR/RTL × 反向的 8 组端点、命中、拖动和键盘操作。
- ProgressBar 方向、reset、极值范围、`%m` 与 busy 动画。
- LineEdit 输入、ComboBox 弹出/键盘、SpinBox 步进、ToolButton 主按钮/分离菜单及 ScrollBar 命中/步进。
- BusyIndicator 显式颜色在主题切换后保留；隐藏时停止计时器，恢复可见后按运行状态恢复。
- 新 RCC 中没有 CSS/QSS 资产；SVG 图标在 manager 销毁和复制后保持安全。
- 明暗图集，以及真实 widgets_demo 的 7 个页面分别切换明暗主题、渲染、检查全部控件无 QSS。

本机验证基于 Linux / Qt 5.15 / offscreen；桌面窗口管理器下的实际鼠标体验、输入法、屏幕阅读器和 Windows/Qt6 仍需对应环境验收。offscreen 的 raise、grab keyboard、propagateSizeHints 警告属于该插件限制，不能用它代替桌面交互验收。

## 6. Core 与复合控件复查

- `FramelessWidgetWindow` 保留自定义标题栏，用 `QMainWindow::setMenuWidget` 放置标题/菜单；工具栏和状态栏使用 Qt 原生区域，内容区域独立伸展。
- `setTitle` 更新 QWidget 标题；contentWidget 返回实际内容；替换内容先隐藏旧控件再延迟删除。对话框重复设置内容复用同一个布局。
- 全屏返回时恢复原来的最大化状态，不改变 resizable 配置；Linux resize 使用同一个边缘宽度，按下时重新命中，尺寸限制保持对侧边缘不跳动。
- 窗口尺寸持久化从库移到 demo，避免所有窗口共用硬编码的 ZKHX/MainWindow 设置键。Qt6 nativeEvent 结果类型与 Qt5 分开。
- `CustomTitleBar` 使用新增 ElidedLabel，标题文字不撑宽窗口；保留对话框的按钮显隐约定。
- 新增 ResponsiveGridLayout，按可用宽度排列卡片，支持 height-for-width、隐藏控件和 RTL。
- 输入、文本、仪表盘 demo 使用自适应卡片；对话框示例独立成页；删除重复工具栏、固定按钮尺寸与重复 setLayout。
- AdvancedTable 分页栏分两行，消除单行最小宽度约 1153 像素对整个窗口的约束；同步跳转框，复制 CSV 正确经过分页和筛选两层代理，并转义字段。
- DateRangePicker 统一构造、两列布局，初始化今天，时间控件参与 setRange/range，时区选择解释显示的当地时间，批量设置只发出一次通知。
- NavigationList 发出原来声明但未触发的点击信号，clear/takeItem 后不保留悬空 ID 指针，使用字体感知的行高。
- BasicGauge 刻度随主题切换；demo 的预设改用真正实现这些接口的 CircularGauge。SpeedGauge 的旧空实现接口保留但标记弃用，避免继续误用。

## 7. 可复现验证

```bash
cmake -S . -B cmake-build-debug -DUI_DEMO_CHECKS=ON
cmake --build cmake-build-debug --parallel 4
QT_QPA_PLATFORM=offscreen ./cmake-build-debug/bin/widgets_demo --verify

# 7 页 × 明暗主题 × 1100/640 逻辑像素宽度
QT_QPA_PLATFORM=offscreen BW_NATIVE_CAPTURE_DIR=/tmp/basewidgets-native-demo \
  ./cmake-build-debug/bin/widgets_demo --smoke-test

# 非整数缩放
QT_QPA_PLATFORM=offscreen QT_SCALE_FACTOR=1.5 \
  ./cmake-build-debug/bin/widgets_demo --verify

# 单独请求资源生成（一般只需正常构建 UI 库）
cmake --build cmake-build-debug --target basewidgets_resources
```

普通 demo 构建不需要 Qt Test；只有 `UI_DEMO_CHECKS=ON` 时链接验证依赖。旧构建目录中的 `UI_BUILD_TESTS` 缓存不再使用。

已检查的布局和逻辑不等同于全平台验证。Windows 的原生消息处理、真实 Wayland/X11 窗口管理器、输入法和可访问性仍需要对应环境验收；不要将 offscreen 图集当作这些平台行为的证明。

性能验收另行对比相同页面、构建类型、缩放与设备下的首次显示、连续重绘、主题切换耗时和动画 CPU 占用；目前只确认移除了 QSS 工作，并停止隐藏 BusyIndicator 的计时器，不宣称已有量化性能提升。

## 8. 本轮验证结果（Linux / Qt 5.15）

- `widgets_demo --verify`：31 项通过；1.5 倍缩放同样通过。
- `--smoke-test`：7 页 × 2 主题 × 2 宽度，共 28 组渲染通过；实际检查了宽/窄窗口图集。
- AddressSanitizer 构建运行同一验证入口：未报告越界或释放后访问。未启用 LeakSanitizer（`detect_leaks=0`），不据此声称没有内存泄漏。
- 资源生成成功，RCC 不含 CSS/QSS，`git diff --check` 通过。

## 9. 按钮层级、窗口控制及构建整合

- 默认 Neutral 按钮只保留中性底色；hover/pressed 表达反馈，鼠标焦点不持续描边。
- Primary 强调当前区域主要动作；Subtle 用于轻操作，Outline 显式开启常驻边线，Danger 表达破坏性动作。
- checked/active 为浅强调选中态，和 Primary 的实底分别表达；键盘焦点仍可见。
- 关闭按钮常态透明、悬停危险红底白叉、按下加深；最小化/最大化为中性反馈。图标由 QPainter 绘制，避免 SVG 在红底上的对比度失效。
- 设计参考：Fluent 2 Button usage 的 Primary/Secondary/Subtle/Transparent 层级，以及 Windows 标题栏的 close hover 约定；本项目保留独立 Outline 作为显式选项。
- `tools/` 内容合并：源资源在 `assets/styles`，构建实现位于 `cmake/resources`。内部可执行文件仅依赖 Qt 和主题解析器，不链接 BaseWidgets，因此无生成循环。
- 资源 C++ 在构建时生成并嵌入 UI 库；默认主题切换直接读取内嵌的各主题 SVG 变体，不写应用目录。
- 同时生成 `<build>/generated/assets/themes.rcc` 供安装包和显式覆盖使用。不会自动读取应用旁旧的 RCC。
- 公共 API 已补充所有权、线程、尺寸单位、主题入口和选中语义说明；demo 增加公开 API 示例、各页面说明和接入 README。

本轮验证：`widgets_demo --verify` 33 项通过；默认主题直接读取内嵌资源，切换不生成磁盘文件。
全新构建（UI_DEMO_CHECKS=OFF）完成编译、安装、外部 consumer 编译和无外部 RCC 启动。
主题 XML 时间戳变更触发资源重建；随后的 Ninja 构建为 no work to do。

2026-10-27 收尾验证：1.5 倍缩放下 33 项检查通过，7 页 × 明暗主题 × 两种宽度的
28 组渲染通过；`package_assets_dir` 安装生成 RCC 成功，普通增量构建无重复资源生成。

## 10. 弹出层与拖动回归修正（2026-10-27）

- ToolButton 不再把菜单箭头画在右下角：独立槽位垂直居中，标签绘制去掉重复 menu 特征；分离菜单的绘制/命中共用几何。
- 通过 SH_DialogButtonBox_ButtonsHaveIcons=false 和初始化清空标准按钮图标，避免桌面主题自动添加“取消/确定”图标；不移除显式业务图标。
- 对话框 Window 调色板采用 bg1 浮层色，预留 1 逻辑像素给边框，页脚加分隔线；普通激活/模态行为交给 QDialog。
- 移除对话框的 WA_ShowWithoutActivating 与 X11 专用状态切换；Wayland 不尝试客户端全局定位或不受支持的手动移动回退。
- 标题栏在取消捕获、隐藏、失活、鼠标已释放时清理手动拖动状态。
- Qt5 Wayland 成功启动系统拖动后，排队向对应 QWindow 发送一次 Qt 层释放事件，清理 QWidgetWindow 的隐式按下目标；不模拟平台输入，也不访问 Qt 私有 API。
- TabWidget 默认无 hover；外观上选中和键盘焦点保持可见，可用 setHoverEffectEnabled 显式启用 hover。
- 对话框隐藏 OK 按钮时同时取消其 default/autoDefault；消息正文明确按 PlainText 处理。

自动化覆盖：大字体/RTL 的三种菜单按钮、标签 hover 默认/开启状态、明暗弹窗边界与无图标按钮、
拖动丢失 release 和 ungrab 后的 hover/click 恢复。系统拖动的真实 Wayland 输入交接仍需桌面验收，
不能用 offscreen 测试的通过代替这一结论。

本轮结果：正常缩放及 1.5 倍缩放的新增回归均通过（37 项）；主 demo 28 组渲染成功，
菜单按钮大字号/RTL 图集、明暗对话框边框和标准按钮截图已检查。
拖动回退测试的最终 hover/click 通过 QWindow 事件分发验证，避免 offscreen 光标移动在分数缩放下缺失 enter/leave 的干扰。
真实 Wayland 系统拖动交接只完成代码修正，未声称已在桌面合成器上复现并验证。
