/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef QTADVANCEDUI_MENUBAR_H
#define QTADVANCEDUI_MENUBAR_H

#include <QMenuBar>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

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

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_MENUBAR_H

