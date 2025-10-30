#ifndef QTADVANCEDUI_TABWIDGET_H
#define QTADVANCEDUI_TABWIDGET_H

#include <QTabWidget>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget* parent = nullptr);
    explicit TabWidget(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~TabWidget() override;

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

#endif // QTADVANCEDUI_TABWIDGET_H