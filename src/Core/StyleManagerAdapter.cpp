/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Core/NativeStyle.h>
#include <QApplication>
#include <QWidget>
#include <QDir>
#include <QResource>
#include <QMetaObject>
#include <QFileInfo>
#include <QStandardPaths>

using namespace acss;

namespace basewidgets {

acss::QtAdvancedStylesheet* StyleManagerAdapter::s_instance = nullptr;
static QMetaObject::Connection s_stylesheetConnection;

namespace {

QString preferredOutputDir()
{
    QString outputDir = QApplication::applicationDirPath();
    if (!QFileInfo(outputDir).isWritable()) {
        outputDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    }
    if (!outputDir.isEmpty()) {
        QDir().mkpath(outputDir);
    }
    return outputDir;
}

} // namespace

void StyleManagerAdapter::EnableHighDpi() {
    /***
     * 启用高分屏支持。
     * 低版本 Qt 使用
     * - QT_SCALE_FACTOR=1.5
     * - QT_AUTO_SCREEN_SCALE_FACTOR
     * 高版本自动缩放
     * ref: https://blog.csdn.net/startl/article/details/105862817
     */
#ifdef Q_OS_WIN
    (void)_putenv(const_cast<char*>("QT_ENABLE_HIGHDPI_SCALING=1"));
#else
    (void)putenv(const_cast<char*>("QT_ENABLE_HIGHDPI_SCALING=1"));
#endif
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    // 非整数缩放
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
}

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
        s_instance = new QtAdvancedStylesheet(qApp);
    } else if (!s_instance->parent()) {
        s_instance->setParent(qApp);
    }

    // Prefer explicit source themes, otherwise use resources embedded in BaseWidgets.
    // External RCC overrides are opt-in, so stale files beside the app cannot shadow rebuilt assets.
    QString resolvedStylesDir=stylesDir;
    if (resolvedStylesDir.isEmpty()) {
        resolvedStylesDir=":/styles";
    } else if (QFileInfo(resolvedStylesDir).isFile()) {
        // Mount external bundles at a separate root rather than replacing the built-in namespace.
        if (QResource::registerResource(resolvedStylesDir,"/basewidgets-override"))
            resolvedStylesDir=":/basewidgets-override/styles";
        else qWarning() << "Cannot register theme bundle:" << resolvedStylesDir;
    }
    s_instance->setStylesDirPath(resolvedStylesDir);
    const QString outputDir=preferredOutputDir();
    if (!outputDir.isEmpty()) s_instance->setOutputDirPath(outputDir);

    if (!styleName.isEmpty()) {
        s_instance->setCurrentStyle(styleName);
    } else {
        s_instance->setCurrentStyle("themes");
    }

    if (themeName.isEmpty() || !s_instance->setCurrentTheme(themeName)) {
        s_instance->setDefaultTheme();
    }

    // Install the shared painter style before widgets are polished. No runtime QSS.
    if (!qobject_cast<NativeStyle*>(qApp->style())) {
        qApp->setStyle(new NativeStyle());
    }
    qApp->setStyleSheet(QString());

    if (s_stylesheetConnection) QObject::disconnect(s_stylesheetConnection);
    s_stylesheetConnection = QObject::connect(s_instance, &QtAdvancedStylesheet::themeChanged,
        qApp, []() {
            auto* tokens = ThemeTokens::instance();
            tokens->reload();
            qApp->setPalette(tokens->palette());
            // Custom painter widgets may not consume QPalette roles directly.
            for (QWidget* widget : QApplication::allWidgets()) widget->update();
        });
    s_instance->updateTheme();
    QApplication::setWindowIcon(s_instance->styleIcon());
}

void StyleManagerAdapter::initialBuilder(const QString &stylesDir) {
    if (!s_instance) {
        s_instance = new QtAdvancedStylesheet(qApp);
    } else if (!s_instance->parent()) {
        s_instance->setParent(qApp);
    }

    QString resolvedStylesDir = stylesDir;
    // 如果未明确传入样式目录，则默认使用应用目录下的 styles 目录
    if (resolvedStylesDir.isEmpty()) {
        resolvedStylesDir = QDir(QApplication::applicationDirPath()).filePath("styles");
    }

    if (!resolvedStylesDir.isEmpty()) {
        s_instance->setStylesDirPath(resolvedStylesDir);
        const QString outputDir = preferredOutputDir();
        if (!outputDir.isEmpty()) {
            s_instance->setOutputDirPath(outputDir);
        }
    }
    s_instance->setCurrentStyle("themes");

    // Compatibility builder initialization: only refresh theme resources and icons.
    s_instance->updateTheme();
}

void StyleManagerAdapter::setInstance(acss::QtAdvancedStylesheet* mgr)
{
    if (s_instance) {
        if (s_instance == mgr) {
            return;
        }
        qWarning() << "StyleManagerAdapter::setInstance ignored: instance already created.";
        return;
    }

    if (!mgr) {
        s_instance = new QtAdvancedStylesheet(qApp);
        return;
    }

    s_instance = mgr;
    if (!s_instance->parent()) {
        s_instance->setParent(qApp);
    }
}

} // namespace basewidgets
