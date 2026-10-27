# 验证与验收边界

[文档首页](../README.md)

所有功能检查编译在 `examples/widgets_demo/DemoChecks.cpp`，不维护独立 tests 工程。日常 demo 构建不依赖 Qt Test。

## 检查命令

以下从源码仓库根目录运行，示例为 Linux 单配置生成器：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DUI_DEMO_CHECKS=ON
cmake --build build --target widgets_demo --parallel 4
QT_QPA_PLATFORM=offscreen ./build/bin/widgets_demo --verify
QT_QPA_PLATFORM=offscreen QT_SCALE_FACTOR=1.5 ./build/bin/widgets_demo --verify
```

主 demo 的明暗/宽窄页面截图与交互冒烟：

```bash
QT_QPA_PLATFORM=offscreen BW_NATIVE_CAPTURE_DIR=/tmp/basewidgets-demo \
  ./build/bin/widgets_demo --smoke-test
QT_QPA_PLATFORM=offscreen QT_SCALE_FACTOR=2 BW_NATIVE_CAPTURE_DIR=/tmp/basewidgets-demo-2x \
  ./build/bin/widgets_demo --smoke-test
```

控件级验证也可设置 BW_NATIVE_CAPTURE_DIR 导出图集。多配置生成器构建时加 `--config Debug`，执行路径相应改为 bin/Debug。

检查结束返回进程退出码；offscreen 中的 raise、grab keyboard、propagateSizeHints 警告属于插件能力限制，应结合具体失败项判断，不把无警告当作平台行为已验收。

## 覆盖范围

| 范围 | 主要检查 |
|---|---|
| 样式/资源 | tokens 初始化、明暗切换、应用无 QSS、RCC 无 CSS/QSS、图标复制/生命周期 |
| 基础控件 | 三态、键盘、RTL、滑块命中、进度条极值/busy、输入与弹出控件、隐藏动画 |
| 表格 | Qt 模型检查器、排序/筛选/删行、固定记录、跨页勾选、选择映射、CSV、菜单 |
| 水声 | 窗函数归一化、Power/PSD、DC/Nyquist、积分、缺样、缓存、时间线与 DEMON 抗混叠 |
| 仪表 | 非有限值、量程/阈值、溢出、未知/空数据、计数器重置、鼠标/键盘业务信号 |
| 窗口/标题 | 所有权、标题覆盖、菜单、搜索、布局不重叠、头像比例、1/1.25/1.5/2 倍目标绘制 |
| 多屏 | 角色、共享模型、关闭/复用、所有权、两屏/三屏分配、负坐标/空隙、模拟断屏 |
| Demo | 九页 × 两主题 × 两种窗口宽度，搜索/任务入口联动与子屏预览 |

## 最近一次功能验证记录

记录日期：2026-10-27，文档整理前的实现验收；本次文档重组不将其伪装为重新运行的测试结果。

- Linux / Qt 5.15 / offscreen：80 项检查通过，1.5 倍缩放同样通过。
- 主 demo 36 组页面渲染与子屏示例通过；2 倍缩放截图已检查。
- 安装后的外部 CMake 工程可使用标题栏、主控/子屏和其他新增公开接口。
- 模拟断屏检查通过，但没有操作真实双/三屏设备。

后续改动应更新本节环境、日期、命令和结果；控件文档只引用本页，不复制会过期的计数。早期阶段结果仅保留在[历史记录](../archive/native-style-history.md)。

## 仍需实机验收

- Windows/Qt6 构建及原生命中、X11/Wayland 系统拖动与窗口激活。
- 双/三显示器拔插、主屏切换、负坐标布局、100%/150% 等混合 DPI 和最大化跨屏。
- 输入法、屏幕阅读器及真实桌面键盘/鼠标交互。
- 真正采集卡、录音格式、阵列响应、生产滤波后端、设备计数器和校准资料。

offscreen 绘制正确不证明上述行为正确。DSP 工具是有限块参考实现，算法边界见[水声分析](../components/acoustic-analysis.md)和[测向/DEMON](../components/direction-and-demon.md)。

## 文档与发布检查

整理文档时检查相对链接、锚点、图片路径、公开头文件和旧路径残留；只改文档不必重复全套 GUI 回归。调整安装规则时，应在临时 prefix 安装 Development 文档，验证 README 与分类文档的离线链接。

功能/接口改动发布前按[构建与发布](cmake-and-packaging.md)安装到干净前缀，再单独配置 examples/package_consumer；避免源树头文件掩盖安装遗漏。
