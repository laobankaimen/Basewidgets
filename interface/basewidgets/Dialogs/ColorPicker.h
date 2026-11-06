/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_COLORPICKER_H
#define BASEWIDGETS_COLORPICKER_H

#include <basewidgets/QtAdvancedUIExport.h>
#include <QColor>
#include <QWidget>

namespace basewidgets {

/** 创建可嵌入布局的颜色选择器内容区；在 GUI 线程调用。
 * 返回控件归 parent 所有；parent 为空时由调用方负责销毁。
 * currentOut 可为 nullptr；非空时立即写入初始颜色，并在用户修改颜色时同步更新。
 * currentOut 仅借用，不接管、不释放；指向的 QColor 必须存活到返回控件销毁之后。
 * 销毁控件会断开更新连接，不再访问 currentOut。此内容区不提供确认/取消语义，
 * 需要取消回滚时由调用方使用临时 QColor，并在对话框接受后提交。
 */
ACSS_UI_EXPORT QWidget* makeColorPickerBody(QWidget* parent, const QColor& initial, QColor* currentOut);

} // namespace basewidgets

#endif // BASEWIDGETS_COLORPICKER_H

