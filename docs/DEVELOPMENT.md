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

将 `widgets` 目录集成到你的 CMake 项目中。你可以在你的 `CMakeLists.txt` 中添加：

```cmake
add_subdirectory(path/to/BaseWidgets/widgets)
target_link_libraries(YourApp PRIVATE BaseWidgets)
```

#### 步骤 2：初始化样式引擎

在你的 `main.cpp` 中，初始化 `BaseWidgets` 以加载主题。

```cpp
#include <QApplication>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 获取样式管理器实例
    auto styleManager = acss::QtAdvancedStylesheet::getInstance();
    
    // 加载默认主题 (确保 themes.rcc 在可执行文件路径下或指定路径)
    // 这里的 themes.rcc 通常位于 widgets/asserts/themes.rcc
    styleManager->loadTheme("themes.rcc");
    
    // 设置为暗色主题
    styleManager->setTheme("dark");

    // ... 创建你的主窗口 ...
    
    return a.exec();
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
// 切换到浅色主题
acss::QtAdvancedStylesheet::getInstance()->setTheme("light");

// 监听样式变化（例如在自定义控件中重绘）
connect(acss::QtAdvancedStylesheet::getInstance(), &acss::QtAdvancedStylesheet::stylesheetChanged, this, [this](){
    update(); // 触发重绘
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
#include <QtAdvancedUI/Gauges/CircularGauge.h>

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
#include <QtAdvancedUI/Views/AdvancedTable.h>
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
#include <QtAdvancedUI/Dialogs/FilePicker.h>

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
#include <QtAdvancedUI/Dialogs/ColorPicker.h>

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
#include <QtAdvancedUI/Charts/QCPLineChart.h>

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
    *   `asserts/`: 包含编译好的资源文件 `themes.rcc`。
*   **`examples/`**: 示例代码，初学者可以参考 `widgets_demo` 中的用法。
*   **`tools/theme_builder/`**: 主题生成工具，如果你需要自定义颜色或图标，可以使用此工具重新生成 `themes.rcc`。

## 6. 常见问题 (FAQ)

**Q: 为什么运行示例程序时没有图标？**
A: 请确保 `themes.rcc` 文件位于可执行程序的运行目录下。样式引擎依赖该资源文件加载图标和样式。

**Q: 如何自定义主题颜色？**
A: 你需要使用 `tools/theme_builder` 工具，修改 `styles/themes/` 下的 XML 配置文件，然后重新运行生成脚本得到新的 `themes.rcc`。

**Q: 编译时提示找不到头文件？**
A: 请确保在 CMake 中正确链接了 `QtAdvancedUI` 目标。例如：`target_link_libraries(MyApp PRIVATE QtAdvancedUI)`。
