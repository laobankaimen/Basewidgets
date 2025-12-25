/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef QTADVANCEDUI_LINEEDIT_H
#define QTADVANCEDUI_LINEEDIT_H

#include <QLineEdit>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEdit(QWidget* parent = nullptr);
    explicit LineEdit(const QString& text, QWidget* parent = nullptr);
    explicit LineEdit(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~LineEdit() override;

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

#endif // QTADVANCEDUI_LINEEDIT_H

