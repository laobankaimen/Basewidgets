#ifndef QTADVANCEDUI_TABLEWIDGET_H
#define QTADVANCEDUI_TABLEWIDGET_H

#include <QTableWidget>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT TableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit TableWidget(QWidget* parent = nullptr);
    explicit TableWidget(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~TableWidget() override;

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

#endif // QTADVANCEDUI_TABLEWIDGET_H