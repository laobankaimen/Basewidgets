/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef FONTPICKER_H
 #define FONTPICKER_H
 
 #include <QWidget>
 #include <QFont>
 
 namespace basewidgets {
 
 class FontPicker {
 public:
     static QWidget* makeFontPickerBody(QWidget* parent, QFont current, QFont* currentOut);
 };
 
 } // namespace basewidgets
 
 #endif // FONTPICKER_H

