#ifndef QTADVANCEDUI_COMBOBOX_H
#define QTADVANCEDUI_COMBOBOX_H

#include <QComboBox>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT ComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit ComboBox(QWidget* parent = nullptr);
    explicit ComboBox(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ComboBox() override;

    void setStyleManager(acss::QtAdvancedStylesheet* manager);
    acss::QtAdvancedStylesheet* styleManager() const { return m_manager; }

protected:
    void applyStylesheet();

private:
    void attachToManager();

private slots:
    void onStylesheetChanged();

private:
    acss::QtAdvancedStylesheet* m_manager = nullptr;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_COMBOBOX_H
