#ifndef QTADVANCEDUI_TREEVIEW_H
#define QTADVANCEDUI_TREEVIEW_H

#include <QTreeView>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT TreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit TreeView(QWidget* parent = nullptr);
    explicit TreeView(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~TreeView() override;

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

#endif // QTADVANCEDUI_TREEVIEW_H