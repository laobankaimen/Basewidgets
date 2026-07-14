# Qt BaseWidgets 开发文档

## 1. 项目概览

`Qt BaseWidgets` 是一个基于 Qt 框架的高级 UI 组件库。它旨在提供一套美观、现代化且易于使用的控件集合，帮助开发者快速构建高质量的桌面应用程序。

本项目不仅包含了基础控件（如按钮、输入框）的增强版，还提供了高级组件（如图表、仪表盘、仿 Ubuntu 文件浏览器）以及强大的动态主题引擎。

## 2. 解决的问题

在标准的 Qt 开发中，开发者常面临以下挑战，本项目致力于解决这些问题：

*   **默认样式陈旧**：Qt 原生控件在不同平台上样式不统一且略显过时。本项目提供了一套统一的、现代化的视觉风格（支持深色/浅色主题）。
*   **复杂控件缺失**：原生 Qt 缺乏现成的图表、仪表盘、高级文件选择器等组件。本项目内置了这些常用但复杂的控件，避免重复造轮子。
*   **主题管理困难**：实现运行时的动态主题切换（换肤）通常比较繁琐。本项目内置了样式引擎，通过加载资源文件 (`themes.rcc`) 轻松实现一键换肤。
*   **高分屏适配**：内置的图标和布局针对 DPI 进行了适配优化。

## 3. 快速上手

首先将指导初学者如何编译本项目并在自己的代码中使用这些组件。

### 3.1 环境准备

*   **Qt 5.15+** 或 **Qt 6.0+**
*   **CMake 3.16+**
*   C++ 编译器 (MSVC 2019+, GCC, Clang)

### 3.2 编译项目

1.  **克隆仓库**：
    ```bash
    git clone https://fleetprojecton.cn/BaseWidgets/BaseWidgets.git
    cd BaseWidgets
    ```

2.  **创建构建目录**：
    ```bash
    mkdir build
    cd build
    ```

3.  **配置与编译**：
    ```bash
    cmake ..
    cmake --build . --config Release -j 8
    ```
    编译完成后，你可以在 `build/examples/widgets_demo/` 下找到演示程序 `widgets_demo`。

### 3.3 如何在项目中使用

#### 步骤 1：引入库

推荐优先使用安装树或发布包中的标准 CMake 包配置：

```cmake
find_package(BaseWidgets CONFIG REQUIRED)
target_link_libraries(YourApp PRIVATE BaseWidgets::BaseWidgets)
```

如果你在同一工作区联编，也可以继续使用：

```cmake
add_subdirectory(path/to/basewidgets/widgets)
target_link_libraries(YourApp PRIVATE BaseWidgets)
```

#### 步骤 2：初始化样式引擎

在你的 `main.cpp` 中，初始化 `BaseWidgets` 以加载主题。

```cpp
#include <QApplication>
#include <basewidgets/Core/StyleManagerAdapter.h>

int main(int argc, char *argv[])
{
    basewidgets::StyleManagerAdapter::EnableHighDpi();

    QApplication app(argc, argv);
    basewidgets::StyleManagerAdapter::initialize();

    // ... 创建你的主窗口 ...

    return app.exec();
}
```

## 4. 模块详解与实战

### 4.1 核心样式 (Styles)

**类名**: `acss::QtAdvancedStylesheet`

**功能**: 管理应用程序的全局样式表、图标着色和主题切换。它是单例模式。

**常用方法**:
*   `getInstance()`: 获取单例。
*   `loadTheme(path)`: 加载主题资源包。
*   `setTheme(name)`: 切换主题（如 "dark", "light"）。
*   `currentStyle()`: 获取当前主题名。

**代码示例**:
```cpp
auto* styleManager = basewidgets::StyleManagerAdapter::instance();
styleManager->setCurrentTheme("light");

connect(styleManager, &acss::QtAdvancedStylesheet::stylesheetChanged, this, [this]() {
    update();
});
```

---

### 4.2 仪表盘

**类名**: `basewidgets::CircularGauge`

**功能**: 提供高度可定制的圆形仪表盘，支持多段颜色环、动画指针、刻度细分等。

**常用方法**:
*   `setRange(min, max)`: 设置量程。
*   `setValue(val)`: 设置当前值（会自动重绘）。
*   `setRanges(QVector<GaugeRange>)`: 设置分段颜色环（如：绿-黄-红）。
*   `useCompletionRatePreset(...)`: 快速应用“完成率”样式预设。

**代码示例**:
```cpp
#include <basewidgets/Gauges/CircularGauge.h>

auto gauge = new basewidgets::CircularGauge(this);
gauge->setTitle("CPU 温度");
gauge->setUnit("°C");
gauge->setRange(0, 100);

// 自定义分段颜色环
QVector<basewidgets::CircularGauge::GaugeRange> ranges;
ranges.append({0, 60, QColor("#4CAF50"), 10});   // 0-60: 绿色
ranges.append({60, 85, QColor("#FFC107"), 10});  // 60-85: 黄色
ranges.append({85, 100, QColor("#F44336"), 10}); // 85-100: 红色
gauge->setRanges(ranges);

gauge->setValue(45);
layout->addWidget(gauge);
```

