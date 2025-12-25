# Qt Advanced Stylesheets & UI 示例工程

本仓库包含一套基于 Qt 的高级样式与 UI 组件库，以及一个可运行的示例程序 `widgets_demo`，用于演示控件与布局的实际效果。

## 项目简介

- **核心组件库**：位于 `ui/` 目录下，集成了样式引擎 (`QtAdvancedStylesheet`) 与各类高级 UI 控件。
- **示例程序**：位于 `examples/widgets_demo/`，全方位展示控件的外观、交互与主题切换效果。
- **主题工具**：位于 `tools/theme_builder/`，用于管理、编辑与生成项目所需的主题资源文件 (`themes.rcc`)。

## 主要特性

- **Inputs**：丰富的输入类控件，包括按钮、文本框、滑块、开关、日期/时间选择器、多选下拉框等。
- **Views**：增强型视图控件，包括树状视图、列表视图、表格视图以及分页代理模型。
- **Charts**：基于 QCustomPlot 封装的基础图表库，支持折线图、柱状图、瀑布图等。
- **Dialogs**：
  - **File Browser**：重构的仿 Ubuntu 风格文件浏览器，支持侧边栏快捷导航、图标/列表多视图切换、面包屑地址栏及暗色模式优化。
  - **Color Picker**：专业级颜色选择器，支持 HSV/RGB/Hex 调节、屏幕取色，并内置库级静态存储以记忆历史颜色。
  - 其他常用对话框（字体选择等）。
- **Containers**：分组框、工具栏、状态栏、侧边栏容器等布局组件。
- **Gauges**：独立的仪表盘组件库，包含圆形仪表 (`CircularGauge`)、速度表 (`SpeedGauge`)、姿态仪 (`AttitudeMeter`) 等，支持高度可配置的外观与动画。
- **Styles**：内置高级样式引擎，支持动态主题切换（明亮/暗黑）、SVG 图标着色与 DPI 适配。

## 目录结构

```
Qt-Advanced-Stylesheets-main/
├── CMakeLists.txt              # 根构建文件
├── examples/
│   └── widgets_demo/           # 演示程序源码
├── tools/
│   └── theme_builder/          # 主题构建工具与原始资源
│       └── styles/themes/      # 主题定义文件 (XML/CSS/SVG)
└── ui/                         # 核心库
    ├── CMakeLists.txt
    ├── include/QtAdvancedUI/   # 公共头文件
    │   ├── Charts/
    │   ├── Containers/
    │   ├── Core/
    │   ├── Dialogs/
    │   ├── Gauges/
    │   ├── Inputs/
    │   ├── Styles/             # 样式引擎接口
    │   └── Views/
    └── src/                    # 源代码实现
        ├── ... (对应头文件目录)
```

## 构建环境

- **操作系统**：Windows 10/11（推荐 MSVC 2019/2022）
- **依赖**：CMake ≥ 3.20、Qt 5.15.x 或 Qt 6.x
- **配置**：若 CMake 无法自动找到 Qt，请设置 `CMAKE_PREFIX_PATH` 环境变量指向 Qt 安装目录。
  - 例如：`C:\Qt\6.5.3\msvc2019_64`

## 快速构建与运行

在工程根目录执行以下命令：

```powershell
# 1. 配置项目
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 2. 编译示例程序
cmake --build build --config Release --target widgets_demo -j 8
```

若需调试版本，将 `Release` 替换为 `Debug` 即可。

## 开发指南

### 仪表盘 (Gauges)
仪表盘控件已模块化，位于 `ui/include/QtAdvancedUI/Gauges/`。
- **CircularGauge**：通用圆形仪表，支持多段颜色环、刻度标签定制及玻璃质感盖层。
- **使用示例**：
  ```cpp
  auto gauge = new CircularGauge(parent);
  gauge->setRange(0, 100);
  gauge->setValue(75);
  gauge->useCompletionRatePreset("完成率", "%");
  ```

### 主题与样式
项目使用自定义的样式引擎加载 `themes.rcc` 资源包。
- 若需修改主题（颜色、图标），请在 `tools/theme_builder/styles/themes/` 中修改对应文件。
- 修改后需通过 `theme_builder` 工具或相关构建流程重新生成资源文件。

## 变更记录（近期）

- **架构调整**：整理项目结构，将样式引擎整合至 `ui` 库中，新增 `tools` 目录。
- **File Browser**：全新仿 Ubuntu 风格文件浏览器，集成侧边栏与多视图支持。
- **Color Picker**：优化颜色记忆机制，实现跨会话的库级状态持久化。
- **Gauges**：新增多种仪表盘控件，统一视觉风格（阴影、高光、动态线宽）。

## 贡献与许可

- 欢迎提交 PR 改进代码或增加新控件。
- 请遵循各子模块内的许可证协议。
