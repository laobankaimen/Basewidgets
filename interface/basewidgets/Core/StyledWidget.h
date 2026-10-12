/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_STYLEDWIDGET_H
#define BASEWIDGETS_STYLEDWIDGET_H

#include <QWidget>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

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

} // namespace basewidgets

#endif // BASEWIDGETS_STYLEDWIDGET_H

