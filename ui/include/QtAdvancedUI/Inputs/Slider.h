#ifndef QTADVANCEDUI_SLIDER_H
#define QTADVANCEDUI_SLIDER_H

#include <QSlider>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT Slider : public QSlider
{
    Q_OBJECT
public:
    explicit Slider(Qt::Orientation orientation = Qt::Horizontal, QWidget* parent = nullptr);
    explicit Slider(acss::QtAdvancedStylesheet* manager, Qt::Orientation orientation = Qt::Horizontal, QWidget* parent = nullptr);
    ~Slider() override;

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

#endif // QTADVANCEDUI_SLIDER_H
