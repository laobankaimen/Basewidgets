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
