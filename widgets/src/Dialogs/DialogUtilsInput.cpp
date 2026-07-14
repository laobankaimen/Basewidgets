/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */

#include <basewidgets/Dialogs/DialogUtils.h>
#include <basewidgets/Core/FramelessDialog.h>

#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

namespace basewidgets {

namespace {

QWidget* createInputBody(QWidget* parent)
{
    auto* body = new QWidget(parent);
    body->setProperty("bwMsgContent", QVariant(true));
    body->setProperty("msgContent", QVariant(true));
    return body;
}

} // namespace

QString DialogUtils::inputText(QWidget* parent, const QString& title, const QString& label, const QString& text)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title);

    auto* body = createInputBody(&dlg);
    auto* layout = new QVBoxLayout(body);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* lbl = new QLabel(label, body);
    auto* edit = new QLineEdit(text, body);
    layout->addWidget(lbl);
    layout->addWidget(edit);

    dlg.setContentWidget(body);
    dlg.setCustomDialogBtn(true, true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);
    if (dlg.exec() == QDialog::Accepted) return edit->text();
    return QString();
}

int DialogUtils::inputInt(QWidget* parent, const QString& title, const QString& label, int value, int min, int max, int step)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title);

    auto* body = createInputBody(&dlg);
    auto* layout = new QVBoxLayout(body);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* lbl = new QLabel(label, body);
    auto* spin = new QSpinBox(body);
    spin->setRange(min, max);
    spin->setSingleStep(step);
    spin->setValue(value);
    layout->addWidget(lbl);
    layout->addWidget(spin);

    dlg.setContentWidget(body);
    dlg.setCustomDialogBtn(true, true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);
    if (dlg.exec() == QDialog::Accepted) return spin->value();
    return value;
}

double DialogUtils::inputDouble(QWidget* parent, const QString& title, const QString& label, double value, double min, double max, int decimals)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title);

    auto* body = createInputBody(&dlg);
    auto* layout = new QVBoxLayout(body);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* lbl = new QLabel(label, body);
    auto* spin = new QDoubleSpinBox(body);
    spin->setRange(min, max);
    spin->setDecimals(decimals);
    spin->setValue(value);
    layout->addWidget(lbl);
    layout->addWidget(spin);

    dlg.setContentWidget(body);
    dlg.setCustomDialogBtn(true, true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);
    if (dlg.exec() == QDialog::Accepted) return spin->value();
    return value;
}

} // namespace basewidgets
