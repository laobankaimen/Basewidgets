#include <QtAdvancedUI/Dialogs/DialogUtils.h>
#include <QtAdvancedUI/Core/FramelessDialog.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QIcon>
#include <QApplication>
#include <QAbstractButton>
#include <QGroupBox>
#include <QEvent>

using namespace acss;

namespace QtAdvancedUI {

static QWidget* makeContent(QWidget* parent, QWidget* body, QDialogButtonBox* box)
{
    QWidget* content = new QWidget(parent);
    auto* v = new QVBoxLayout(content);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(10);
    v->addWidget(body);
    if (box) v->addWidget(box);
    content->setLayout(v);
    return content;
}

// 额外本地化 QColorDialog 内部英文标签与功能按钮
static void localizeColorDialogInternals(QColorDialog* cd)
{
    if (!cd) return;

    auto normalize = [](QString s) {
        s.replace('&', "");
        s.replace('_', "");
        return s.trimmed();
    };

    // 按钮（非底部 OK/Cancel）：PICK SCREEN COLOR / ADD TO CUSTOM COLORS
    const auto btns = cd->findChildren<QAbstractButton*>();
    for (auto* b : btns) {
        const QString t = normalize(b->text());
        if (t.contains("PICK SCREEN COLOR", Qt::CaseInsensitive) || t.contains("Pick Screen Color", Qt::CaseInsensitive)) {
            b->setText(QObject::tr("取屏幕颜色"));
        } else if (t.contains("ADD TO CUSTOM COLORS", Qt::CaseInsensitive) || t.contains("Add to Custom Colors", Qt::CaseInsensitive)) {
            b->setText(QObject::tr("添加到自定义颜色"));
        }
    }

    // 组标题或标签：Basic colors / Custom colors
    const auto groups = cd->findChildren<QGroupBox*>();
    for (auto* g : groups) {
        const QString t = normalize(g->title());
        if (t.contains("Basic colors", Qt::CaseInsensitive)) g->setTitle(QObject::tr("基本颜色"));
        else if (t.contains("Custom colors", Qt::CaseInsensitive)) g->setTitle(QObject::tr("自定义颜色"));
    }

    const auto labels = cd->findChildren<QLabel*>();
    for (auto* l : labels) {
        const QString t = normalize(l->text());
        if (t.startsWith("Hue", Qt::CaseInsensitive)) l->setText(QObject::tr("色相"));
        else if (t.startsWith("Sat", Qt::CaseInsensitive)) l->setText(QObject::tr("饱和度"));
        else if (t.startsWith("Val", Qt::CaseInsensitive)) l->setText(QObject::tr("明度"));
        else if (t.startsWith("Red", Qt::CaseInsensitive)) l->setText(QObject::tr("红"));
        else if (t.startsWith("Green", Qt::CaseInsensitive)) l->setText(QObject::tr("绿"));
        else if (t.startsWith("Blue", Qt::CaseInsensitive)) l->setText(QObject::tr("蓝"));
        else if (t.startsWith("HTML", Qt::CaseInsensitive)) l->setText(QObject::tr("HTML"));
        else if (t.contains("Basic colors", Qt::CaseInsensitive)) l->setText(QObject::tr("基本颜色"));
        else if (t.contains("Custom colors", Qt::CaseInsensitive)) l->setText(QObject::tr("自定义颜色"));
    }
}

// 事件过滤器：在对话框显示/布局后再次本地化，确保子控件已创建
class ColorDialogLocalizer : public QObject {
public:
    explicit ColorDialogLocalizer(QColorDialog* d) : QObject(d), cd(d) {}
protected:
    bool eventFilter(QObject* obj, QEvent* e) override {
        if (obj == cd) {
            switch (e->type()) {
            case QEvent::Show:
            case QEvent::Polish:
            case QEvent::LayoutRequest:
            case QEvent::LanguageChange:
                localizeColorDialogInternals(cd);
                break;
            default:
                break;
            }
        }
        return QObject::eventFilter(obj, e);
    }
private:
    QColorDialog* cd = nullptr;
};

// ===== QFileDialog 中文本地化：定义提前放在使用前 =====
static void localizeFileDialogInternals(QFileDialog* fd)
{
    if (!fd) return;
    auto normalize = [](QString s) {
        s.replace('&', "");
        s.replace('_', "");
        return s.trimmed();
    };

    // 底部按钮优先：Open/Save/Ok/Cancel/Close/Apply
    if (auto* box = fd->findChild<QDialogButtonBox*>()) {
        if (auto* b = box->button(QDialogButtonBox::Open)) b->setText(QObject::tr("打开"));
        if (auto* b = box->button(QDialogButtonBox::Save)) b->setText(QObject::tr("保存"));
        if (auto* b = box->button(QDialogButtonBox::Ok)) b->setText(QObject::tr("确定"));
        if (auto* b = box->button(QDialogButtonBox::Apply)) b->setText(QObject::tr("应用"));
        if (auto* b = box->button(QDialogButtonBox::Cancel)) b->setText(QObject::tr("取消"));
        if (auto* b = box->button(QDialogButtonBox::Close)) b->setText(QObject::tr("关闭"));
    }

    // 顶部和侧边工具按钮：Back/Forward/Parent/New Folder/Delete/List/Details
    const auto btns = fd->findChildren<QAbstractButton*>();
    for (auto* b : btns) {
        const QString t = normalize(b->text());
        if (t.contains("BACK", Qt::CaseInsensitive)) b->setText(QObject::tr("后退"));
        else if (t.contains("FORWARD", Qt::CaseInsensitive)) b->setText(QObject::tr("前进"));
        else if (t.contains("PARENT", Qt::CaseInsensitive)) b->setText(QObject::tr("上级文件夹"));
        else if (t.contains("NEW FOLDER", Qt::CaseInsensitive)) b->setText(QObject::tr("新建文件夹"));
        else if (t.contains("DELETE", Qt::CaseInsensitive)) b->setText(QObject::tr("删除"));
        else if (t.contains("LIST", Qt::CaseInsensitive)) b->setText(QObject::tr("列表视图"));
        else if (t.contains("DETAIL", Qt::CaseInsensitive)) b->setText(QObject::tr("详细信息"));
    }

    // 标签：Look in / File name / Files of type
    const auto labels = fd->findChildren<QLabel*>();
    for (auto* l : labels) {
        const QString t = normalize(l->text());
        if (t.startsWith("Look in", Qt::CaseInsensitive)) l->setText(QObject::tr("查看位置："));
        else if (t.startsWith("File name", Qt::CaseInsensitive)) l->setText(QObject::tr("文件名："));
        else if (t.contains("Files of type", Qt::CaseInsensitive) || t.contains("File type", Qt::CaseInsensitive)) l->setText(QObject::tr("文件类型："));
    }
}

class FileDialogLocalizer : public QObject {
public:
    explicit FileDialogLocalizer(QFileDialog* d) : QObject(d), fd(d) {}
protected:
    bool eventFilter(QObject* obj, QEvent* e) override {
        if (obj == fd) {
            switch (e->type()) {
            case QEvent::Show:
            case QEvent::Polish:
            case QEvent::LayoutRequest:
            case QEvent::LanguageChange:
                localizeFileDialogInternals(fd);
                break;
            default:
                break;
            }
        }
        return QObject::eventFilter(obj, e);
    }
private:
    QFileDialog* fd = nullptr;
};

QString DialogUtils::openFile(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("打开文件") : title);

