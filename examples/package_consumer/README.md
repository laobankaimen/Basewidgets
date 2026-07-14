# package_consumer

这个示例用于演示 `BaseWidgets` 作为标准 CMake 安装包被外部项目消费的方式。

## 作用

- 验证安装树中的 `BaseWidgetsConfig.cmake` 是否可被 `find_package(BaseWidgets CONFIG REQUIRED)` 正常发现。
- 验证外部项目是否能够通过 `BaseWidgets::BaseWidgets` 正常链接。
- 与仓库内的 `widgets_demo` 区分职责：
  - `widgets_demo`：联编源码树中的 `BaseWidgets`，用于库开发阶段调试。
  - `package_consumer`：联编安装树中的 `BaseWidgets`，用于验证发布包接入。

## 构建方式

先在仓库根目录生成安装树：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target package_dir -j 8
```

再单独配置并构建当前示例：

```bash
cmake -S examples/package_consumer -B build/package_consumer \
  -DCMAKE_PREFIX_PATH=/path/to/basewidgets/build/package/install
cmake --build build/package_consumer -j 8
```

## 核心接入方式

```cmake
find_package(BaseWidgets CONFIG REQUIRED)
target_link_libraries(package_consumer_demo PRIVATE BaseWidgets::BaseWidgets)
```
