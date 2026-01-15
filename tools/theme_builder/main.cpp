#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <iostream>

using namespace basewidgets;
using namespace acss;

// Helper to copy directory recursively
bool copyDirRecursively(const QString &srcPath, const QString &dstPath) {
    QDir srcDir(srcPath);
    if (!srcDir.exists()) return false;
    
    QDir dstDir(dstPath);
    if (!dstDir.exists()) {
        dstDir.mkpath(".");
    }

    QDirIterator it(srcPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString srcFilePath = it.next();
        QString relativePath = srcDir.relativeFilePath(srcFilePath);
        QString dstFilePath = dstDir.filePath(relativePath);

        if (it.fileInfo().isDir()) {
            QDir().mkpath(dstFilePath);
        } else {
            if (QFile::exists(dstFilePath)) {
                QFile::remove(dstFilePath);
            }
            // Skip .rcc and .qrc files
            if (it.fileInfo().suffix() == "rcc" || it.fileInfo().suffix() == "qrc") {
                continue;
            }
            if (!QFile::copy(srcFilePath, dstFilePath)) {
                return false;
            }
        }
    }
    return true;
}

// Helper to delete directory recursively
bool removeDirRecursively(const QString &dirName) {
    QDir dir(dirName);
    if (!dir.exists()) return true;
    
    // Qt 5.0+ has removeRecursively(), but we can implement a fallback if needed.
    // Assuming Qt 5.0+ here.
    return dir.removeRecursively();
}

void generateQrcFile(const QString& outputDir, const QString& rccFileName, bool cleanUp = false) {
    QDir dir(outputDir);
    if (!dir.exists()) return;

    QString qrcContent = "<RCC>\n    <qresource prefix=\"/styles\">\n";
    
    QDirIterator it(outputDir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        QString relativePath = dir.relativeFilePath(filePath);
        
        // Skip .qrc and .rcc files to avoid recursive inclusion
        if (relativePath.endsWith(".qrc") || relativePath.endsWith(".rcc")) continue;
        
        qrcContent += QString("        <file>%1</file>\n").arg(relativePath);
    }
    
    qrcContent += "    </qresource>\n</RCC>";

    QString qrcPath = dir.filePath("themes.qrc");
    QFile qrcFile(qrcPath);
    bool rccGenerated = false;
    
    if (qrcFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&qrcFile);
        out << qrcContent;
        qrcFile.close();
        std::cout << "Generated QRC file: " << qrcPath.toStdString() << std::endl;
        
        // Try to run rcc to generate binary resource
        QString rccPath = "rcc"; // Assume rcc is in PATH
        QStringList arguments;
        arguments << "-binary" << "themes.qrc" << "-o" << rccFileName;
        
        std::cout << "Running rcc to generate " << rccFileName.toStdString() << "..." << std::endl;
        QProcess process;
        process.setWorkingDirectory(outputDir);
        process.start(rccPath, arguments);
        process.waitForFinished();
        
        if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
            std::cout << "Successfully generated binary resource: " << QDir(outputDir).absoluteFilePath(rccFileName).toStdString() << std::endl;
            rccGenerated = true;
        } else {
            std::cout << "Failed to generate binary resource. Ensure 'rcc' is in your PATH." << std::endl;
            std::cout << "Process error: " << process.readAllStandardError().toStdString() << std::endl;
        }
    } else {
        std::cerr << "Failed to write QRC file" << std::endl;
    }
    
    // Cleanup logic: remove all generated files except the .rcc file (if generated successfully)
    if (cleanUp && rccGenerated) {
        std::cout << "Cleaning up intermediate files..." << std::endl;
        
        // If rccFileName is relative (like "../themes.rcc"), it's already outside the outputDir, so deleting outputDir is safe.
        // If rccFileName is inside outputDir, we should move it out or be careful.
        // In our main(), we will set outputDir to a 'build' subdirectory and rccFileName to "../themes.rcc".
        
        // We assume outputDir contains ONLY the temporary files.
        // Just in case, we won't delete outputDir itself inside this function if it's the current working directory, 
        // but we can delete its content.
        
        // Actually, the main() calls this. Let's handle directory deletion in main() or via helper.
        // But here we can delete themes.qrc
        QFile::remove(qrcPath);
    }
}