    auto* fd = new QFileDialog(&dlg, title, initialDir, filter);
    fd->setFileMode(QFileDialog::ExistingFile);
    fd->setAcceptMode(QFileDialog::AcceptOpen);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget);

    // 立即本地化内部控件，并安装事件过滤器保证显示后也能替换
    localizeFileDialogInternals(fd);
    auto* fdLocalizer = new FileDialogLocalizer(fd);
    fd->installEventFilter(fdLocalizer);

    QObject::connect(fd, &QFileDialog::accepted, &dlg, &QDialog::accept);
    QObject::connect(fd, &QFileDialog::rejected, &dlg, &QDialog::reject);

    dlg.setContentWidget(makeContent(&dlg, fd, nullptr));

    if (dlg.exec() == QDialog::Accepted) {
        const QStringList files = fd->selectedFiles();
        if (!files.isEmpty()) return files.first();
    }
    return QString();
}

QStringList DialogUtils::openFiles(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择文件") : title);

    auto* fd = new QFileDialog(&dlg, title, initialDir, filter);
    fd->setFileMode(QFileDialog::ExistingFiles);
    fd->setAcceptMode(QFileDialog::AcceptOpen);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget);

    localizeFileDialogInternals(fd);
    auto* fdLocalizer = new FileDialogLocalizer(fd);
    fd->installEventFilter(fdLocalizer);

    QObject::connect(fd, &QFileDialog::accepted, &dlg, &QDialog::accept);
    QObject::connect(fd, &QFileDialog::rejected, &dlg, &QDialog::reject);

    dlg.setContentWidget(makeContent(&dlg, fd, nullptr));

    if (dlg.exec() == QDialog::Accepted) {
        return fd->selectedFiles();
    }
    return {};
}

