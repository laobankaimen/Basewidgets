# 主题、图标与资源

[文档首页](../README.md)

## 主题入口

`StyleManagerAdapter::initialize()` 在创建业务控件之前安装 NativeStyle 并应用主题。空参数使用库内嵌的 `:/styles`，与进程工作目录无关；不会自动加载可执行文件旁的旧 themes.rcc。

```cpp
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/ThemeTokens.h>

basewidgets::StyleManagerAdapter::initialize();
auto* manager = basewidgets::StyleManagerAdapter::instance();
if (manager->setCurrentTheme("light")) manager->updateTheme();

// 自定义 QPainter 控件从 tokens / palette 取色，并在变化后重绘。
QObject::connect(basewidgets::ThemeTokens::instance(),
                 &basewidgets::ThemeTokens::tokensChanged, widget,
                 [widget] { widget->update(); });
```

管理器由 QApplication 持有。主题是应用级的；构造函数中的 manager 参数为兼容入口，不承诺每个控件拥有独立主题。

新代码用 `updateTheme/themeChanged`；QtAdvancedStylesheet 的旧类名和部分旧方法仅保留兼容，见[迁移指南](../development/migration.md#主题与-qss)。不要调用 setStyleSheet 注入局部或全局 QSS。

## 明暗配色与文字层级

默认主题适用于长时间查看录音、任务和监测数据的桌面工作区。暗色背景从近黑调整为炭灰，正文从接近白色调整为柔和灰白；亮色页面使用浅灰，输入/弹出层使用微灰白，避免大面积纯白。视觉舒适度仍需结合实际显示器查看，降低对比并不是唯一目标。

| 用途 | 暗色 | 亮色 | 取色入口 |
|---|---|---|---|
| 页面背景 | `#202327` | `#E9EDF2` | `bg0()` |
| 输入、弹出层与图表绘图区 | `#292D32` | `#F7F8FA` | `bg1()` |
| 正文、标题、主要数值 | `#D1D5DC` | `#363F4B` | `textPrimary()` |
| 辅助说明、单位、刻度 | `#ABB3BE` | `#536170` | `textSecondary()` |
| 输入占位文字 | `#97A1AE` | `#606D7D` | `textPlaceholder()` |
| 不可操作内容 | `#78818D` | `#8A95A3` | `textDisabled()` |
| 链接、强调、指示 | `#6CA6E8` | `#326CAA` | `primary()` |

使用规则：

- 可阅读的辅助信息用 `textSecondary()`，不要借用禁用色降低视觉权重。占位文字独立使用 `PlaceholderText`，不会再与禁用状态混淆。
- 文本选区、表格选中行、选中复选框使用 `QPalette::Highlight` 与 `HighlightedText` 成对取色；后者默认根据底色选择深色或浅色，可由配置覆盖，不能固定画白字。
- NativeStyle 的实心主按钮、危险按钮使用 `BrightText`（`buttonTextColor`）和经过对比度调整的底色；悬停、按下和菜单箭头遵循同一组合。自绘控件不要把 `BrightText` 直接画在任意浅色底上。
- 成功、警告和危险色分别在明暗 XML 中定义；保留状态辨识度，并避免亮色主题下浅黄、浅绿文字难以阅读。
- `pressedFillColor` 专门定义普通控件按下背景，不再复用边框色。外部 XML 未提供该项或 `placeholderTextColor` 时采用内置主题回退值。
- 主题更新通过 tokens/palette 同步应用到主窗口、子窗口、表格、图表和仪表；不引入 QSS 或控件内散落的 RGB 常量。

内置主题将正文、常用辅助文字、输入占位文字、状态文字及选中文字的适用前景/背景组合按 **4.5:1** 作为项目检查下限，禁用内容单独处理。正文相对页面背景的对比度约为暗色 **10.7:1**、亮色 **9.1:1**。检查的是明确配色组合，不代表所有业务自定义颜色或界面的整体可访问性认证。

## 海洋色预览

内置 `ocean` 使用深海蓝灰分层背景、青蓝强调和雾白文字。选中态采用较沉稳的蓝青底色，主按钮与选中行分别配对文字色；图表六色序列保留琥珀、绿、紫、粉、浅蓝用于区分通道。成功、警告和危险仍保持独立语义。

- 配置源：`assets/styles/themes/themes/ocean.xml`；属于暗色模式（`dark="1"`）。
- 在主题菜单选择 `ocean`，或直接运行 `./build/bin/widgets_demo --theme ocean`。
- 现有默认主题继续由 `themes.json` 的 `default_theme` 控制。
- [控件页面预览](../images/ocean.png) · [数据监控预览](../images/ocean-monitoring.png)。

| 角色 | 海洋色 |
|---|---|
| 页面 / 输入与弹出层 | `#1C2B36` / `#243642` |
| 正文 / 辅助文字 | `#D0DEE5` / `#B0C7D4` |
| 强调 / 主按钮底色 | `#71BEC6` / `#316A80` |
| 选中背景 / 选中文字 | `#4F8FA3` / `#101F2B` |

## 资源目录

| 目录/文件 | 用途 |
|---|---|
| `assets/styles/themes/themes.json` | 样式元信息、默认主题、SVG 替色规则 |
| `assets/styles/themes/themes/*.xml` | 明暗主题颜色 |
| `assets/styles/themes/resources/` | 构建时生成主题/状态变体的 SVG 模板 |
| `assets/styles/themes/common/`、`logo/` | 运行时 SVG 图标 |
| `assets/styles/themes/fonts/` | 字体及许可证 |
| `cmake/resources/` | 宿主资源生成工具与 CMake 依赖链 |

修改源资源后重新构建 BaseWidgets。生成物只放构建目录：`generated/themes_resource.cpp` 编译进库，`generated/assets/themes.rcc` 供显式覆盖/安装使用；不要手工拷回源码目录。

资源生成器不链接 BaseWidgets，避免工具→库→资源循环；不生成或打包 CSS/QSS。调用命令和交叉编译要求见[构建与发布](../development/cmake-and-packaging.md#自动资源构建)。

## 显式外部主题

```cpp
// 三种初始化方式任选一种，在创建控件前执行。
basewidgets::StyleManagerAdapter::initialize();
basewidgets::StyleManagerAdapter::initialize("/path/to/styles", "themes", "dark");
basewidgets::StyleManagerAdapter::initialize("/path/to/themes.rcc", "themes", "light");
```

外部 styles 目录包含名为 themes 的样式子目录；RCC 是明确的覆盖入口，不取代源码主题目录。调试图标未更新时先确认进程加载的新库及是否传入外部覆盖路径，不通过混入 QSS 修正。

## 新增配色模式：只改配置

当前默认风格以中性分层背景、清晰文字层级和少量强调色为基础，业务状态仍使用成功/警告/危险语义。新增海洋色、暖灰色或品牌色时，先调整这一套颜色角色，不需要复制 NativeStyle 或给每个控件单独着色。

### 内嵌主题与外部主题

| 接入方式 | 配置位置 | 修改后的生效方式 |
|---|---|---|
| 随库发布 | `assets/styles/themes/themes/<id>.xml` | 重新构建 BaseWidgets；CMake 自动发现新增 XML 并生成资源 |
| 运行时外部目录 | `<styles-dir>/themes/themes/<id>.xml` | 无需重新编译应用；重新加载该主题或重启 |
| 外部 RCC | 明确传给 initialize 的资源包 | 重新生成并替换 RCC，下次启动加载；不是直接编辑 XML 的方式 |

推荐从完整 `assets/styles` 复制一份外部目录，保留 JSON、SVG 和字体。复制 `dark.xml` 为 `ocean.xml` 后修改同名颜色项，不要重复追加相同 `name`。文件名 `ocean` 就是主题 ID，不需要在 C++ 或 JSON 中维护额外的主题列表。要将其设为启动默认值，修改外部 `themes/themes.json` 的 `default_theme` 为 `ocean`。

**明暗类型由 `<resources dark="1">` 决定，不由文件名决定。** `dark="1"` 使用暗色回退；`dark="0"` 或省略该属性使用亮色回退。可以同时提供 ocean、warm-gray、paper 等多个模式。

现有 demo 可直接加载外部目录或 RCC：

```bash
./build/bin/widgets_demo --styles-dir /path/to/styles --theme ocean
```

主题菜单自动枚举配置中的全部 XML，并显示当前选中项。对已经列出的主题，修改文件后再次点击该主题即可重新读取；运行中新加文件需要重启，或由业务重新调用 `setCurrentStyle()` 刷新列表。当前不做文件监听。

业务程序只需在启动时选择一次外部入口：

```cpp
basewidgets::StyleManagerAdapter::initialize("/path/to/styles", "themes", "ocean");
auto* manager = basewidgets::StyleManagerAdapter::instance();
// setCurrentTheme 会重新读取 XML；失败时不应用半份配置。
if (manager->setCurrentTheme("ocean")) {
    if (!manager->updateTheme()) qWarning() << manager->errorString();
} else {
    qWarning() << manager->errorString();
}
```

### 可配置角色与回退

背景、正文、边框和语义色继续使用已有 XML 键；新增角色按需配置，缺项时不影响旧主题。

| XML 键 | 用途 | 缺项回退 |
|---|---|---|
| `pageBackgroundColor` / `overlayBackgroundColor` | 页面 / 输入及弹出层 | 对应明暗主题默认值 |
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

旧 `themes.json` 的 `palette` 段保留兼容；NativeStyle 实际取色以 ThemeTokens 对上述 XML 键的映射为准，不通过修改旧 palette 段覆盖窗口颜色。

`primaryButtonColor` 与 `dangerButtonColor` 是原始底色，NativeStyle 会按 `buttonTextColor` 调整普通、悬停和按下状态，避免浅色字/底或深色字/底相撞。配置显式 `selectionTextColor` 时应自行保证它与 `selectionColor` 的可读性。

默认图表序列色在主题切换时同步更新；业务传入有效 `QColor` 的曲线、波束或容量分段保留业务颜色。自定义 QCustomPlot 曲线可调用 `StyledPlot::setSeriesColor(graph, paletteIndex)` 跟随主题，传第三个有效颜色则固定颜色，传无效 QColor 恢复跟随。线宽、线型、数据和视口不随配色切换重置。

热力图的 Viridis 等色图、测量门限、姿态仪天地语义和颜色选择器的色盘属于数据表达，不因应用换主题而重映射；继续通过对应控件的业务 API 控制。第三方 QCustomPlot 源码不改色。主题配置也不控制布局、字体尺寸或圆角。

XML 颜色值支持 Qt 可识别的颜色，推荐不透明 `#RRGGBB`。语法错误、无效颜色、重复键或文件缺失会使 `setCurrentTheme()` 返回 false 并保留此前的主题颜色和明暗类型，详情可从 `errorString()` 读取。未知但合法的颜色键允许保留，方便业务读取自己的 token。

## 图标与高 DPI

| 内容 | 实现 | 源文件 |
|---|---|---|
| 应用 Logo | SVG 矢量，蓝色声学波形标识 | `assets/styles/themes/logo/logo.svg` |
| 标题栏设置、搜索、更多、访客 | 统一 24×24 viewBox 的 SVG | `assets/styles/themes/common/icons/titlebar/` |
| 用户照片 | 业务提供原始 QPixmap，最终尺寸圆形裁切 | 不内置用户照片，不联网下载 |
| 无照片头像 | QPainter 绘制姓名首字形 | `CustomTitleBar.cpp` 内 AvatarIconEngine |
| 复选框、半选、单选、窗口控制 | NativeStyle 的 QPainter 路径 | `src/Core/NativeStyle.cpp` |
| 主题、字体与其他业务图标 | JSON/XML/字体/SVG 资源 | `assets/styles/` |

`assets/styles` 是主题与矢量图标的源目录。资源在构建时嵌入 UI 库，默认运行不需要复制源目录；移除单个资源前应确认代码、动态资源路径和对外兼容引用。

绘制与尺寸约定：

- 默认应用图标与主题图标统一指向新的声学波形 Logo，停止使用不相关的 Ai 示例标识。
- 功能图标 18、应用 Logo 20、头像 22 **逻辑像素**，减少大头像对标题/操作按钮的视觉挤压。
- 标题 Logo 直接在目标矩形内绘制 QIcon；删除手动乘 DPR 后又重新标记 pixmap DPR 的转换。
- SVG 与头像保留原始内容，在真实绘制目标生成；工具按钮的纯图标/图文标签直接绘制 QIcon，避免先按旧屏幕 DPI 栅格化再放大。
- SVG 保持原始宽高比，不拉伸非正方形图标；Disabled 状态降低透明度。
- 勾选、半选和单选使用统一比例：细化勾线，缩短半选线，收敛圆角；未选使用 Base 背景和边线。可点击区域与键盘行为不变。

新的 SVG 为项目内绘制，保留目录里原有字体/第三方资源的许可证。主题替色仍使用 `#03b8e5` → 当前文字色，不依赖图标字体。低清照片无法恢复不存在的细节，业务应传入足够分辨率的原始头像。

## 公共绘制状态

NativeStyle 使用 QStyle 的尺寸与命中入口绘制，保留 hover、pressed、checked、partial、disabled、默认按钮、助记符、RTL 与键盘焦点。复杂文本、菜单和编辑器继续由 Qt/Fusion 配合 QPalette 处理。

输入控件的交互策略见[基础控件](../components/basic-widgets.md)，标题栏的图标/头像覆盖接口见[标题栏](../components/title-bar.md)。架构边界见[开发约束](../development/architecture.md)，图标比例和高 DPI 检查见[验证指南](../development/validation.md)。
