#ifndef QTADVANCEDUI_COLORPICKER_H
#define QTADVANCEDUI_COLORPICKER_H

#include <QWidget>
#include <QHBoxLayout>
#include <QColor>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QtAdvancedUI/Inputs/PushButton.h>
#include <QtAdvancedUI/Inputs/Label.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT ColorPicker : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPicker(QWidget* parent = nullptr);
    explicit ColorPicker(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ColorPicker() override;

    void setStyleManager(acss::QtAdvancedStylesheet* manager);
    acss::QtAdvancedStylesheet* styleManager() const { return m_manager; }

    void setColor(const QColor& c);
    QColor color() const { return m_color; }

signals:
    void colorChanged(const QColor& c);

private:
    void attachToManager();
    void applyStylesheet();
    void chooseColor();
    void updatePreview();

private slots:
    void onStylesheetChanged();

private:
    acss::QtAdvancedStylesheet* m_manager = nullptr;
    QColor m_color;
    PushButton* m_button = nullptr;
    QLabel* m_preview = nullptr;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_COLORPICKER_H