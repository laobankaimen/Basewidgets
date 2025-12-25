/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef QTADVANCEDUI_TOOLBOX_H
#define QTADVANCEDUI_TOOLBOX_H

#include <QToolBox>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT ToolBox : public QToolBox
{
    Q_OBJECT
public:
    explicit ToolBox(QWidget* parent = nullptr);
    explicit ToolBox(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ToolBox() override;

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

#endif // QTADVANCEDUI_TOOLBOX_H

