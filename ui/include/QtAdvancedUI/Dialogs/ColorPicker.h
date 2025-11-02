#ifndef QTADVANCEDUI_COLORPICKER_H
#define QTADVANCEDUI_COLORPICKER_H

#include <QtAdvancedUI/QtAdvancedUIExport.h>

class QWidget;
class QColor;

namespace QtAdvancedUI {

ACSS_UI_EXPORT QWidget* makeColorPickerBody(QWidget* parent, const QColor& initial, QColor* currentOut);

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_COLORPICKER_H