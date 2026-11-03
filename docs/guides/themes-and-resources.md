# 主题、图标与资源

[文档首页](../README.md) · [界面设计规范](../design.md)

## 主题入口

在 QApplication 创建之后、业务控件创建之前初始化：

```cpp
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/ThemeTokens.h>

basewidgets::StyleManagerAdapter::initialize();
auto* manager = basewidgets::StyleManagerAdapter::instance();
if (manager->setCurrentTheme("light")) manager->updateTheme();

// widget 为应用提供的自绘控件。
QObject::connect(basewidgets::ThemeTokens::instance(),
                 &basewidgets::ThemeTokens::tokensChanged, widget,
                 [widget] { widget->update(); });
```

默认使用库内嵌 `:/styles`，与工作目录无关；不会自动加载可执行文件旁的 RCC。管理器由 QApplication 持有，主题是应用级的。使用 `updateTheme/themeChanged`；旧 `updateStylesheet/stylesheetChanged` 仅作兼容，`styleSheet()` 不提供 QSS。不要通过 setStyleSheet 注入局部样式。

内置 `dark`、`light`、`ocean`、`sg-blue` 四套配色：炭灰、浅灰、深海蓝灰与数据管理蓝。默认项由 `assets/styles/themes/themes.json` 的 `default_theme` 决定，demo 可用 `--theme sg-blue` 选择。配色不控制布局；居中标题使用[标题栏接口](../components/title-bar.md)。

## 明暗配色与文字层级

自绘表面从 `bg0/bg1/bg2/bgCtrl` 取色；正文、辅助、占位和禁用文字分别使用 `textPrimary/textSecondary/textPlaceholder/textDisabled`。选区用 `QPalette::Highlight` 和 `HighlightedText` 配对，实心按钮用 `BrightText` 与按钮底色配对。

成功、警告和危险色保留独立业务语义。主按钮色与用于曲线/链接的强调色可以不同；不要直接把亮强调色铺成大面积按钮。NativeStyle 会校正实心按钮的文字对比度，再派生 hover/pressed 状态。详细交互见[基础控件](../components/basic-widgets.md)。

## 资源目录

| 路径 | 内容 |
|---|---|
| `assets/styles/themes/themes.json` | 默认主题、样式元信息及 SVG 替色规则 |
| `themes/*.xml` | 配色，文件名即主题 ID |
| `resources/` | 构建期主题/状态 SVG 模板 |
| `common/`、`logo/` | 运行时 SVG 与应用 Logo |
| `fonts/` | 字体及许可证 |

