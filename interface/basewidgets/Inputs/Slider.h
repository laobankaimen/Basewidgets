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

/** 滑块包装；保留 QSlider 数值/拖动/键盘语义，自绘几何与命中区域由 NativeStyle 统一。 */
class ACSS_UI_EXPORT Slider : public QSlider
{
    Q_OBJECT
public:
    explicit Slider(Qt::Orientation orientation = Qt::Horizontal, QWidget* parent = nullptr);
    explicit Slider(acss::QtAdvancedStylesheet* manager, Qt::Orientation orientation = Qt::Horizontal, QWidget* parent = nullptr);
    ~Slider() override;

    /** 兼容管理器入口；不转移所有权。全应用主题请通过 StyleManagerAdapter 设置。 */
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

