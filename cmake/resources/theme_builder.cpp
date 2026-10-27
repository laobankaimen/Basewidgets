#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QLibraryInfo>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QXmlStreamWriter>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>

namespace {
bool isStyleSheet(const QString& path)
{
    return path.endsWith(".css", Qt::CaseInsensitive) || path.endsWith(".qss", Qt::CaseInsensitive)
        || path.endsWith(".css.template", Qt::CaseInsensitive) || path.endsWith(".qss.template", Qt::CaseInsensitive);
}
bool copyThemeSources(const QString& source, const QString& destination)
{
    QDir root(source);
    QDirIterator files(source, QDir::Files, QDirIterator::Subdirectories);
    while (files.hasNext()) {
        const QString path = files.next();
        if (isStyleSheet(path) || path.endsWith(".rcc") || path.endsWith(".qrc")) continue;
        const QString target = QDir(destination).filePath(root.relativeFilePath(path));
        if (!QDir().mkpath(QFileInfo(target).absolutePath()) || !QFile::copy(path, target)) {
            qCritical() << "Cannot copy theme resource:" << path;
            return false;
        }
    }
    return true;
}
}

int main(int argc, char** argv)
{
    // Resource generation needs font/icon services but never opens a window.
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("Build theme colors, fonts and SVG resources (no QSS).");
    parser.addHelpOption();
    parser.addOption({"styles-dir", "Source styles directory", "directory"});
    parser.addOption({"cpp-output", "Generated C++ resource source for the UI library", "file"});
    parser.addOption({"rcc", "Qt host rcc executable selected by CMake", "file"});
    parser.addOption({"output", "Destination RCC file", "file", QDir(app.applicationDirPath()).filePath("generated_themes/themes.rcc")});
    parser.process(app);

    QString source = parser.value("styles-dir");
    if (source.isEmpty() || !QDir(source).exists()) {
        qCritical() << "Theme source directory not found; supply --styles-dir.";
        return 1;
    }
    source = QFileInfo(source).absoluteFilePath();
    const QString output = QFileInfo(parser.value("output")).absoluteFilePath();
    if (!QDir().mkpath(QFileInfo(output).absolutePath())) return 1;
    // A fresh staging directory prevents stale CSS from entering the resource bundle.
    QTemporaryDir staging;
    if (!staging.isValid() || !copyThemeSources(source, staging.path())) return 1;

    acss::QtAdvancedStylesheet manager;
    manager.setStylesDirPath(staging.path());
    manager.setOutputDirPath(staging.path());
    if (manager.styles().isEmpty()) { qCritical() << "No styles in" << source; return 1; }
    for (const QString& style : manager.styles()) {
        if (!manager.setCurrentStyle(style) || manager.themes().isEmpty()) {
            qCritical() << manager.currentStylePath() << manager.errorString(); return 1;
        }
        for (const QString& theme : manager.themes()) {
            if (!manager.setCurrentTheme(theme) || !manager.updateTheme()) {
                qCritical() << style << theme << manager.errorString(); return 1;
            }
        }
    }
    QStringList paths;
    QDirIterator files(staging.path(), QDir::Files, QDirIterator::Subdirectories);
    while (files.hasNext()) paths << QDir(staging.path()).relativeFilePath(files.next());
    paths.sort();
    QFile qrc(QDir(staging.path()).filePath("themes.qrc"));
    if (!qrc.open(QIODevice::WriteOnly)) return 1;
    QXmlStreamWriter xml(&qrc);
    xml.setAutoFormatting(true);
    xml.writeStartDocument(); xml.writeStartElement("RCC"); xml.writeStartElement("qresource");
    xml.writeAttribute("prefix", "/styles");
    for (const QString& path : paths) xml.writeTextElement("file", path);
    xml.writeEndElement(); xml.writeEndElement(); xml.writeEndDocument(); qrc.close();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString rcc = QDir(QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath)).filePath("rcc");
#else
    QString rcc = QDir(QLibraryInfo::location(QLibraryInfo::BinariesPath)).filePath("rcc");
#endif
#ifdef Q_OS_WIN
    rcc += ".exe";
#endif
    if (!QFileInfo::exists(rcc)) rcc = QStandardPaths::findExecutable("rcc");
    if (rcc.isEmpty()) { qCritical() << "Qt rcc executable not found."; return 1; }
    if (!parser.value("rcc").isEmpty()) rcc=parser.value("rcc");
    QProcess process;
    process.setWorkingDirectory(staging.path());
    const QString binary = QDir(staging.path()).filePath("themes.rcc");
    process.start(rcc, {"--binary", "themes.qrc", "-o", binary});
    if (!process.waitForFinished(30000) || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        qCritical() << "rcc failed:" << process.errorString() << process.readAllStandardError();
        return 1;
    }
    auto publish = [](const QString& input,const QString& output) {
        QFile compiled(input);
        QSaveFile destination(output);
        if (!QDir().mkpath(QFileInfo(output).absolutePath()) || !compiled.open(QIODevice::ReadOnly)
            || !destination.open(QIODevice::WriteOnly)) return false;
        const QByteArray bytes=compiled.readAll();
        return destination.write(bytes)==bytes.size() && destination.commit();
    };
    // Generate a self-contained Qt resource translation unit before the staging directory disappears.
    // Its static initializer registers :/styles automatically when BaseWidgets is loaded.
    const QString cppOutput=parser.value("cpp-output");
    if (!cppOutput.isEmpty()) {
        const QString cpp=QDir(staging.path()).filePath("themes_resource.cpp");
        process.start(rcc,{"--name","basewidgets_themes","themes.qrc","-o",cpp});
        if (!process.waitForFinished(30000) || process.exitStatus()!=QProcess::NormalExit || process.exitCode()!=0
            || !publish(cpp,cppOutput)) {
            qCritical() << "C++ resource generation failed:" << process.readAllStandardError(); return 1;
        }
    }
    if (!publish(binary,output)) return 1;
    qInfo().noquote() << "Theme resource written:" << output;
    return 0;
}