int main(int argc, char *argv[])
{
    // 需要 QApplication 来处理一些与绘图相关的初始化（如 QFontDatabase, QIcon）
    // 使用 offscreen 平台插件避免弹出窗口（如果是在服务器上运行）
    // 但在本地工具中，直接使用默认平台通常也没问题
    QApplication app(argc, argv);

    std::cout << "Starting Theme Builder..." << std::endl;

    // 初始化样式管理器
    // 由于 styles 目录已移动到 tools/theme_builder/styles，我们需要在运行时定位它
    // 在开发环境中，它可能位于源代码目录中；在部署后，它可能位于可执行文件旁边
    QString appDir = QApplication::applicationDirPath();
    std::cout << "App Dir: " << appDir.toStdString() << std::endl;
    
    QString stylesDir = QDir(appDir).filePath("styles");
    if (!QDir(stylesDir).exists()) {
        // 尝试在源代码目录中查找（假设从 build 目录运行）
        // Common build depth: build/tools/theme_builder/Release -> 4 levels up to root
        QStringList candidates = {
            "../../tools/theme_builder/styles",
            "../../../tools/theme_builder/styles",
            "../../../../tools/theme_builder/styles",
            "../../../../../tools/theme_builder/styles"
        };
        
        for (const QString& relPath : candidates) {
            QString candidate = QDir(appDir).filePath(relPath);
            if (QDir(candidate).exists()) {
                stylesDir = candidate;
                break;
            }
        }
    }
    
    // Resolve to absolute path
    if (QDir(stylesDir).exists()) {
        stylesDir = QFileInfo(stylesDir).canonicalFilePath();
    }
    
    std::cout << "Using Styles Directory: " << stylesDir.toStdString() << std::endl;
    
    StyleManagerAdapter::initialBuilder(stylesDir);
    
    auto* mgr = StyleManagerAdapter::instance();
    if (!mgr) {
        std::cerr << "Failed to initialize StyleManagerAdapter" << std::endl;
        return 1;
    }

    // Set a custom output directory for the builder tool
    // This allows generating themes into a clean directory for manual copying
    QString buildOutputDir = QDir(QApplication::applicationDirPath()).filePath("generated_themes");
    
    // Use a temporary subdirectory for building
    QString tempBuildDir = QDir(buildOutputDir).filePath("temp_build");
    
    // Copy all source styles to temp_build
    std::cout << "Copying source styles to temp build directory..." << std::endl;
    if (!copyDirRecursively(stylesDir, tempBuildDir)) {
        std::cerr << "Failed to copy source styles to temp build directory" << std::endl;
    }
    
    mgr->setOutputDirPath(tempBuildDir);

    std::cout << "Styles Directory: " << mgr->stylesDirPath().toStdString() << std::endl;
    std::cout << "Output Directory: " << mgr->outputDirPath().toStdString() << std::endl;

    // 遍历所有可用样式
    const QStringList styles = mgr->styles();
    for (const QString& style : styles) {
        std::cout << "\nProcessing Style: " << style.toStdString() << std::endl;
        
        if (!mgr->setCurrentStyle(style)) {
            std::cerr << "  Error: Failed to set style " << style.toStdString() 
                      << " (" << mgr->errorString().toStdString() << ")" << std::endl;
            continue;
        }

        // 遍历当前样式的所用主题
        const QStringList themes = mgr->themes();
        for (const QString& theme : themes) {
            std::cout << "  - Generating Theme: " << theme.toStdString() << "... ";
            
            if (!mgr->setCurrentTheme(theme)) {
                std::cout << "FAILED (Set Theme Error: " << mgr->errorString().toStdString() << ")" << std::endl;
                continue;
            }

            // 强制更新（生成）样式表和资源
            // updateStylesheet(true) 会忽略已存在的文件，强制重新生成
            if (mgr->updateStylesheet(true)) {
                std::cout << "SUCCESS" << std::endl;
                std::cout << "    Output: " << mgr->currentStyleOutputPath().toStdString() << std::endl;
            } else {
                std::cout << "FAILED (Generate Error: " << mgr->errorString().toStdString() << ")" << std::endl;
            }
        }
    }

    std::cout << "\nTheme generation completed." << std::endl;
    
    // Generate QRC and binary resource
    // rcc will be generated in generated_themes/themes.rcc
    generateQrcFile(tempBuildDir, "../themes.rcc", true);
    
    // Clean up the temporary directory
    //std::cout << "Removing temporary files..." << std::endl;
    //removeDirRecursively(tempBuildDir);
    std::cout << "Done. The binary resource is at: " << QDir(buildOutputDir).filePath("themes.rcc").toStdString() << std::endl;

    return 0;
}
