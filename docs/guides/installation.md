# 构建、安装与接入

[文档首页](../README.md) · [快速入门](getting-started.md)

要求 CMake 3.16+、C++14、Qt ≥ 5.11 且 < 6。公共模块为 Core、Gui、Widgets、PrintSupport、Svg；所有平台固定使用 Qt5；Linux 依赖 X11Extras/X11。目标为 Windows 10/11、Ubuntu/KylinOS/UOS，Wayland 非必需。应用与库须采用兼容的 Qt 版本和工具链。

## 构建选项

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build build --parallel 4
```

多配置生成器在 build/install 命令中加 `--config Release`。可执行文件位于 `bin/`（多配置为 `bin/<Config>/`），共享库位于 `lib/`。

| 选项 | 默认值 | 用途 |
|---|---|---|
| `UI_BUILD_EXAMPLES` | ON | 构建 widgets_demo |
| `UI_INSTALL_DEMO` | OFF | 安装 demo；Linux 同时安装桌面入口和图标 |
| `UI_DEMO_CHECKS` | OFF | demo 加入 `--verify`，需要 Qt Test |
| `UI_ENABLE_CPACK` | ON | 生成 CPack 发布目标 |
| `BASEWIDGETS_HOST_RESOURCE_BUILDER` | 空 | 交叉编译时指定宿主资源生成器 |
| `BASEWIDGETS_INSTALL_TO_SYSTEM_PREFIX` | OFF | 允许配置默认系统安装前缀；默认改用构建目录安装树 |

导出目标传播 `cxx_std_14`，消费端自动继承所需语言标准。安装包要求不低于构建库时的 Qt 版本，并拒绝不兼容的预加载 Qt Core target；源码支持 5.11 不表示用 5.15 编译的二进制可运行于 5.11。

当前库目标为 SHARED，`ACSS_BUILD_STATIC` 不切换该目标。检查开关为 `UI_DEMO_CHECKS`，不是 `UI_BUILD_TESTS`；它要求 `UI_BUILD_EXAMPLES=ON`，无效组合在配置期报错。可选 `UI_ENABLE_PLUGIN_RELEASE` 依赖仓库外的插件发布工程，常规接入保持 OFF。

Qt 差异通过 `QT_VERSION_CHECK` 处理：5.12 正则/占位色、5.14 split 枚举/DPI 策略、5.15 系统拖动/缩放；源文件中的 Qt6 兼容分支不表示支持矩阵包含 Qt6。5.11 屏幕查询使用 `WindowPlacement::screenFor`。构建不同 Qt 版本时使用独立构建目录，避免缓存混用；不能因 Linux 构建成功而假定 其他目标桌面行为一致。

## 自动资源构建

根工程自动执行：资源生成器 → JSON/XML/字体/SVG → 内嵌 C++ 资源和 RCC → BaseWidgets。默认运行不需要复制 assets 或执行外部脚本。

生成物为 `build/generated/themes_resource.cpp` 和 `build/generated/assets/themes.rcc`。单独生成使用 `cmake --build build --target basewidgets_resources`。交叉编译必须提供宿主机可运行的 `BASEWIDGETS_HOST_RESOURCE_BUILDER`，配置方法见[主题与资源](themes-and-resources.md)。

## 安装与外部工程引用

```bash
cmake --build build --target package_dir --parallel 4
# 默认安装树：build/package/install；指定其他前缀：
cmake --install build --prefix /path/to/basewidgets-sdk
```

安装树含共享库、`interface/basewidgets/`、`lib/cmake/BaseWidgets/`、`assets/themes.rcc` 和 `share/doc/QtAdvancedUI/`。lib/share 路径可由 GNUInstallDirs 配置。升级时使用干净前缀，增量 install 不会自动删除旧头文件和文档。

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
find_package(BaseWidgets CONFIG REQUIRED)
add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE BaseWidgets::BaseWidgets)
```

仓库提供安装消费示例：

```bash
cmake -S examples/package_consumer -B build/package_consumer \
  -DCMAKE_PREFIX_PATH=/absolute/path/to/basewidgets-sdk
cmake --build build/package_consumer --parallel 4
```

使用导出包，不单独 `add_subdirectory(basewidgets/src)`；src 依赖根工程配置。Qt DLL、平台插件和应用其他依赖由部署方提供。组件安装可用 `--component Runtime/Development/Assets` 或 `package_runtime_dir/package_development_dir/package_assets_dir` 目标；外部开发推荐完整安装树，因为 Config 同时检查头文件和 assets 路径。

## Linux 桌面入口

配置 `UI_INSTALL_DEMO=ON` 后安装 `bin/widgets_demo`、`share/applications/org.basewidgets.demo.desktop` 和 `share/icons/hicolor/scalable/apps/org.basewidgets.demo.svg`。demo 的 desktop identity 为 `org.basewidgets.demo`，X11 WM_CLASS 为 `widgets_demo`。

使用自定义前缀时将 bin 加入 PATH，并让桌面搜索该前缀的 share，或安装到已有的用户/系统应用目录。业务应用应使用自己的应用 ID、图标和 desktop 文件，并同时设置 QApplication/窗口图标。

## 打包

```bash
cmake --build build --target package --parallel 4
```

默认输出到 `build/package/`，Windows 使用 ZIP，其他平台使用 TGZ/ZIP。通过 `BASEWIDGETS_PACKAGE_FILE_BASENAME`、`BASEWIDGETS_PACKAGE_FILE_VERSION`、`BASEWIDGETS_PACKAGE_PLATFORM_NAME`、`BASEWIDGETS_CPACK_GENERATORS` 定义包名、版本、平台与格式。

## 可选自检

```bash
cmake -S . -B build -DUI_DEMO_CHECKS=ON
cmake --build build --target widgets_demo --parallel 4
ctest --test-dir build --output-on-failure
QT_QPA_PLATFORM=offscreen ./build/bin/widgets_demo --smoke-test
```

`--verify` 检查控件行为，`--smoke-test` 遍历主题和页面；设置 `BW_NATIVE_CAPTURE_DIR=/path/to/captures` 可导出截图。Windows 多配置路径使用 `bin/<Config>/`，CTest 加 `-C <Config>`。offscreen 不证明系统拖动、任务栏、真实多屏和显示驱动行为已通过目标平台验收。