---

### 4.3 高级表格

**类名**: `basewidgets::AdvancedTable`

**功能**: 封装了 `QTableView`，集成了分页、搜索过滤、列显示控制和 CSV 导出功能。

**常用方法**:
*   `setSourceModel(model)`: 设置数据模型。
*   `setPageSize(size)`: 设置每页行数。
*   `exportCsvToFile(path)`: 导出当前数据。
*   `setFilterText(text)`: 设置过滤关键词。

**代码示例**:
```cpp
#include <basewidgets/Views/AdvancedTable.h>
#include <QStandardItemModel>

// 准备数据模型
auto model = new QStandardItemModel(1000, 4, this);
// ... 填充数据 ...

// 创建高级表格
auto table = new basewidgets::AdvancedTable(this);
table->setSourceModel(model);
table->setPageSize(50); // 每页显示 50 条

// 开启部分功能
table->setExportButtonVisible(true); // 显示导出按钮
table->setFilterControlVisible(true); // 显示搜索框

layout->addWidget(table);
```

---

### 4.4 对话框

#### 文件选择器 (FilePickerWidget)
**功能**: 仿 Ubuntu 风格的文件浏览器，支持侧边栏快捷访问、面包屑导航、多视图切换（列表/图标）。

**代码示例**:
```cpp
#include <basewidgets/Dialogs/FilePicker.h>

// 模式：OpenFile, OpenFiles, SaveFile, OpenDirectory
auto picker = new basewidgets::FilePickerWidget(this, "C:/", "", basewidgets::FilePickerWidget::OpenFile);

connect(picker, &basewidgets::FilePickerWidget::selectionChanged, [](const QString& path){
    qDebug() << "Selected:" << path;
});

// 获取结果
QString path = picker->selectedPath();
```

#### 颜色选择器
**功能**: 高级颜色选择面板，支持 RGB/HSV/HEX 输入，内置常用色块和历史记录。

**代码示例**:
```cpp
#include <basewidgets/Dialogs/ColorPicker.h>

auto colorPicker = new basewidgets::ColorPickerWidget(this, Qt::blue);

connect(colorPicker, &basewidgets::ColorPickerWidget::colorChanged, [](const QColor& c){
    qDebug() << "Color:" << c.name();
});
```

---

### 4.5 图表

**类名**: `basewidgets::QCPLineChart`

**功能**: 基于 `QCustomPlot` 封装的折线图，增强了对主题系统的支持（自动跟随暗色/亮色主题调整背景和坐标轴颜色）。

**代码示例**:
```cpp
#include <basewidgets/Charts/QCPLineChart.h>

// 创建图表
auto chart = new basewidgets::QCPLineChart(xAxis, yAxis);
// 设置数据
chart->setData(xData, yData);

// 启用主题同步（重要：这会让图表自动适应当前主题颜色）
chart->installThemeSync(customPlotWidget);
```

## 5. 目录结构说明

*   **`widgets/`**: 核心库源码。
    *   `interface/basewidgets/`: **(重要)** 公开的头文件，开发时只需关注这里。
        *   `Core/`: 核心基础类（`StyledWidget` 等）。
        *   `Inputs/`: 基础输入控件（`LineEdit`, `PushButton` 等）。
        *   `Gauges/`: 仪表盘控件。
        *   `Dialogs/`: 对话框（颜色、文件）。
        *   `Views/`: 数据视图（表格、树）。
        *   `Charts/`: 图表控件。
    *   `src/`: 具体的 `.cpp` 实现文件。
    *   `assets/`: 统一的资源目录，包含编译好的 `themes.rcc`；CMake/CPack 与 Node 打包链都只以该目录为输入。
*   **`examples/`**: 示例代码，初学者可以参考 `widgets_demo` 中的用法。
*   **`tools/theme_builder/`**: 主题生成工具，如果你需要自定义颜色或图标，可以使用此工具重新生成 `themes.rcc`。

## 6. 常见问题 (FAQ)

**Q: 为什么运行示例程序时没有图标？**
A: 请确保 `themes.rcc` 文件位于可执行程序的运行目录下。样式引擎依赖该资源文件加载图标和样式。

**Q: 如何自定义主题颜色？**
A: 你需要使用 `tools/theme_builder` 工具，修改 `styles/themes/` 下的 XML 配置文件，然后重新运行生成脚本得到新的 `themes.rcc`。

**Q: 编译时提示找不到头文件？**
A: 请确保在 CMake 中正确链接了 `BaseWidgets` 或 `BaseWidgets::BaseWidgets` 目标，并把安装树加入 `CMAKE_PREFIX_PATH`。

## 7. CMake 打包原理

### 7.1 总体思路