QString DialogUtils::saveFile(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("保存文件") : title);

    auto* fd = new QFileDialog(&dlg, title, initialDir, filter);
    fd->setFileMode(QFileDialog::AnyFile);
    fd->setAcceptMode(QFileDialog::AcceptSave);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget);

    localizeFileDialogInternals(fd);
    auto* fdLocalizer = new FileDialogLocalizer(fd);
    fd->installEventFilter(fdLocalizer);

    QObject::connect(fd, &QFileDialog::accepted, &dlg, &QDialog::accept);
    QObject::connect(fd, &QFileDialog::rejected, &dlg, &QDialog::reject);

    dlg.setContentWidget(makeContent(&dlg, fd, nullptr));

    if (dlg.exec() == QDialog::Accepted) {
        const QStringList files = fd->selectedFiles();
        if (!files.isEmpty()) return files.first();
    }
    return QString();
}

QString DialogUtils::openDirectory(QWidget* parent, const QString& title, const QString& initialDir)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择文件夹") : title);

    auto* fd = new QFileDialog(&dlg, title, initialDir);
    fd->setFileMode(QFileDialog::Directory);
    fd->setOption(QFileDialog::ShowDirsOnly, true);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget);

    localizeFileDialogInternals(fd);
    auto* fdLocalizer = new FileDialogLocalizer(fd);
    fd->installEventFilter(fdLocalizer);

    QObject::connect(fd, &QFileDialog::accepted, &dlg, &QDialog::accept);
    QObject::connect(fd, &QFileDialog::rejected, &dlg, &QDialog::reject);

    dlg.setContentWidget(makeContent(&dlg, fd, nullptr));

    if (dlg.exec() == QDialog::Accepted) {
        const QStringList dirs = fd->selectedFiles();
        if (!dirs.isEmpty()) return dirs.first();
    }
    return QString();
}

QColor DialogUtils::pickColor(QWidget* parent, const QColor& initial, const QString& title)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择颜色") : title);

    auto* cd = new QColorDialog(&dlg);
    cd->setOption(QColorDialog::DontUseNativeDialog, true);
    cd->setOption(QColorDialog::NoButtons, false); // use native buttons
    cd->setCurrentColor(initial);
    cd->setWindowFlags(Qt::Widget);

    QObject::connect(cd, &QColorDialog::accepted, &dlg, &QDialog::accept);
    QObject::connect(cd, &QColorDialog::rejected, &dlg, &QDialog::reject);

    // 本地化按钮文本（底部）
    if (auto* box = cd->findChild<QDialogButtonBox*>()) {
        if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
        if (auto* cancelBtn = box->button(QDialogButtonBox::Cancel)) cancelBtn->setText(QObject::tr("取消"));
        if (auto* resetBtn = box->button(QDialogButtonBox::Reset)) resetBtn->setText(QObject::tr("重置"));
    }
    // 额外本地化内部英文控件（立即尝试一次）
    localizeColorDialogInternals(cd);
    // 安装事件过滤器，确保显示后也能本地化
    auto* localizer = new ColorDialogLocalizer(cd);
    cd->installEventFilter(localizer);

    dlg.setContentWidget(makeContent(&dlg, cd, nullptr));

    if (dlg.exec() == QDialog::Accepted) return cd->currentColor();
    return QColor();
}

