#ifndef QTADVANCEDUI_LISTWIDGET_H
#define QTADVANCEDUI_LISTWIDGET_H

#include <QListWidget>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT ListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit ListWidget(QWidget* parent = nullptr);
    explicit ListWidget(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ListWidget() override;

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

#endif // QTADVANCEDUI_LISTWIDGET_H