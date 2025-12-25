#ifndef QTADVANCEDUI_STATUSBAR_H
#define QTADVANCEDUI_STATUSBAR_H

#include <QStatusBar>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT StatusBar : public QStatusBar
{
    Q_OBJECT
public:
    explicit StatusBar(QWidget* parent = nullptr);
    explicit StatusBar(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~StatusBar() override;

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

#endif // QTADVANCEDUI_STATUSBAR_H