QFont DialogUtils::pickFont(QWidget* parent, const QFont& initial, bool* okOut, const QString& title)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择字体") : title);

    auto* fd = new QFontDialog(initial, &dlg);
    fd->setWindowFlags(Qt::Widget);

    QObject::connect(fd, &QFontDialog::accepted, &dlg, &QDialog::accept);
    QObject::connect(fd, &QFontDialog::rejected, &dlg, &QDialog::reject);

    // 本地化按钮文本
    if (auto* box = fd->findChild<QDialogButtonBox*>()) {
        if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
        if (auto* cancelBtn = box->button(QDialogButtonBox::Cancel)) cancelBtn->setText(QObject::tr("取消"));
        if (auto* applyBtn = box->button(QDialogButtonBox::Apply)) applyBtn->setText(QObject::tr("应用"));
    }

    dlg.setContentWidget(makeContent(&dlg, fd, nullptr));

    const bool ok = (dlg.exec() == QDialog::Accepted);
    if (okOut) *okOut = ok;
    return ok ? fd->selectedFont() : QFont();
}

void DialogUtils::messageInfo(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    auto* body = new QWidget(&dlg);
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    iconLabel->setPixmap(icon.pixmap(32, 32));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

    dlg.setContentWidget(makeContent(&dlg, body, box));
    dlg.exec();
}

void DialogUtils::messageWarning(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    auto* body = new QWidget(&dlg);
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    iconLabel->setPixmap(icon.pixmap(32, 32));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
 
    dlg.setContentWidget(makeContent(&dlg, body, box));
    dlg.exec();
}

void DialogUtils::messageError(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    auto* body = new QWidget(&dlg);
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
    iconLabel->setPixmap(icon.pixmap(32, 32));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

    dlg.setContentWidget(makeContent(&dlg, body, box));
    dlg.exec();
}

bool DialogUtils::askQuestion(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    auto* body = new QWidget(&dlg);
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    iconLabel->setPixmap(icon.pixmap(32, 32));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, &dlg);
    if (auto* yesBtn = box->button(QDialogButtonBox::Yes)) yesBtn->setText(QObject::tr("是"));
    if (auto* noBtn = box->button(QDialogButtonBox::No)) noBtn->setText(QObject::tr("否"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
 
    dlg.setContentWidget(makeContent(&dlg, body, box));
    return dlg.exec() == QDialog::Accepted;
}

QString DialogUtils::inputText(QWidget* parent, const QString& title, const QString& label, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    QWidget* body = new QWidget(&dlg);
    auto* v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);
    QLabel* lbl = new QLabel(label, body);
    QLineEdit* edit = new QLineEdit(text, body);
    v->addWidget(lbl);
    v->addWidget(edit);
    body->setLayout(v);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* cancelBtn = box->button(QDialogButtonBox::Cancel)) cancelBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
 
    dlg.setContentWidget(makeContent(&dlg, body, box));
    if (dlg.exec() == QDialog::Accepted) return edit->text();
    return QString();
}

int DialogUtils::inputInt(QWidget* parent, const QString& title, const QString& label, int value, int min, int max, int step)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    QWidget* body = new QWidget(&dlg);
    auto* v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);
    QLabel* lbl = new QLabel(label, body);
    QSpinBox* spin = new QSpinBox(body);
    spin->setRange(min, max);
    spin->setSingleStep(step);
    spin->setValue(value);
    v->addWidget(lbl);
    v->addWidget(spin);
    body->setLayout(v);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* cancelBtn = box->button(QDialogButtonBox::Cancel)) cancelBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
 
    dlg.setContentWidget(makeContent(&dlg, body, box));
    if (dlg.exec() == QDialog::Accepted) return spin->value();
    return value;
}

double DialogUtils::inputDouble(QWidget* parent, const QString& title, const QString& label, double value, double min, double max, int decimals)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    QWidget* body = new QWidget(&dlg);
    auto* v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);
    QLabel* lbl = new QLabel(label, body);
    QDoubleSpinBox* spin = new QDoubleSpinBox(body);
    spin->setRange(min, max);
    spin->setDecimals(decimals);
    spin->setValue(value);
    v->addWidget(lbl);
    v->addWidget(spin);
    body->setLayout(v);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* cancelBtn = box->button(QDialogButtonBox::Cancel)) cancelBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    dlg.setContentWidget(makeContent(&dlg, body, box));
    if (dlg.exec() == QDialog::Accepted) return spin->value();
    return value;
}

} // namespace QtAdvancedUI