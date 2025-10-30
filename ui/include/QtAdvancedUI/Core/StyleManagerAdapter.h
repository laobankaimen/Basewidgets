#ifndef QTADVANCEDUI_STYLEMANAGERADAPTER_H
#define QTADVANCEDUI_STYLEMANAGERADAPTER_H

#include <QtAdvancedStylesheet.h>
#include <QString>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT StyleManagerAdapter
{
public:
    // 返回全局样式管理器实例（懒加载）
    static acss::QtAdvancedStylesheet* instance();

    // 初始化样式管理器（可传入样式目录、样式名、主题名）
    static void initialize(const QString& stylesDir = QString(),
                           const QString& styleName = QString(),
                           const QString& themeName = QString());

    // 设置/获取全局实例（用于主程序自定义管理器）
    static void setInstance(acss::QtAdvancedStylesheet* mgr);

private:
    static acss::QtAdvancedStylesheet* s_instance;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_STYLEMANAGERADAPTER_H