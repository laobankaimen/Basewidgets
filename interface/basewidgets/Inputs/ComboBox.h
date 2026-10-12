/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_COMBOBOX_H
#define BASEWIDGETS_COMBOBOX_H

#include <QComboBox>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

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

} // namespace basewidgets

#endif // BASEWIDGETS_COMBOBOX_H

