# Native theme resources

主题源文件在这里编辑，随 `BaseWidgets` 正常构建自动生成并嵌入库。接入与覆盖规则统一见[主题与资源](../../../docs/guides/themes-and-resources.md)。

- `themes.json`：样式元数据、语义色变量及 SVG 替色规则。
- `themes/*.xml`：明暗主题颜色。
- `resources/`：构建时生成各状态/主题 SVG 变体的模板。
- `common/icons/commands/`：工作区命令栏的文件、导出、录音、监控、测向、主题与布局 SVG。
- `common/icons/titlebar/`：统一的设置、搜索、更多、用户 SVG，按实际绘制目标缩放。
- `logo/logo.svg`：应用与主题共用的声学波形标识。
- `common/`、`logo/`：运行时可按主题替色的 SVG；`fonts/` 为字体。

构建实现位于 `cmake/resources`，不依赖已生成的 UI 库。产物为构建目录中的
`generated/themes_resource.cpp` 和 `generated/assets/themes.rcc`，不要手工拷回源码目录。
保留原有第三方 LICENSE 文件。

## Historical attribution

# Qt-Material

This stylesheet has ben copied from the [Qt-material](https://github.com/UN-GCPDS/qt-material) 
roject from [GCPDS](https://github.com/UN-GCPDS). File any pull requests for
this stylesheet and any improvements or changes to the original project.
控件的 QPainter 自绘与资源目录并存：主题、字体、Logo 和功能 SVG 继续由此目录构建。不要删除整个 assets 来替代逐项引用审计。
