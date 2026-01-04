/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_FRAME_H
#define BASEWIDGETS_FRAME_H

#include <QFrame>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

class ACSS_UI_EXPORT Frame : public QFrame
{
    Q_OBJECT
public:
    explicit Frame(QWidget* parent = nullptr);
    explicit Frame(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~Frame() override;

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

#endif // BASEWIDGETS_FRAME_H

