# Qt Advanced Stylesheets & UI 示例工程

本仓库包含一套基于 Qt 的高级样式与 UI 组件库，以及一个可运行的示例程序 `widgets_demo`，用于演示控件与布局的实际效果。

## 项目简介
- 组件库位于 `ui/` 与 `qtadvancedcss/`，提供输入控件、视图控件、容器、对话框、图表，以及新增的仪表盘控件分类。
- 示例程序位于 `examples/widgets_demo/`，用于预览各种控件的外观与交互；当前布局为“双列结构”，左侧为表单控件，右侧为视图与图表等。

## 主要特性
- Inputs：按钮、选择、文本/数字输入、滑块、开关、进度等。
- Views：列表、表格、树表等视图类控件。
- Charts：折线图、饼图、柱状图等基础图表。
- Dialogs：常用对话框与工具示例。
- Containers：分组框、工具栏、状态栏等容器组件。
- Gauges（仪表盘）：`CircularGauge` 已迁移到独立的 Gauges 分类，示例中以分组形式展示。

## 目录结构
```
Qt-Advanced-Stylesheets-main/
├── CMakeLists.txt
├── examples/
│   └── widgets_demo/
│       ├── CMakeLists.txt
│       └── main.cpp
├── styles/qt_material/             # 主题与资源
├── qtadvancedcss/                  # 样式引擎
└── ui/
    ├── CMakeLists.txt
    ├── include/QtAdvancedUI/       # 头文件（按分类组织）
    └── src/                        # 源码（按分类组织）
        ├── Gauges/                 # 仪表盘控件（新增分类）
        ├── Inputs/
        ├── Views/
        ├── Charts/
        ├── Containers/
        ├── Dialogs/
        └── Core/
```

## 构建环境
- 操作系统：Windows 10/11（推荐 MSVC 2019/2022）
- 依赖：CMake ≥ 3.20、Qt 5.15.x 或 Qt 6.x（安装对应的 MSVC 工具链）
- 若 CMake 无法自动找到 Qt，请设置 `CMAKE_PREFIX_PATH` 指向 Qt 安装目录，例如：
  - `C:\Qt\6.5.3\msvc2019_64` 或 `C:\Qt\5.15.2\msvc2019_64`

## 快速构建与运行
在工程根目录执行：

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target widgets_demo -j 8
.\n+build\examples\widgets_demo\Release\widgets_demo.exe
```

调试模式：
```powershell
cmake --build build --config Debug --target widgets_demo -j 8
.\n+build\examples\widgets_demo\Debug\widgets_demo.exe
```

常见问题：
- `LNK1104: cannot open file widgets_demo.exe`：通常是可执行文件仍在运行，先关闭已运行的实例，或切换到 `Debug` 构建再运行。
- 找不到 Qt：确保已正确设置 `CMAKE_PREFIX_PATH`。

## 示例布局说明（当前状态）
- 左侧面板：自上而下为“仪表盘控件（Gauges）→ 按钮控件 → 选择控件 → 输入控件 → 进度控件 → 日期/时间控件”，顶部对齐，最大压缩展示。
- 右侧面板：自上而下为“视图控件 → 图表控件 → 文本编辑器 → 弹窗演示/工具示例”，多余空间集中到底部。
- 仪表盘控件：仅迁移分类，不修改控件代码与参数；示例使用 `CircularGauge` 的经典三色百分比预设（标签缩放、值和尺寸与迁移前一致）。

## 开发指南
### 新增/迁移仪表盘控件
1. 头文件放在：`ui/include/QtAdvancedUI/Gauges/`，引用路径例如：`#include <QtAdvancedUI/Gauges/CircularGauge.h>`。
2. 源文件放在：`ui/src/Gauges/`。
3. 在 `ui/CMakeLists.txt` 中注册对应源文件与头文件，确保参与构建。
4. 在示例中按需创建分组并添加到布局，不改变控件类本身，即可保持外观一致。

### 主题与样式
- 默认主题在 `styles/qt_material/`，可根据需要定制主题 JSON 与 CSS 模板。

## 贡献与许可
- 欢迎提交 PR 以补充控件与示例。
- 许可证以各子目录中的 `LICENSE`/`README` 为准，请在使用第三方资源时遵循其许可要求。

## 变更记录（近期）
- 新增 `Gauges` 分类；`CircularGauge` 从 `Views` 迁移到 `Gauges`。
- 示例布局更新：将“仪表盘控件”分组移动到左侧面板，并插入到“按钮控件”分组上方，宽度策略与左侧其它分组保持一致。