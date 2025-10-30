#ifndef QTADVANCEDUI_FONTPICKER_H
#define QTADVANCEDUI_FONTPICKER_H

#include <QWidget>
#include <QHBoxLayout>
#include <QFont>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Inputs/PushButton.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT FontPicker : public QWidget
{
    Q_OBJECT
public:
    explicit FontPicker(QWidget* parent = nullptr);
    explicit FontPicker(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~FontPicker() override;

    void setStyleManager(acss::QtAdvancedStylesheet* manager);
    acss::QtAdvancedStylesheet* styleManager() const { return m_manager; }

    void setFontValue(const QFont& f);
    QFont fontValue() const { return m_font; }

signals:
    void fontChanged(const QFont& f);

private:
    void attachToManager();
    void applyStylesheet();
    void chooseFont();
    void updatePreview();

private slots:
    void onStylesheetChanged();

private:
    acss::QtAdvancedStylesheet* m_manager = nullptr;
    QFont m_font;
    QLabel* m_preview = nullptr;
    PushButton* m_button = nullptr;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_FONTPICKER_H