/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_MENUBAR_H
#define BASEWIDGETS_MENUBAR_H

#include <QMenuBar>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

class ACSS_UI_EXPORT MenuBar : public QMenuBar
{
    Q_OBJECT
public:
    explicit MenuBar(QWidget* parent = nullptr);
    explicit MenuBar(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~MenuBar() override;

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

#endif // BASEWIDGETS_MENUBAR_H

