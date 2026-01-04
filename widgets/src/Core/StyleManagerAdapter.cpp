/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <QApplication>
#include <QDir>
#include <QResource>

using namespace acss;

namespace basewidgets {

acss::QtAdvancedStylesheet* StyleManagerAdapter::s_instance = nullptr;

acss::QtAdvancedStylesheet* StyleManagerAdapter::instance()
{
    if (!s_instance) {
        initialize();
    }
    return s_instance;
}

void StyleManagerAdapter::initialize(const QString& stylesDir,
                                     const QString& styleName,
                                     const QString& themeName)
{
    if (!s_instance) {
        s_instance = new QtAdvancedStylesheet();
    }

    QString resolvedStylesDir = stylesDir;
    // 如果未明确传入样式目录，则默认使用应用目录下的 styles 目录
    if (resolvedStylesDir.isEmpty()) {
        resolvedStylesDir = QDir(QApplication::applicationDirPath()).filePath("styles");
    }

    if (!resolvedStylesDir.isEmpty()) {
        s_instance->setStylesDirPath(resolvedStylesDir);
        // Default output to application directory as requested
        // Generated files will be in <AppDir>/<StyleName>/<ThemeName>
        s_instance->setOutputDirPath(QApplication::applicationDirPath());

        // Try to load themes.rcc from styles directory or application directory
        QString rccName = "themes.rcc";
        QString rccPath = QDir(resolvedStylesDir).filePath(rccName);
        if (!QFile::exists(rccPath)) {
            rccPath = QDir(QApplication::applicationDirPath()).filePath(rccName);
        }
        
        bool rccLoaded = false;
        if (QFile::exists(rccPath)) {
            if (QResource::registerResource(rccPath)) {
                rccLoaded = true;
            }
        }
        
        if (rccLoaded) {
            // If binary resource is loaded, use it as the source for styles and output
            // This allows running entirely from the packed resource without generating files
            s_instance->setStylesDirPath(":/styles");
            s_instance->setOutputDirPath(":/styles");
        }
    }

    // 允许通过环境变量或外部配置覆盖输出目录（可选）
    // const QString outDir = QDir(QApplication::applicationDirPath()).filePath("output");
    // s_instance->setOutputDirPath(outDir);

    if (!styleName.isEmpty()) {
        s_instance->setCurrentStyle(styleName);
    } else {
        s_instance->setCurrentStyle("themes");
    }

    if (!themeName.isEmpty()) {
        s_instance->setCurrentTheme(themeName);
    }
    // 始终确保设置默认主题（当未指定或样式要求默认初始化时）
    s_instance->setDefaultTheme();

    // 仅更新应用调色板和图标，不强制重新生成（由 updateStylesheet 内部判断模板是否需要处理）
    // 假设样式表已由外部工具生成并作为静态资源或文件提供，此处主要负责加载
    s_instance->updateStylesheet();
    QApplication::setWindowIcon(s_instance->styleIcon());

    #ifdef Q_OS_LINUX
    //qApp->setStyle(QStyleFactory::create("Fusion"));
    //在linux强制保持统一风格，防止被GTK样式覆盖
    QApplication::setStyle("Fusion");
    #endif
    qApp->setStyleSheet(s_instance->styleSheet());

    // 监听样式变化，自动应用到整个应用
    QObject::connect(s_instance, &QtAdvancedStylesheet::stylesheetChanged, []() {
        auto* mgr = StyleManagerAdapter::instance();
        QApplication::setWindowIcon(mgr->styleIcon());
        qApp->setStyleSheet(mgr->styleSheet());
    });
}

void StyleManagerAdapter::setInstance(acss::QtAdvancedStylesheet* mgr)
{
    s_instance = mgr;
}

} // namespace basewidgets

