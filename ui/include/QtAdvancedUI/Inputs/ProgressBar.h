#ifndef QTADVANCEDUI_PROGRESSBAR_H
#define QTADVANCEDUI_PROGRESSBAR_H

#include <QProgressBar>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

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

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_PROGRESSBAR_H