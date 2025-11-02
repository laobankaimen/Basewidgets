#ifndef QTADVANCEDUI_DIALOGUTILS_H
#define QTADVANCEDUI_DIALOGUTILS_H

#include <QString>
#include <QStringList>
#include <QColor>
#include <QFont>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

class QWidget;

namespace QtAdvancedUI {

class ACSS_UI_EXPORT DialogUtils {
public:
    // File dialogs
    static QString openFile(QWidget* parent = nullptr,
                            const QString& title = QString(),
                            const QString& initialDir = QString(),
                            const QString& filter = QString());
    static QStringList openFiles(QWidget* parent = nullptr,
                                 const QString& title = QString(),
                                 const QString& initialDir = QString(),
                                 const QString& filter = QString());
    static QString saveFile(QWidget* parent = nullptr,
                            const QString& title = QString(),
                            const QString& initialDir = QString(),
                            const QString& filter = QString());
    static QString openDirectory(QWidget* parent = nullptr,
                                 const QString& title = QString(),
                                 const QString& initialDir = QString());

    // Color & font dialogs
    static QColor pickColor(QWidget* parent = nullptr,
                            const QColor& initial = QColor(),
                            const QString& title = QString());
    static QFont pickFont(QWidget* parent = nullptr,
                          const QFont& initial = QFont(),
                          bool* okOut = nullptr,
                          const QString& title = QString());

    // Message boxes
    static void messageInfo(QWidget* parent, const QString& title, const QString& text);
    static void messageWarning(QWidget* parent, const QString& title, const QString& text);
    static void messageError(QWidget* parent, const QString& title, const QString& text);
    static void messageSuccess(QWidget* parent, const QString& title, const QString& text);
    static bool askQuestion(QWidget* parent, const QString& title, const QString& text);

    // Input dialogs
    static QString inputText(QWidget* parent, const QString& title, const QString& label, const QString& text = QString());
    static int inputInt(QWidget* parent, const QString& title, const QString& label, int value = 0, int min = 0, int max = 100, int step = 1);
    static double inputDouble(QWidget* parent, const QString& title, const QString& label, double value = 0.0, double min = 0.0, double max = 100.0, int decimals = 2);
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_DIALOGUTILS_H