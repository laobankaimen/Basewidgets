#ifndef QTADVANCEDUI_RADIOBUTTON_H
#define QTADVANCEDUI_RADIOBUTTON_H

#include <QRadioButton>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT RadioButton : public QRadioButton
{
    Q_OBJECT
public:
    explicit RadioButton(QWidget* parent = nullptr);
    explicit RadioButton(const QString& text, QWidget* parent = nullptr);
    explicit RadioButton(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~RadioButton() override;

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

#endif // QTADVANCEDUI_RADIOBUTTON_H