/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */

#include <basewidgets/Dialogs/DialogUtils.h>
#include <basewidgets/Core/FramelessDialog.h>

#include "ColorPicker.h"
#include "FilePicker.h"

#include <QVariant>

namespace basewidgets {

static QColor s_lastPickedColor = QColor("#26A69A");

QColor DialogUtils::getLastPickedColor()
{
    return s_lastPickedColor;
}

QString DialogUtils::openFile(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("打开文件") : title);
    auto* widget = new FilePickerWidget(&dlg, initialDir, filter, FilePickerWidget::OpenFile);
    widget->setProperty("bwMsgContent", QVariant(true));
    widget->setProperty("msgContent", QVariant(true));
    dlg.setContentWidget(widget);
    dlg.setCustomDialogBtn(true,true,true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);
    QObject::connect(widget, &FilePickerWidget::fileActivated, &dlg, &QDialog::accept);

    if (dlg.exec() == QDialog::Accepted) {
        return widget->selectedPath();
    }
    return QString();
}

QStringList DialogUtils::openFiles(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择文件") : title);
    auto* widget = new FilePickerWidget(&dlg, initialDir, filter, FilePickerWidget::OpenFiles);
    widget->setProperty("bwMsgContent", QVariant(true));
    widget->setProperty("msgContent", QVariant(true));
    dlg.setContentWidget(widget);
    dlg.setCustomDialogBtn(true,true,true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);
    QObject::connect(widget, &FilePickerWidget::fileActivated, &dlg, &QDialog::accept);

    if (dlg.exec() == QDialog::Accepted) {
        return widget->selectedPaths();
    }
    return QStringList{};
}

QString DialogUtils::saveFile(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("保存文件") : title);
    auto* widget = new FilePickerWidget(&dlg, initialDir, filter, FilePickerWidget::SaveFile);
    widget->setProperty("bwMsgContent", QVariant(true));
    widget->setProperty("msgContent", QVariant(true));
    dlg.setContentWidget(widget);
    dlg.setCustomDialogBtn(true,true,true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);
    QObject::connect(widget, &FilePickerWidget::fileActivated, &dlg, &QDialog::accept);

    if (dlg.exec() == QDialog::Accepted) {
        return widget->selectedPath();
    }
    return QString();
}

QString DialogUtils::openDirectory(QWidget* parent, const QString& title, const QString& initialDir)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择文件夹") : title);
    auto* widget = new FilePickerWidget(&dlg, initialDir, QString(), FilePickerWidget::OpenDirectory);
    widget->setProperty("bwMsgContent", QVariant(true));
    widget->setProperty("msgContent", QVariant(true));
    dlg.setContentWidget(widget);
    dlg.setCustomDialogBtn(true,true,true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);
    QObject::connect(widget, &FilePickerWidget::fileActivated, &dlg, &QDialog::accept);

    if (dlg.exec() == QDialog::Accepted) {
        return widget->selectedPath();
    }
    return QString();
}

QColor DialogUtils::pickColor(QWidget* parent, const QColor& initial, const QString& title)
{
    QColor effectiveInitial = initial.isValid() ? initial : s_lastPickedColor;
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择颜色") : title);
    auto* widget = new ColorPickerWidget(&dlg, effectiveInitial);
    widget->setProperty("bwMsgContent", QVariant(true));
    widget->setProperty("msgContent", QVariant(true));
    dlg.setContentWidget(widget);
    dlg.setCustomDialogBtn(true,true,true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        QColor c = widget->currentColor();
        // Update global memory on confirmation
        s_lastPickedColor = c;
        return c;
    }
    return effectiveInitial;
}

} // namespace basewidgets
