/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_COLORPICKER_H
#define BASEWIDGETS_COLORPICKER_H

#include <basewidgets/QtAdvancedUIExport.h>

class QWidget;
class QColor;

namespace basewidgets {

ACSS_UI_EXPORT QWidget* makeColorPickerBody(QWidget* parent, const QColor& initial, QColor* currentOut);

} // namespace basewidgets

#endif // BASEWIDGETS_COLORPICKER_H

