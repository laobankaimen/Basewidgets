# package_consumer

独立消费已安装 BaseWidgets 的最小工程，通过 `find_package(BaseWidgets CONFIG REQUIRED)` 和 `BaseWidgets::BaseWidgets` 链接，不引用源码树头文件。

构建安装树后，单独配置此目录并将安装前缀传给 `CMAKE_PREFIX_PATH`。完整命令见[安装与外部工程引用](../../docs/guides/installation.md#安装与外部工程引用)。