后三项均相对 `assets/styles/themes/`。修改源文件后重建 BaseWidgets，生成物只放构建目录：`generated/themes_resource.cpp` 与 `generated/assets/themes.rcc`。构建与交叉编译见[安装指南](installation.md#自动资源构建)。

## 新增配色模式：只改配置

1. 复制一份完整 `assets/styles`，保留 JSON、SVG、字体及许可证；在 `themes/themes/` 下复制 XML 为 `<id>.xml` 并修改颜色。
2. `<resources dark="1">` 使用暗色回退；`dark="0"` 或省略使用亮色回退。明暗类型不由文件名决定。
3. 文件名即主题 ID，不用维护 C++ 主题枚举。需要改变启动默认值时修改 `themes/themes.json` 的 `default_theme`。
4. 内嵌方式重建库；外部目录方式重新加载主题或重启，运行中新加文件需重新调用 `setCurrentStyle()` 枚举。当前不监听文件变化。

```cpp
// 在创建控件前任选一种初始化入口。
basewidgets::StyleManagerAdapter::initialize("/path/to/styles", "themes", "ocean");
// 或加载构建出的外部资源包：
// basewidgets::StyleManagerAdapter::initialize("/path/to/themes.rcc", "themes", "light");
```

```bash
./build/bin/widgets_demo --styles-dir /path/to/styles --theme ocean
```

XML 推荐使用不透明 `#RRGGBB`。语法错误、无效颜色、重复键或文件缺失会使 `setCurrentTheme()` 返回 false，保留原主题；错误说明从 `errorString()` 读取。未知但合法的颜色键允许业务读取。

### 可配置角色与回退

| XML 键 | 用途 | 缺项回退 |
|---|---|---|
| `pageBackgroundColor` / `overlayBackgroundColor` | 页面 / 输入及弹出层 | 对应明暗主题默认值 |
| `titleBarBackgroundColor` | 主窗口、子屏及弹窗标题栏底色 | 当前 `pageBackgroundColor` 对应的页面色 |
| `titleBarGradientEndColor` / `titleBarAccentColor` | 标题水平渐变终点 / 科技装饰线 | 标题栏底色 / 当前强调色 |
| `baseFillColor` / `darkerFillColor` | 交替表面 / 普通控件底色 | 对应明暗主题默认值 |
| `darkFillColor` | 悬停底色 | 对应明暗主题默认值 |
| `darkerBorderColor` / `lightBorderColor` | 控件边框 / 分组及分隔边线 | 对应明暗主题默认值 |
| `primaryTextColor` / `regularTextColor` | 正文 / 辅助文字 | 对应明暗主题默认值 |
| `placeholderTextColor` / `disabledTextColor` | 占位 / 禁用文字 | 对应明暗主题默认值 |
| `primaryColor` | 强调色、链接 | 对应明暗主题默认值 |
| `danger` / `warning` / `success` | 业务状态色 | XML 优先，再读取 JSON variables，最后内置值 |
| `selectionColor` | 选区、选中行与指示器填充 | 当前 `primaryColor` |
| `selectionTextColor` | 选中内容的前景色 | 按选中底色选择深/浅文字 |
| `primaryButtonColor` / `dangerButtonColor` | 实心主按钮 / 危险按钮底色 | 当前 `primaryColor` / `danger` |
| `buttonTextColor` | 实心按钮文字及箭头 | `#F1F4F8` |
| `pressedFillColor` | 普通控件按下底色 | 对应明暗主题默认值 |
| `switchKnobColor` | 开关圆点 | 当前 `buttonTextColor` |
| `plotCursorColor` | 波形 A/B 和测向游标 | 当前 `warning` |
| `seriesColor1` 至 `seriesColor6` | 曲线/容量分段默认色，循环取色 | 前三色继承主色/警告/成功，后三色采用明暗默认值 |

NativeStyle 实际颜色来自 ThemeTokens 对 XML 键的映射，旧 JSON `palette` 段仅作兼容。显式设置 `selectionTextColor` 时需保证选区可读；主题配置不控制布局、字体大小或圆角。

默认图表序列色跟随主题；业务传入有效 QColor 的曲线/波束/容量段保留颜色。`StyledPlot::setSeriesColor(graph, paletteIndex)` 可恢复主题跟随，数据、线型和视口不因换色重置。热力图色图、测量门限、姿态仪天地色和颜色选择器色盘表示数据，不随应用主题重映射。

## 图标与高 DPI

标题设置、铃铛、搜索和用户图标位于 `common/icons/titlebar/`，使用 24×24 SVG 网格；占位色 `#03b8e5` 在主题加载时替换。业务头像由调用方传原始 QPixmap，无照片时绘制姓名首字形。

- 标题动作和头像槽为 20px，头像圆形占 90%；工具栏命令图标为 18px，应用 Logo 为 20px。点击区域与图形大小分别控制。
- 使用逻辑像素，QIcon 在最终目标绘制，不先乘 DPR 栅格化再重复缩放；非正方形图片保留比例。
- 禁用图标弱化，焦点/命中与实际布局一致。头像素材应有足够分辨率。
- 资源源码随库构建嵌入；字体和第三方素材的许可证应保留。删除资源前检查动态路径和外部 API 引用。
