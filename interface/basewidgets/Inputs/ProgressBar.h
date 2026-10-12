/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_PROGRESSBAR_H
#define BASEWIDGETS_PROGRESSBAR_H

#include <QProgressBar>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

class ACSS_UI_EXPORT ProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    explicit ProgressBar(QWidget* parent = nullptr);
    explicit ProgressBar(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ProgressBar() override;

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

#endif // BASEWIDGETS_PROGRESSBAR_H

