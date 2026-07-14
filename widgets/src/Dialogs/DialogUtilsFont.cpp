/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */

#include <basewidgets/Dialogs/DialogUtils.h>
#include <basewidgets/Core/FramelessDialog.h>

#include <QCheckBox>
#include <QEvent>
#include <QFontDialog>
#include <QGroupBox>
#include <QLabel>
#include <QVariant>

namespace basewidgets {

namespace {

void localizeFontDialogInternals(QFontDialog* fd)
{
    if (!fd) return;

    auto normalize = [](QString s) {
        s.replace('&', "");
        s.replace('_', "");
        return s.trimmed();
    };

    const auto groups = fd->findChildren<QGroupBox*>();
    for (auto* g : groups) {
        const QString t = normalize(g->title());
        const QString tn = t.toLower();
        if (tn.contains("font style")) g->setTitle(QObject::tr("字体样式"));
        else if (tn.contains("font")) g->setTitle(QObject::tr("字体"));
        else if (tn.contains("style")) g->setTitle(QObject::tr("样式"));
        else if (tn.contains("size") || tn.contains("point size")) g->setTitle(QObject::tr("大小"));
        else if (tn.contains("writing system")) g->setTitle(QObject::tr("书写系统"));
        else if (tn.contains("sample") || tn.contains("preview")) g->setTitle(QObject::tr("示例"));
        else if (tn.contains("effects")) g->setTitle(QObject::tr("效果"));
    }

    const auto labels = fd->findChildren<QLabel*>();
    for (auto* l : labels) {
        const QString t = normalize(l->text());
        const QString tn = t.toLower();
        if (tn.startsWith("font style")) l->setText(QObject::tr("字体样式"));
        else if (tn.startsWith("font")) l->setText(QObject::tr("字体"));
        else if (tn.startsWith("family")) l->setText(QObject::tr("字体"));
        else if (tn.startsWith("style")) l->setText(QObject::tr("样式"));
        else if (tn.startsWith("point size") || tn.startsWith("size")) l->setText(QObject::tr("大小"));
        else if (tn.startsWith("writing system")) l->setText(QObject::tr("书写系统"));
        else if (tn.startsWith("effects")) l->setText(QObject::tr("效果"));
        else if (tn.startsWith("sample") || tn.startsWith("preview")) l->setText(QObject::tr("示例"));
    }

    const auto checks = fd->findChildren<QCheckBox*>();
    for (auto* c : checks) {
        const QString t = normalize(c->text());
        if (t.contains("Bold", Qt::CaseInsensitive)) c->setText(QObject::tr("粗体"));
        else if (t.contains("Italic", Qt::CaseInsensitive)) c->setText(QObject::tr("斜体"));
        else if (t.contains("Underline", Qt::CaseInsensitive) || t.contains("Underlined", Qt::CaseInsensitive)) c->setText(QObject::tr("下划线"));
        else if (t.contains("Strikeout", Qt::CaseInsensitive) || t.contains("Strikethrough", Qt::CaseInsensitive)) c->setText(QObject::tr("删除线"));
        else if (t.contains("Kerning", Qt::CaseInsensitive)) c->setText(QObject::tr("字距"));
    }
}

class FontDialogLocalizer : public QObject {
public:
    explicit FontDialogLocalizer(QFontDialog* d) : QObject(d), fd(d) {}

protected:
    bool eventFilter(QObject* obj, QEvent* e) override
    {
        if (obj == fd) {
            switch (e->type()) {
            case QEvent::Show:
            case QEvent::Polish:
            case QEvent::LayoutRequest:
            case QEvent::LanguageChange:
                localizeFontDialogInternals(fd);
                break;
            default:
                break;
            }
        }
        return QObject::eventFilter(obj, e);
    }

private:
    QFontDialog* fd = nullptr;
};

} // namespace

QFont DialogUtils::pickFont(QWidget* parent, const QFont& initial, bool* okOut, const QString& title)
{
    FramelessDialog dlg(true, parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择字体") : title);

    auto* fd = new QFontDialog(initial, &dlg);
    fd->setProperty("bwMsgContent", QVariant(true));
    fd->setProperty("msgContent", QVariant(true));
    fd->setOption(QFontDialog::DontUseNativeDialog, true);
    fd->setOption(QFontDialog::NoButtons, true);
    fd->setWindowFlags(Qt::Widget);
    fd->installEventFilter(new FontDialogLocalizer(fd));

    dlg.setContentWidget(fd);
    dlg.setCustomDialogBtn(true, true, true);
    QObject::connect(&dlg, &FramelessDialog::okButtonClicked, &dlg, &QDialog::accept);
    QObject::connect(&dlg, &FramelessDialog::cancelButtonClicked, &dlg, &QDialog::reject);

    const bool ok = (dlg.exec() == QDialog::Accepted);
    if (okOut) *okOut = ok;
    return ok ? fd->selectedFont() : QFont();
}

} // namespace basewidgets
