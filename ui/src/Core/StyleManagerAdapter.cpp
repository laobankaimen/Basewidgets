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
    } else {
        s_instance->setCurrentTheme("dark_teal");
    }

    s_instance->updateStylesheet();
}

void StyleManagerAdapter::setInstance(acss::QtAdvancedStylesheet* mgr)
{
    s_instance = mgr;
}

} // namespace QtAdvancedUI