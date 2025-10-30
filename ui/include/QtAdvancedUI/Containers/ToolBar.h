#ifndef QTADVANCEDUI_TOOLBAR_H
#define QTADVANCEDUI_TOOLBAR_H

#include <QToolBar>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT ToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit ToolBar(QWidget* parent = nullptr);
    explicit ToolBar(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ToolBar() override;

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

#endif // QTADVANCEDUI_TOOLBAR_H