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

/** 通用带边框容器；默认 StyledPanel，自绘面板边界由 NativeStyle 提供。 */
class ACSS_UI_EXPORT Frame : public QFrame
{
    Q_OBJECT
public:
    explicit Frame(QWidget* parent = nullptr);
    explicit Frame(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~Frame() override;

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

#endif // BASEWIDGETS_FRAME_H

