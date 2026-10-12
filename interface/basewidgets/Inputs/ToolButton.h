/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_TOOLBUTTON_H
#define BASEWIDGETS_TOOLBUTTON_H

#include <QToolButton>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

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

} // namespace basewidgets

#endif // BASEWIDGETS_TOOLBUTTON_H

