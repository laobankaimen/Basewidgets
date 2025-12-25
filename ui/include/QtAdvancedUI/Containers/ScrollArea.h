#ifndef QTADVANCEDUI_SCROLLAREA_H
#define QTADVANCEDUI_SCROLLAREA_H

#include <QScrollArea>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT ScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit ScrollArea(QWidget* parent = nullptr);
    explicit ScrollArea(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ScrollArea() override;

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

#endif // QTADVANCEDUI_SCROLLAREA_H
