/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_STYLEMANAGERADAPTER_H
#define BASEWIDGETS_STYLEMANAGERADAPTER_H

#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <QString>
#include <QDebug>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 全应用主题入口。所有函数在 GUI 线程调用，initialize 应在创建业务控件之前执行。 */
class ACSS_UI_EXPORT StyleManagerAdapter
{
public:
    /** 高 DPI 兼容配置；必须在 QApplication 构造前调用。Qt6 原生启用高 DPI。 */
    static void EnableHighDpi();
    /** 返回由 QApplication 持有的全局管理器；首次调用会自动初始化，不要 delete。 */
    static acss::QtAdvancedStylesheet* instance();

    /** 安装 NativeStyle 并应用主题色和图标。
     * @param stylesDir 空值使用库内嵌资源；也可传外部 styles 目录或 RCC 文件路径。
     * @param styleName 空值使用 themes；目录中对应同名样式子目录。
     * @param themeName 空值使用 JSON 默认主题；切换时用 setCurrentTheme()+updateTheme()。
     * 外部 RCC 为显式覆盖，应用目录中的旧资源不会自动覆盖内嵌主题。
     */
    static void initialize(const QString& stylesDir = QString(),
                           const QString& styleName = QString(),
                           const QString& themeName = QString());
    /** 旧资源构建入口，仅为兼容；工程构建已自动生成资源，普通应用无需调用。 */
    static void initialBuilder(const QString& stylesDir = QString());

    /** 在首次 instance/initialize 前注入管理器；无父对象时由 QApplication 接管，已有实例时忽略替换。 */
    static void setInstance(acss::QtAdvancedStylesheet* mgr);

private:
    static acss::QtAdvancedStylesheet* s_instance;
};

} // namespace basewidgets

#endif // BASEWIDGETS_STYLEMANAGERADAPTER_H

