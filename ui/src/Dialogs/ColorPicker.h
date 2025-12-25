#pragma once

#include <QWidget>
#include <QColor>

namespace QtAdvancedUI {

class ColorPicker
{
public:
    static QWidget* makeColorPickerBody(QWidget* parent, const QColor& initial, QColor* currentOut);
};

} // namespace QtAdvancedUI
