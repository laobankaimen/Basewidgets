/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef QTADVANCEDUI_TOOLBUTTON_H
#define QTADVANCEDUI_TOOLBUTTON_H

#include <QToolButton>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT ToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ToolButton(QWidget* parent = nullptr);
    explicit ToolButton(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ToolButton() override;

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

#endif // QTADVANCEDUI_TOOLBUTTON_H

