#ifndef QTADVANCEDUI_FILEPICKER_H
#define QTADVANCEDUI_FILEPICKER_H

#include <QWidget>
#include <QHBoxLayout>
#include <QString>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QtAdvancedUI/Inputs/LineEdit.h>
#include <QtAdvancedUI/Inputs/ToolButton.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT FilePicker : public QWidget
{
    Q_OBJECT
public:
    enum Mode { OpenFile, SaveFile, OpenDirectory };

    explicit FilePicker(QWidget* parent = nullptr);
    explicit FilePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~FilePicker() override;

    void setStyleManager(acss::QtAdvancedStylesheet* manager);
    acss::QtAdvancedStylesheet* styleManager() const { return m_manager; }

    void setMode(Mode m);
    Mode mode() const { return m_mode; }

    void setTitle(const QString& title);
    void setFilter(const QString& filter);
    void setInitialDir(const QString& dir);
    void setPath(const QString& path);
    QString path() const;

signals:
    void pathChanged(const QString& path);

private:
    void attachToManager();
    void applyStylesheet();
    void browse();

private slots:
    void onStylesheetChanged();

private:
    acss::QtAdvancedStylesheet* m_manager = nullptr;
    Mode m_mode = OpenFile;
    QString m_title;
    QString m_filter;
    QString m_initialDir;
    LineEdit* m_edit = nullptr;
    ToolButton* m_button = nullptr;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_FILEPICKER_H