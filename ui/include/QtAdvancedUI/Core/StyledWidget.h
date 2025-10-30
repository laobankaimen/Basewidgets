#ifndef QTADVANCEDUI_STYLEDWIDGET_H
#define QTADVANCEDUI_STYLEDWIDGET_H

#include <QWidget>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT StyledWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StyledWidget(QWidget* parent = nullptr);
    explicit StyledWidget(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~StyledWidget() override;

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

#endif // QTADVANCEDUI_STYLEDWIDGET_H