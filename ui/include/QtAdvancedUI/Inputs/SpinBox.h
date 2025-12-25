#ifndef QTADVANCEDUI_SPINBOX_H
#define QTADVANCEDUI_SPINBOX_H

#include <QSpinBox>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT SpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit SpinBox(QWidget* parent = nullptr);
    explicit SpinBox(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~SpinBox() override;

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

#endif // QTADVANCEDUI_SPINBOX_H
