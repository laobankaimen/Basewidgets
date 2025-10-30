#ifndef QTADVANCEDUI_FRAME_H
#define QTADVANCEDUI_FRAME_H

#include <QFrame>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

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

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_FRAME_H