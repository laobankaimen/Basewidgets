#include <QtAdvancedUI/Dialogs/DialogUtils.h>
#include <QtAdvancedUI/Core/FramelessDialog.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

#include "ColorPicker.h"
#include "FontPicker.h"
#include "FilePicker.h"

#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QColorDialog>
#include <QFontDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QIcon>
#include <QApplication>
#include <QAbstractButton>
#include <QGroupBox>
#include <QEvent>
#include <QCheckBox>
#include <QFileSystemModel>
#include <QTreeView>
#include <QComboBox>
#include <QRegularExpression>
#include <QFontDatabase>
#include <QDir>
#include <QFrame>
#include <QPainter>
#include <QMouseEvent>
#include <QImage>
#include <QPixmap>
#include <QListWidget>
#include <QToolButton>
#include <QSplitter>

using namespace acss;

namespace QtAdvancedUI {

// 颜色选择区域已拆分至 ColorPicker.cpp

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

// 统一风格的圆形彩色徽标图标
static QPixmap makeBadgeIcon(const QColor& circleColor, const QString& shape, const QColor& shapeColor)
{
    QPixmap pixmap;
    if (shape == "check") {
        pixmap = StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/circle_check.svg").pixmap(40,40);
    } else if (shape == "cross") {
        pixmap = StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/circle_xmark.svg").pixmap(40,40);
    } else if (shape == "info") {
        pixmap = StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/circle_info.svg").pixmap(40,40);
    } else if (shape == "question") {
        pixmap = StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/circle_question.svg").pixmap(40,40);
    } else if (shape == "warning") {
        pixmap = StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/images/icons/circle_exclamation.svg").pixmap(40,40);
    }
    return pixmap;
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

// 额外本地化 QFontDialog 内部英文标签与控件
static void localizeFontDialogInternals(QFontDialog* fd)
{
    if (!fd) return;

    auto normalize = [](QString s) {
        s.replace('&', "");
        s.replace('_', "");
        return s.trimmed();
    };

    // 组标题或标签：Font / Font style / Style / Size / Writing System / Sample / Effects
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
        // 注意顺序：先匹配 "font style"，避免被 "font" 误伤
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

// 事件过滤器：在字体对话框显示/布局后再次本地化
class FontDialogLocalizer : public QObject {
public:
    explicit FontDialogLocalizer(QFontDialog* d) : QObject(d), fd(d) {}
protected:
    bool eventFilter(QObject* obj, QEvent* e) override {
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

// ===== QFileDialog 中文本地化：定义提前放在使用前 =====
static void localizeFileDialogInternals(QFileDialog* fd)
{
    if (!fd) return;
    auto normalize = [](QString s) {
        s.replace('&', "");
        s.replace('_', "");
        return s.trimmed();
    };

    // 底部按钮（QFileDialog 内部自带）：本库外部统一提供按钮，为避免重复，这里仅本地化文本后隐藏内部按钮栏
    if (auto* box = fd->findChild<QDialogButtonBox*>()) {
        if (auto* b = box->button(QDialogButtonBox::Open)) b->setText(QObject::tr("打开"));
        if (auto* b = box->button(QDialogButtonBox::Save)) b->setText(QObject::tr("保存"));
        if (auto* b = box->button(QDialogButtonBox::Ok)) b->setText(QObject::tr("确定"));
        if (auto* b = box->button(QDialogButtonBox::Apply)) b->setText(QObject::tr("应用"));
        if (auto* b = box->button(QDialogButtonBox::Cancel)) b->setText(QObject::tr("取消"));
        if (auto* b = box->button(QDialogButtonBox::Close)) b->setText(QObject::tr("关闭"));
        // 隐藏内部按钮栏，使用外部统一的 QDialogButtonBox（确定/取消）
        box->setVisible(false);
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
    QFileDialog* fd = new QFileDialog(&dlg, QString(), initialDir.isEmpty() ? QDir::homePath() : initialDir);
    fd->setFileMode(QFileDialog::ExistingFile);
    fd->setViewMode(QFileDialog::Detail);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget); // 作为主体嵌入
    if (!filter.isEmpty()) fd->setNameFilters(filter.split(";;", QString::SkipEmptyParts));
    fd->installEventFilter(new FileDialogLocalizer(fd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, fd, box));
    return dlg.exec() == QDialog::Accepted ? fd->selectedFiles().value(0) : QString();
}

QStringList DialogUtils::openFiles(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择文件") : title);
    QFileDialog* fd = new QFileDialog(&dlg, QString(), initialDir.isEmpty() ? QDir::homePath() : initialDir);
    fd->setFileMode(QFileDialog::ExistingFiles);
    fd->setViewMode(QFileDialog::Detail);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget);
    if (!filter.isEmpty()) fd->setNameFilters(filter.split(";;", QString::SkipEmptyParts));
    fd->installEventFilter(new FileDialogLocalizer(fd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, fd, box));
    return dlg.exec() == QDialog::Accepted ? fd->selectedFiles() : QStringList{};
}

QString DialogUtils::saveFile(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("保存文件") : title);
    QFileDialog* fd = new QFileDialog(&dlg, QString(), initialDir.isEmpty() ? QDir::homePath() : initialDir);
    fd->setAcceptMode(QFileDialog::AcceptSave);
    fd->setFileMode(QFileDialog::AnyFile);
    fd->setViewMode(QFileDialog::Detail);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget);
    if (!filter.isEmpty()) fd->setNameFilters(filter.split(";;", QString::SkipEmptyParts));
    fd->installEventFilter(new FileDialogLocalizer(fd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, fd, box));
    return dlg.exec() == QDialog::Accepted ? fd->selectedFiles().value(0) : QString();
}

QString DialogUtils::openDirectory(QWidget* parent, const QString& title, const QString& initialDir)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择文件夹") : title);
    QFileDialog* fd = new QFileDialog(&dlg, QString(), initialDir.isEmpty() ? QDir::homePath() : initialDir);
    fd->setFileMode(QFileDialog::Directory);
    fd->setOption(QFileDialog::ShowDirsOnly, true);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget);
    fd->installEventFilter(new FileDialogLocalizer(fd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, fd, box));
    return dlg.exec() == QDialog::Accepted ? fd->selectedFiles().value(0) : QString();
}

