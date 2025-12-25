#ifndef FONTPICKER_H
 #define FONTPICKER_H
 
 #include <QWidget>
 #include <QFont>
 
 namespace QtAdvancedUI {
 
 class FontPicker {
 public:
     static QWidget* makeFontPickerBody(QWidget* parent, QFont current, QFont* currentOut);
 };
 
 } // namespace QtAdvancedUI
 
 #endif // FONTPICKER_H
