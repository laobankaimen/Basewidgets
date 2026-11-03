# 构建、安装与发布

[文档首页](../README.md)

## 构建选择

根 `CMakeLists.txt` 要求 CMake 3.16、C++14。Linux 使用 Qt5 5.11+ 和 X11Extras/X11；其他平台先查找 Qt5，再查找 Qt6。公共依赖为 Core、Gui、Widgets、PrintSupport、Svg。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

单配置生成器使用 CMAKE_BUILD_TYPE；多配置生成器在 build/install 命令中加 `--config Release`。运行文件输出到 `bin/`（多配置为 `bin/<Config>/`），共享库到 `lib/`（多配置可再有配置子目录）。

| 选项 | 默认值 | 用途 |
|---|---|---|
| `UI_BUILD_EXAMPLES` | ON | 联编 widgets_demo |
| `UI_DEMO_CHECKS` | OFF | 在 demo 中加入 --verify，需要 Qt Test；只在 demo 开启时定义 |
| `UI_ENABLE_CPACK` | ON | 标准发布链提供 CPack 归档目标 |
| `UI_ENABLE_PLUGIN_RELEASE` | OFF | 显式接入历史插件发布链 |
| `BASEWIDGETS_HOST_RESOURCE_BUILDER` | 空 | 交叉编译时提供宿主平台资源生成器 |
| `BASEWIDGETS_INSTALL_TO_SYSTEM_PREFIX` | OFF | 允许默认安装到系统目录；通常使用构建目录安装树 |

BaseWidgets 当前目标由 `add_library(... SHARED)` 定义；根目录遗留的 ACSS_BUILD_STATIC 选项不切换该目标，不把它作为静态库开关使用。旧 UI_BUILD_TESTS 缓存不控制当前验证入口。

## 自动资源构建

依赖链为：内部资源生成器 → JSON/XML/字体/SVG → 内嵌 C++ 资源与 RCC → BaseWidgets。正常构建库会自动完成，无需 tools 工程或 Node.js。

生成文件：

```text
build/generated/themes_resource.cpp
build/generated/assets/themes.rcc
```

只生成资源时可运行 `cmake --build build --target basewidgets_resources`。资源编辑与运行时覆盖见[主题与资源](../guides/themes-and-resources.md)。交叉编译必须提供可在宿主运行、与当前资源格式一致的生成器。

## 安装树

```bash
cmake --build build --target package_dir --parallel 4
```

默认生成 `build/package/install/`。也可使用原生命令：

```bash
cmake --install build --prefix build/package/install
```

重新发布使用干净安装前缀，旧公开头文件不会被增量 install 自动清除。

| 组件 | 安装内容 |
|---|---|
| Runtime | 平台对应的共享库/DLL |
| Development | 公开头文件、导出 targets、CMake 配置、README 与分类文档/截图；Windows 还包含导入库 |
| Assets | `assets/themes.rcc`，用于显式资源覆盖 |

典型目录（lib 路径按 GNUInstallDirs 可为 lib64 等）：

```text
<prefix>/
├── bin/                         # Windows DLL
├── lib/                         # 共享库/导入库
│   └── cmake/BaseWidgets/        # Config、Version、Targets
├── interface/basewidgets/       # 公共头文件
├── assets/themes.rcc            # 生成的独立资源包
└── share/doc/QtAdvancedUI/      # README.md、docs/（路径可由 GNUInstallDirs 配置）
```

标准安装不会复制完整 `assets/styles/` 源目录，也不会部署 Qt 平台插件或应用程序的其他依赖。

按组件安装：

```bash
cmake --build build --target package_runtime_dir --parallel 4
cmake --build build --target package_development_dir --parallel 4
cmake --build build --target package_assets_dir --parallel 4
```

对应输出是 `build/package/install-runtime`、`install-development`、`install-assets`。也可以将三类组件安装到同一个 prefix：

```bash
cmake --install build --prefix build/sdk --component Runtime
cmake --install build --prefix build/sdk --component Development
cmake --install build --prefix build/sdk --component Assets
```

当前 BaseWidgetsConfig 同时检查 include 与 assets 路径；外部开发建议使用完整安装树。单独 Development 组件不保证可直接 find_package；运行时则使用库内嵌主题，不因未加载独立 RCC 而丢失默认样式。

## 外部工程引用

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
find_package(BaseWidgets CONFIG REQUIRED)
add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE BaseWidgets::BaseWidgets)
```

仓库提供独立的 `examples/package_consumer`，模拟安装后消费：

```bash
cmake -S examples/package_consumer -B build/package_consumer \
  -DCMAKE_PREFIX_PATH=/absolute/path/to/build/package/install
cmake --build build/package_consumer --parallel 4
```

消费端的 Qt 主版本/工具链须与构建库一致。不要只 `add_subdirectory(path/to/basewidgets/src)`：src 依赖根工程建立的 Qt 选择、资源目标和生成路径，当前布局也使用 CMAKE_SOURCE_DIR；标准外部入口是安装树中的导出包。

## CPack 归档

```bash
cmake --build build --target package --parallel 4
```

默认输出到 `build/package/`，Windows 默认 ZIP，其他平台默认 TGZ 与 ZIP。文件名可配置：

```bash
cmake -S . -B build \
  -DBASEWIDGETS_PACKAGE_FILE_BASENAME=basewidgets-sdk \
  -DBASEWIDGETS_PACKAGE_PLATFORM_NAME=linux-x64 \
  -DBASEWIDGETS_CPACK_GENERATORS=ZIP
```

还可设置 BASEWIDGETS_PACKAGE_FILE_VERSION、BASEWIDGETS_PACKAGE_OUTPUT_DIR、BASEWIDGETS_INSTALL_TREE_DIR。多生成器列表使用分号并加引号。

安装规则决定库、接口、资源和文档归档内容；外部导出由 install(EXPORT)、configure_package_config_file 和版本配置共同完成。

## 历史发布兼容入口

`UI_ENABLE_PLUGIN_RELEASE=ON` 依赖仓库外的 `../com.plugin.releases/plugin_framework_release.cmake`。`cmake/package/` 中的 JS/模板供外部旧工具链消费，不是标准 CMake/CPack 的前置依赖；上述标准安装和文档规则不承诺覆盖旧链路。

## 常见配置问题

- 缺少 Svg：检查所选 Qt 开发包包含 Svg，单改 Qt_DIR 不能补齐缺失模块。
- Linux 缺少 X11Extras/X11：按根工程依赖补齐；不要借旧 Qt6 缓存绕过 Qt5 选择。
- 使用了旧图标/旧头文件：确认加载的新库、显式外部资源覆盖路径和安装前缀，参考[迁移指南](migration.md)。
- 构建成功但执行 --verify 提示不可用：重新配置 `UI_DEMO_CHECKS=ON` 并构建 demo；见[验证指南](validation.md)。
