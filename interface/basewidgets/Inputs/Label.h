/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_LABEL_H
#define BASEWIDGETS_LABEL_H

#include <QLabel>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

class ACSS_UI_EXPORT Label : public QLabel
{
    Q_OBJECT
public:
    explicit Label(QWidget* parent = nullptr);
    explicit Label(const QString& text, QWidget* parent = nullptr);
    explicit Label(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~Label() override;

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

#endif // BASEWIDGETS_LABEL_H

