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

QLabel* createInputLabel(const QString& text, QWidget* input, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    label->setObjectName("DialogInputLabel");
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    label->setBuddy(input);
    input->setAccessibleName(text);
    return label;
}

} // namespace

QString DialogUtils::inputText(QWidget* parent, const QString& title, const QString& label, const QString& text)
{
    return inputText(parent, title, label, text, nullptr);
}

QString DialogUtils::inputText(QWidget* parent, const QString& title, const QString& label, const QString& text, bool* accepted)
{
    if (accepted) *accepted = false;
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title);

    auto* body = createInputBody(&dlg);
    auto* layout = new QVBoxLayout(body);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* edit = new QLineEdit(text, body);
    auto* lbl = createInputLabel(label, edit, body);
    layout->addWidget(lbl);
    layout->addWidget(edit);

    dlg.setContentWidget(body);
    dlg.setCustomDialogBtn(true, true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);
    const bool submitted = dlg.exec() == QDialog::Accepted;
    if (accepted) *accepted = submitted;
    return submitted ? edit->text() : QString();
}

int DialogUtils::inputInt(QWidget* parent, const QString& title, const QString& label, int value, int min, int max, int step)
{
    return inputInt(parent, title, label, value, min, max, step, nullptr);
}

int DialogUtils::inputInt(QWidget* parent, const QString& title, const QString& label, int value, int min, int max, int step, bool* accepted)
{
    if (accepted) *accepted = false;
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title);

    auto* body = createInputBody(&dlg);
    auto* layout = new QVBoxLayout(body);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* spin = new QSpinBox(body);
    auto* lbl = createInputLabel(label, spin, body);
    spin->setRange(min, max);
    spin->setSingleStep(step);
    spin->setValue(value);
    layout->addWidget(lbl);
    layout->addWidget(spin);

    dlg.setContentWidget(body);
    dlg.setCustomDialogBtn(true, true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);
    const bool submitted = dlg.exec() == QDialog::Accepted;
    if (accepted) *accepted = submitted;
    return submitted ? spin->value() : value;
}

double DialogUtils::inputDouble(QWidget* parent, const QString& title, const QString& label, double value, double min, double max, int decimals)
{
    return inputDouble(parent, title, label, value, min, max, decimals, nullptr);
}

double DialogUtils::inputDouble(QWidget* parent, const QString& title, const QString& label, double value, double min, double max, int decimals, bool* accepted)
{
    if (accepted) *accepted = false;
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title);

    auto* body = createInputBody(&dlg);
    auto* layout = new QVBoxLayout(body);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* spin = new QDoubleSpinBox(body);
    auto* lbl = createInputLabel(label, spin, body);
    spin->setRange(min, max);
    spin->setDecimals(decimals);
    spin->setValue(value);
    layout->addWidget(lbl);
    layout->addWidget(spin);

    dlg.setContentWidget(body);
    dlg.setCustomDialogBtn(true, true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);
    const bool submitted = dlg.exec() == QDialog::Accepted;
    if (accepted) *accepted = submitted;
    return submitted ? spin->value() : value;
}

} // namespace basewidgets
