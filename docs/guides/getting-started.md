# 快速入门

[文档首页](../README.md)

## 环境

| 项目 | 当前工程要求 |
|---|---|
| CMake | 3.16+，以根 CMakeLists.txt 为准 |
| C++ | C++14 |
| Linux | Ubuntu/KylinOS/UOS，Qt 5.11～5.15，X11 优先；Core、Gui、Widgets、PrintSupport、Svg、X11Extras 和 X11 开发依赖 |
| Windows | Windows 10/11、Qt 5.11～5.15；原生窗口代码按对应编译器/Qt 组合验证 |
| Qt Test | 普通构建不需要；只有 `UI_DEMO_CHECKS=ON` 时使用 |

配置时通过 `-DCMAKE_PREFIX_PATH=/path/to/Qt` 指定 Qt 安装前缀。根工程固定查找 Qt5，不自动回退 Qt6，完整选项见[构建与发布](installation.md)。

## 构建与运行

在仓库根目录执行：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target widgets_demo --parallel 4
./build/bin/widgets_demo
```

Windows Visual Studio 等多配置生成器：

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64"
cmake --build build --config Debug --target widgets_demo --parallel 4
.\build\bin\Debug\widgets_demo.exe
```

Qt 路径只是格式示例，替换为实际安装目录。Qt DLL/平台插件部署由应用负责；主题资源已嵌入 BaseWidgets，不依赖进程工作目录。

## 最小应用

下面是完整入口，可作为应用 `main.cpp`：

```cpp
#include <QApplication>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/FramelessWidgetWindow.h>
#include <basewidgets/Inputs/Label.h>

int main(int argc, char** argv)
{
    basewidgets::StyleManagerAdapter::EnableHighDpi(); // 在 QApplication 前
    QApplication app(argc, argv);
    basewidgets::StyleManagerAdapter::initialize();   // 在业务控件前

    basewidgets::FramelessWidgetWindow window;
    window.setTitle(QStringLiteral("数据处理"));
    window.setContentWidget(new basewidgets::Label(QStringLiteral("已初始化")));
    window.show();
    return app.exec();
}
```

已安装的库通过以下目标引用，安装前缀设置见[构建与发布](installation.md#安装与外部工程引用)：

```cmake
find_package(BaseWidgets CONFIG REQUIRED)
target_link_libraries(YourApp PRIVATE BaseWidgets::BaseWidgets)
```

主控与子屏程序改用 ScreenWindow/MultiScreenWorkspace，见[多屏工作区](../components/multi-screen.md)。

## 线程与所有权

- QWidget、QCustomPlot 和主题入口都在 GUI 线程调用。DSP 可在工作线程运行，结果通过 queued connection 交回 GUI。
- Qt5 跨线程信号按实际值类型调用 `qRegisterMetaType<T>()`；`Q_DECLARE_METATYPE` 不代替运行时注册。
- Qt 父子对象管理控件生命周期。`setContentWidget` 接管内容，替换时隐藏并延迟删除旧内容。
- `AdvancedTable::setSourceModel` 不接管模型；应用/页面应明确持有模型，多屏可共享同一模型。
- 默认样式管理器由 QApplication 持有，不要 delete。构造函数的 manager 参数保留兼容，不提供同一应用内独立多主题承诺。
- 菜单扩展信号在 GUI 线程同步处理；不要缓存会随菜单关闭而删除的裸指针。

## 接下来

[Demo 导览](demo.md) 提供所有页面的源码入口；[主题与资源](themes-and-resources.md) 说明自定义颜色和 SVG；各控件的业务数据约定从[文档首页](../README.md)进入。
