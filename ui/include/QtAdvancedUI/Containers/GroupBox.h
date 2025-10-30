#ifndef QTADVANCEDUI_GROUPBOX_H
#define QTADVANCEDUI_GROUPBOX_H

#include <QGroupBox>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT GroupBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit GroupBox(QWidget* parent = nullptr);
    explicit GroupBox(const QString& title, QWidget* parent = nullptr);
    explicit GroupBox(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~GroupBox() override;

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

#endif // QTADVANCEDUI_GROUPBOX_H