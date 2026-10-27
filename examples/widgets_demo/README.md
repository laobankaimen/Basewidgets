# widgets_demo：公开 API 接入示例

本目录集中展示 UI 库的用法。普通构建无需 Qt Test，也不需要单独执行资源工具。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target widgets_demo --parallel 4
./build/bin/widgets_demo
```

`BaseWidgets` 构建会从 `assets/styles` 自动生成主题色、字体和 SVG 资源并编译进库。
可执行文件的工作目录不影响默认主题；修改 JSON/XML/SVG 后重新构建即可。
独立 RCC 同时生成到 `build/generated/assets/themes.rcc`，只在需要外部覆盖包时使用。

## 基本接入顺序

```cpp
// 必须在 QApplication 之前设置高 DPI 兼容选项。
basewidgets::StyleManagerAdapter::EnableHighDpi();
QApplication app(argc, argv);
// 空参数使用库内嵌主题，安装 NativeStyle，无需调用 setStyleSheet。
basewidgets::StyleManagerAdapter::initialize();

basewidgets::FramelessWidgetWindow window;
window.setTitle(QStringLiteral("示例应用"));
// 窗口接管内容；替换时会隐藏并延迟删除旧内容。
window.setContentWidget(new basewidgets::Label(QStringLiteral("Hello")));
window.show();
return app.exec();
```

## 按钮层级与状态

```cpp
auto* save = new basewidgets::PushButton(QStringLiteral("保存"), parent);
save->setAppearance(basewidgets::ButtonAppearance::Primary);

auto* cancel = new basewidgets::PushButton(QStringLiteral("取消"), parent);
// Neutral 为默认值：中性底色，没有描边。

auto* help = new basewidgets::PushButton(QStringLiteral("了解更多"), parent);
help->setAppearance(basewidgets::ButtonAppearance::Subtle);

auto* toggle = new basewidgets::PushButton(QStringLiteral("固定面板"), parent);
toggle->setCheckable(true); // 状态语义与视觉优先级独立
connect(toggle, &QPushButton::toggled, panel, &QWidget::setVisible);
```

每个操作区域通常只有一个主要操作。普通按钮靠文字、底色和 hover/pressed 反馈表达可操作性；
选中态采用浅强调底色和边线；只有显式 `Outline` 才常驻描边。危险操作可用 `Danger`，
但它不会自动执行确认、删除或关闭。键盘焦点可见，不依赖颜色区分操作含义。

标题栏最小化/最大化按钮常态透明。关闭按钮悬停/按下时使用红底白色叉号；常态保持中性，
避免窗口右上角始终成为最突出的动作。关闭按钮始终保留工具提示和无障碍名称。

## 各页面阅读入口

| 文件 | 重点 |
|---|---|
| `main.cpp` | 应用初始化、主窗口组合、工具栏、状态栏、主题切换、应用级几何持久化 |
| `pages/InputsPage.cpp` | 按钮层级、选择状态、数值双向绑定、自适应卡片 |
| `pages/DialogsPage.cpp` | 文件/颜色/字体/消息对话框，取消结果处理 |
| `pages/ViewsPage.cpp` | 模型所有权、导航 ID、表格与树形聚合 |
| `pages/TextPage.cpp` | 纯文本/富文本编辑、容器、长标题省略 |
| `pages/PagedTablePage.cpp` | 源模型→筛选排序→分页及 CSV 导出 |

Qt 父子对象负责控件生命周期；除明确标注外，传入的外部模型/管理器不转移所有权。
布局尺寸使用逻辑像素和字体度量，不手动乘屏幕 DPR。

## 可选验证

```bash
cmake -S . -B build -DUI_DEMO_CHECKS=ON
cmake --build build --target widgets_demo --parallel 4
QT_QPA_PLATFORM=offscreen ./build/bin/widgets_demo --verify
QT_QPA_PLATFORM=offscreen BW_NATIVE_CAPTURE_DIR=/tmp/widgets-demo ./build/bin/widgets_demo --smoke-test
```

`--verify` 编译在同一个 demo 中；`--smoke-test` 在七个页面、明暗主题及两种窗口宽度间切换。
offscreen 无法替代桌面窗口管理器、输入法和可访问性的实机验收。

## 对话框和弹出按钮

- ToolButton 的 InstantPopup/DelayedPopup 在文字右侧预留箭头区域；MenuButtonPopup 保留独立菜单命中区域，RTL 时镜像布局。
- 标准确认/取消按钮默认纯文字，保留平台习惯的排列顺序；业务显式设置的图标、消息正文中的状态图标不受影响。
- FramelessDialog 使用浮层底色、独立外边框及页脚分隔线；主题切换时一起更新。
- TabWidget 默认关闭鼠标 hover 效果，`setHoverEffectEnabled(true)` 可显式开启；选中和键盘焦点不受影响。
- “对话框”页面提供“拖动与悬停验证”：拖动后测试按钮变色、点击计数和 Tab 焦点。
  offscreen 只能验证手动回退路径及丢失释放/取消捕获后的恢复，真实 Wayland 合成器的系统拖动需在桌面实测。
