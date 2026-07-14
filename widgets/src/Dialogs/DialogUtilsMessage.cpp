/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */

#include <basewidgets/Dialogs/DialogUtils.h>
#include <basewidgets/Core/FramelessDialog.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

#include <QColor>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QVariant>
#include <QWidget>

namespace basewidgets {

namespace {

QPixmap makeBadgeIcon(const QString& shape)
{
    constexpr int size = 32;
    if (shape == "check") {
        return StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/circle_check.svg").pixmap(size, size);
    }
    if (shape == "cross") {
        return StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/circle_xmark.svg").pixmap(size, size);
    }
    if (shape == "info") {
        return StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/circle_info.svg").pixmap(size, size);
    }
    if (shape == "question") {
        return StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/circle_question.svg").pixmap(size, size);
    }
    if (shape == "warning") {
        return StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/circle_exclamation.svg").pixmap(size, size);
    }
    return QPixmap();
}

QWidget* createMessageBody(QWidget* parent, const QString& text, const QString& shape)
{
    auto* body = new QWidget(parent);
    body->setProperty("bwMsgContent", QVariant(true));
    body->setProperty("msgContent", QVariant(true));

    auto* layout = new QHBoxLayout(body);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    auto* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(shape));

    auto* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setObjectName("DialogTextLabel");

    layout->addWidget(iconLabel);
    layout->addWidget(textLabel, 1);
    body->setLayout(layout);
    return body;
}

} // namespace

void DialogUtils::messageInfo(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title);
    dlg.setContentWidget(createMessageBody(&dlg, text, "info"));
    dlg.setCustomDialogBtn(true, false);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    dlg.exec();
}

void DialogUtils::messageWarning(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title);
    dlg.setContentWidget(createMessageBody(&dlg, text, "warning"));
    dlg.setCustomDialogBtn(true, false);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    dlg.exec();
}

void DialogUtils::messageError(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title);
    dlg.setContentWidget(createMessageBody(&dlg, text, "cross"));
    dlg.setCustomDialogBtn(true, false);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    dlg.exec();
}

void DialogUtils::messageSuccess(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title);
    dlg.setContentWidget(createMessageBody(&dlg, text, "check"));
    dlg.setCustomDialogBtn(true, false);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    dlg.exec();
}

bool DialogUtils::askQuestion(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title);
    dlg.setContentWidget(createMessageBody(&dlg, text, "question"));
    dlg.setCustomDialogBtn(true, true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);
    return dlg.exec() == QDialog::Accepted;
}

} // namespace basewidgets
