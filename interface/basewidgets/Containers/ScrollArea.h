/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_SCROLLAREA_H
#define BASEWIDGETS_SCROLLAREA_H

#include <QScrollArea>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 滚动容器；默认 widgetResizable=true，setWidget 后由滚动区域接管内容控件。 */
class ACSS_UI_EXPORT ScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit ScrollArea(QWidget* parent = nullptr);
    explicit ScrollArea(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ScrollArea() override;

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

#endif // BASEWIDGETS_SCROLLAREA_H

