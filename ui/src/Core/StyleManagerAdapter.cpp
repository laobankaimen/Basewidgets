#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QApplication>
#include <QDir>

using namespace acss;

namespace QtAdvancedUI {

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
#ifdef UI_STYLES_DIR
    if (resolvedStylesDir.isEmpty()) {
        resolvedStylesDir = QString::fromUtf8(UI_STYLES_DIR);
    }
#endif

    // 如果未明确传入样式目录，则默认使用应用目录下的 styles 目录
    if (resolvedStylesDir.isEmpty()) {
        resolvedStylesDir = QDir(QApplication::applicationDirPath()).filePath("styles");
    }

    if (!resolvedStylesDir.isEmpty()) {
        s_instance->setStylesDirPath(resolvedStylesDir);
    }

    // 输出目录使用应用目录下 output（与 examples 一致）
    const QString outDir = QDir(QApplication::applicationDirPath()).filePath("output");
    s_instance->setOutputDirPath(outDir);

    if (!styleName.isEmpty()) {
        s_instance->setCurrentStyle(styleName);
    } else {
        s_instance->setCurrentStyle("qt_material");
    }

    if (!themeName.isEmpty()) {
        s_instance->setCurrentTheme(themeName);
    }
    // 始终确保设置默认主题（当未指定或样式要求默认初始化时）
    s_instance->setDefaultTheme();

    // 初次生成并应用到整个应用
    s_instance->updateStylesheet();
    QApplication::setWindowIcon(s_instance->styleIcon());
    qApp->setStyleSheet(s_instance->styleSheet());

    // 监听样式变化，自动应用到整个应用
    QObject::connect(s_instance, &QtAdvancedStylesheet::stylesheetChanged, []() {
        auto* mgr = StyleManagerAdapter::instance();
        QApplication::setWindowIcon(mgr->styleIcon());
        qApp->setStyleSheet(mgr->styleSheet());
    });
    Q_INIT_RESOURCE(images);//库中需要显示初始化资源才能在主程序使用
}

void StyleManagerAdapter::setInstance(acss::QtAdvancedStylesheet* mgr)
{
    s_instance = mgr;
}

} // namespace QtAdvancedUI