QColor DialogUtils::pickColor(QWidget* parent, const QColor& initial, const QString& title)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择颜色") : title);
    QColorDialog* cd = new QColorDialog(initial, &dlg);
    cd->setOption(QColorDialog::ShowAlphaChannel, true);
    cd->setOption(QColorDialog::DontUseNativeDialog, true);
    cd->setOption(QColorDialog::NoButtons, true); // 仅保留外部统一按钮
    cd->setWindowFlags(Qt::Widget);
    cd->installEventFilter(new ColorDialogLocalizer(cd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, cd, box));
    return dlg.exec() == QDialog::Accepted ? cd->currentColor() : QColor();
}

QFont DialogUtils::pickFont(QWidget* parent, const QFont& initial, bool* okOut, const QString& title)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择字体") : title);
    QFontDialog* fd = new QFontDialog(initial, &dlg);
    fd->setOption(QFontDialog::DontUseNativeDialog, true);
    fd->setOption(QFontDialog::NoButtons, true); // 仅保留外部统一按钮
    fd->setWindowFlags(Qt::Widget);
    fd->installEventFilter(new FontDialogLocalizer(fd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, fd, box));
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
    iconLabel->setPixmap(makeBadgeIcon(QColor(35, 155, 249), "info", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
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
    iconLabel->setPixmap(makeBadgeIcon(QColor(255, 166, 0), "warning", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
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
    iconLabel->setPixmap(makeBadgeIcon(QColor(242, 94, 67), "cross", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

    dlg.setContentWidget(makeContent(&dlg, body, box));
    dlg.exec();
}

void DialogUtils::messageSuccess(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    auto* body = new QWidget(&dlg);
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(QColor(76, 175, 80), "check", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
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
    iconLabel->setPixmap(makeBadgeIcon(QColor(35, 155, 249), "question", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
        okBtn->setDefault(true);
        okBtn->setAutoDefault(true);
    }
    if (auto* cancelBtn = box->button(QDialogButtonBox::Cancel)) { cancelBtn->setText(QObject::tr("取消")); }
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
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
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
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
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
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* cancelBtn = box->button(QDialogButtonBox::Cancel)) cancelBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    dlg.setContentWidget(makeContent(&dlg, body, box));
    if (dlg.exec() == QDialog::Accepted) return spin->value();
    return value;
}

} // namespace QtAdvancedUI