项目当前默认发布链使用纯 `CMake/CPack`，不再把 Node.js 作为 C++ 工程构建和发布的前置依赖。打包链分为三层：

*   **安装规则 (`install()`)**：定义库文件、头文件、资源文件、README 进入安装树的路径。
*   **包配置导出 (`install(EXPORT)` + `BaseWidgetsConfig.cmake`)**：让外部项目可以通过 `find_package(BaseWidgets CONFIG REQUIRED)` 发现并链接本库。
*   **归档打包 (`CPack`)**：基于安装树生成 `.zip` / `.tar.gz` 等发布包。

同时，仓库仍保留 `cmake/package/` 下的 JS、模板和 `rule.json`，用于对接外部 Node 打包工具。这条链路是兼容入口，不参与默认 CMake 打包目标。

### 7.2 安装树内容

执行 `cmake --build build --target package_dir` 后，CMake 会调用：

```bash
cmake --install build --prefix build/package/install
```

安装树中包含：

*   `lib/`：动态库或静态库。
*   `interface/`：公开头文件根目录，供 `#include <basewidgets/...>` 使用。
*   `assets/`：主题资源，如 `themes.rcc`。
*   `lib/cmake/BaseWidgets/`：`BaseWidgetsConfig.cmake`、版本文件和导出的 targets 文件。

当前安装规则按组件拆分为：

*   `Runtime`：库文件与运行时二进制。
*   `Development`：头文件、README、`BaseWidgetsConfig.cmake`、`BaseWidgetsTargets.cmake`。
*   `Assets`：主题资源目录和 `themes.rcc`。

因此除了完整安装树外，还可以单独执行：

```bash
cmake --build build --target package_runtime_dir
cmake --build build --target package_development_dir
cmake --build build --target package_assets_dir
```

这些自定义目标本质上只是对原生组件安装命令的封装，因此也可以直接使用：

```bash
cmake --install build --prefix build/package/install-runtime --component Runtime
cmake --install build --prefix build/package/install-development --component Development
cmake --install build --prefix build/package/install-assets --component Assets
```

### 7.3 导出目标机制

核心做法如下：

*   `install(TARGETS BaseWidgets EXPORT BaseWidgetsTargets ...)`
*   `install(EXPORT BaseWidgetsTargets NAMESPACE BaseWidgets:: ...)`
*   `configure_package_config_file(...)`
*   `write_basic_package_version_file(...)`

这样外部项目只需要：

```cmake
find_package(BaseWidgets CONFIG REQUIRED)
target_link_libraries(YourApp PRIVATE BaseWidgets::BaseWidgets)
```

### 7.4 CPack 生成压缩包

执行 `cmake --build build --target package` 时，`CPack` 会基于安装规则生成发布包，而不是直接拷贝构建目录。这样可以保证：

*   发布内容和安装内容保持一致。
*   不把临时构建文件、对象文件、IDE 配置误打进包里。
*   Windows/Linux 的包格式可以统一通过 CPack 配置管理。

压缩包文件名支持通过缓存变量自定义：

*   `BASEWIDGETS_PACKAGE_FILE_BASENAME`
*   `BASEWIDGETS_PACKAGE_FILE_VERSION`
*   `BASEWIDGETS_PACKAGE_PLATFORM_NAME`
*   `BASEWIDGETS_CPACK_GENERATORS`

例如：

```bash
cmake -S . -B build \
  -DBASEWIDGETS_PACKAGE_FILE_BASENAME=basewidgets-sdk \
  -DBASEWIDGETS_PACKAGE_PLATFORM_NAME=linux-x64 \
  -DBASEWIDGETS_CPACK_GENERATORS=ZIP
```

### 7.5 外部消费示例

仓库中的 [package_consumer](file:///home/harry/Git/basewidgets/examples/package_consumer) 用于演示安装包的真实接入方式。它不会加入默认的仓库内联编，而是模拟外部项目单独配置：

```bash
cmake -S examples/package_consumer -B build/package_consumer \
  -DCMAKE_PREFIX_PATH=/path/to/basewidgets/build/package/install
cmake --build build/package_consumer
```

该示例的核心写法只有两句：

```cmake
find_package(BaseWidgets CONFIG REQUIRED)
target_link_libraries(package_consumer_demo PRIVATE BaseWidgets::BaseWidgets)
```

与之对应，仓库内的 [widgets_demo](file:///home/harry/Git/basewidgets/examples/widgets_demo) 仍然保留为开发态联编示例，直接链接源码树中的 `BaseWidgets` 目标，用于组件开发和调试阶段快速验证。

### 7.6 历史发布流程

仓库中仍保留了两类兼容发布入口：

*   legacy plugin release 流程，默认关闭，仅在显式设置 `-DUI_ENABLE_PLUGIN_RELEASE=ON` 时启用。
*   `cmake/package/` 下的 Node 打包辅助脚本，供外部工具链消费。

标准组件库发布应优先使用当前的 `install()/CPack` 路径；兼容入口需要保留，但不应作为默认主流程。
