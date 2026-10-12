/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_SLIDER_H
#define BASEWIDGETS_SLIDER_H

#include <QSlider>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

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

} // namespace basewidgets

#endif // BASEWIDGETS_SLIDER_H

