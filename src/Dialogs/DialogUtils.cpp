/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */

#include <basewidgets/Dialogs/DialogUtils.h>
#include <basewidgets/Core/FramelessDialog.h>

#include "ColorPicker.h"
#include "FilePicker.h"

#include <QVariant>
#include <QFileInfo>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QScreen>
#include <basewidgets/Core/WindowPlacement.h>

namespace basewidgets {

static QColor s_lastPickedColor = QColor("#26A69A");

QColor DialogUtils::getLastPickedColor()
{
    return s_lastPickedColor;
}

static void prepareFileDialog(FramelessDialog& dialog,FilePickerWidget* picker,bool save=false)
{
    dialog.setSizeGripEnabled(true);
    QSize target(980,680);
    if(auto* screen=WindowPlacement::screenFor(&dialog))target=target.boundedTo(screen->availableGeometry().size()-QSize(24,24));
    dialog.resize(target);
    if(auto* buttons=dialog.findChild<QDialogButtonBox*>())
        if(auto* ok=buttons->button(QDialogButtonBox::Ok))ok->setText(save?QObject::tr("保存"):QObject::tr("选择"));
    auto accept=[&dialog,picker,save] {
        if(!picker->validateSelection())return;
        if(save && QFileInfo::exists(picker->selectedPath()) &&
           !DialogUtils::askQuestion(&dialog,QObject::tr("替换文件"),QObject::tr("文件已存在，是否替换？\n%1").arg(picker->selectedPath())))return;
        dialog.accept();
    };
    QObject::connect(&dialog,&FramelessDialog::okButtonClicked,&dialog,accept);
    QObject::connect(picker,&FilePickerWidget::fileActivated,&dialog,accept);
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
    prepareFileDialog(dlg,widget);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);

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
    prepareFileDialog(dlg,widget);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);

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
    prepareFileDialog(dlg,widget,true);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);

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
    prepareFileDialog(dlg,widget);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);

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
