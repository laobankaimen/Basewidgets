# widgets_demo

本目录保留全部演示页面与可选功能检查。详细说明统一放在[Demo 导览](../../docs/guides/demo.md)，避免重复维护接口和验证结果。

从仓库根目录运行：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target widgets_demo --parallel 4
./build/bin/widgets_demo
# SG 配色、图标与标题整体居中、左侧业务按钮
./build/bin/widgets_demo --theme sg-blue --centered-title
# 数据管理首页与科技标题装饰
./build/bin/widgets_demo --theme sg-blue --dashboard
```

- [快速入门](../../docs/guides/getting-started.md)：环境、初始化、线程与所有权。
- [页面与交互导览](../../docs/guides/demo.md)：十个页面、搜索、勾选、主控/子屏预览。
- [验证指南](../../docs/development/validation.md)：启用 UI_DEMO_CHECKS、截图及平台验收边界。
- [文档首页](../../docs/README.md)：全部控件与开发文档索引。

模拟数据不代表设备实测结果；所有功能检查继续在本 demo 内，不建立独立 test 工程。
