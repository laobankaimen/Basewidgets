#ifndef QTADVANCEDUI_STYLEDDIALOG_H
#define QTADVANCEDUI_STYLEDDIALOG_H

#include <QDialog>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT StyledDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StyledDialog(QWidget* parent = nullptr);
    explicit StyledDialog(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~StyledDialog() override;

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

#endif // QTADVANCEDUI_STYLEDDIALOG_H