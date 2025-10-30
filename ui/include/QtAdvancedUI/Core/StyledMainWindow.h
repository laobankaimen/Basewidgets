#ifndef QTADVANCEDUI_STYLEDMAINWINDOW_H
#define QTADVANCEDUI_STYLEDMAINWINDOW_H

#include <QMainWindow>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT StyledMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit StyledMainWindow(QWidget* parent = nullptr);
    explicit StyledMainWindow(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~StyledMainWindow() override;

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

#endif // QTADVANCEDUI_STYLEDMAINWINDOW_H