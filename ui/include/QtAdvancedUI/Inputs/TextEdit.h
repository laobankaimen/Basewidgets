#ifndef QTADVANCEDUI_TEXTEDIT_H
#define QTADVANCEDUI_TEXTEDIT_H

#include <QTextEdit>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT TextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit TextEdit(QWidget* parent = nullptr);
    explicit TextEdit(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~TextEdit() override;

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

#endif // QTADVANCEDUI_TEXTEDIT_H
