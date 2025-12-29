/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */

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

// Internal storage for the last picked color, shared across the library
// This static variable ensures color persistence throughout the application lifecycle
static QColor s_lastPickedColor = QColor("#26A69A");

QColor DialogUtils::getLastPickedColor()
{
    // Return the last color picked by the user, stored persistently in the library
    // This allows different parts of the UI to access the most recently used color
    return s_lastPickedColor;
}

static QWidget* makeContent(QWidget* parent, QWidget* body, QDialogButtonBox* box)
{
    QWidget* content = new QWidget(parent);
    auto* v = new QVBoxLayout(content);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(10);
    v->addWidget(body);
    if (box) {
        // box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        // Make buttons expand proportionally
        // const auto buttons = box->buttons();
        // for (auto* btn : buttons) {
        //      btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        //      btn->setMinimumHeight(32); // Ensure decent height
        // }
        v->addWidget(box);
    }
    content->setLayout(v);
    return content;
}

// 统一风格的圆形彩色徽标图标
static QPixmap makeBadgeIcon(const QColor& circleColor, const QString& shape, const QColor& shapeColor)
{
    QPixmap pixmap;
    int size = 32;
    if (shape == "check") {
        pixmap = StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/circle_check.svg").pixmap(size,size);
    } else if (shape == "cross") {
        pixmap = StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/circle_xmark.svg").pixmap(size,size);
    } else if (shape == "info") {
        pixmap = StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/circle_info.svg").pixmap(size,size);
    } else if (shape == "question") {
        pixmap = StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/circle_question.svg").pixmap(size,size);
    } else if (shape == "warning") {
        pixmap = StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/circle_exclamation.svg").pixmap(size,size);
    }
    return pixmap;
}



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



QString DialogUtils::openFile(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("打开文件") : title);
    
    auto* widget = new FilePickerWidget(&dlg, initialDir, filter, FilePickerWidget::OpenFile);
    widget->setProperty("msgContent", QVariant(true));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(widget, &FilePickerWidget::fileActivated, &dlg, &QDialog::accept);

    dlg.setContentWidget(makeContent(&dlg, widget, box));
    if (dlg.exec() == QDialog::Accepted) {
        return widget->selectedPath();
    }
    return QString();
}

QStringList DialogUtils::openFiles(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择文件") : title);
    
    auto* widget = new FilePickerWidget(&dlg, initialDir, filter, FilePickerWidget::OpenFiles);
    widget->setProperty("msgContent", QVariant(true));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    // OpenFiles mode typically doesn't auto-accept on double click of a single file, usually requires selection and OK.
    // But double clicking a file adds it? Standard QFileDialog double click on file in multi-select mode acts as "add and maybe close if it's the only one"?
    // Actually FilePickerWidget emits fileActivated on double click. If we want to support double click to select one file and close, we can connect it.
    // However, for multiple files, usually you select multiple and click OK.
    // If user double clicks one file, it might be interpreted as "I just want this one".
    // Let's connect it for convenience.
    QObject::connect(widget, &FilePickerWidget::fileActivated, &dlg, &QDialog::accept);

    dlg.setContentWidget(makeContent(&dlg, widget, box));
    if (dlg.exec() == QDialog::Accepted) {
        return widget->selectedPaths();
    }
    return QStringList{};
}

QString DialogUtils::saveFile(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("保存文件") : title);
    
    auto* widget = new FilePickerWidget(&dlg, initialDir, filter, FilePickerWidget::SaveFile);
    widget->setProperty("msgContent", QVariant(true));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(widget, &FilePickerWidget::fileActivated, &dlg, &QDialog::accept);

    dlg.setContentWidget(makeContent(&dlg, widget, box));
    if (dlg.exec() == QDialog::Accepted) {
        return widget->selectedPath();
    }
    return QString();
}

QString DialogUtils::openDirectory(QWidget* parent, const QString& title, const QString& initialDir)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择文件夹") : title);
    
    auto* widget = new FilePickerWidget(&dlg, initialDir, QString(), FilePickerWidget::OpenDirectory);
    widget->setProperty("msgContent", QVariant(true));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    // Double click on directory in OpenDirectory mode enters it (handled by widget).
    // It doesn't usually select it and close.
    // So we don't connect fileActivated here.
    
    dlg.setContentWidget(makeContent(&dlg, widget, box));
    if (dlg.exec() == QDialog::Accepted) {
        return widget->selectedPath();
    }
    return QString();
}

QColor DialogUtils::pickColor(QWidget* parent, const QColor& initial, const QString& title)
{
    // If initial color is not provided (invalid), use the last picked color from library memory.
    // If initial is valid, use it (but don't update global memory until confirmed).
    QColor effectiveInitial = initial.isValid() ? initial : s_lastPickedColor;

    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择颜色") : title);
    
    auto* widget = new ColorPickerWidget(&dlg, effectiveInitial);
    widget->setProperty("msgContent", QVariant(true));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) {
        okBtn->setText(QObject::tr("确定"));
        okBtn->setProperty("primary", QVariant(true));
    }
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    
    dlg.setContentWidget(makeContent(&dlg, widget, box));
    if (dlg.exec() == QDialog::Accepted) {
        QColor c = widget->currentColor();
        // Update global memory on confirmation
        s_lastPickedColor = c;
        return c;
    }
    // If canceled or closed, return the effective initial color.
    // This ensures we return the last valid state (either what the user passed, or the global memory).
    return effectiveInitial;
}

QFont DialogUtils::pickFont(QWidget* parent, const QFont& initial, bool* okOut, const QString& title)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择字体") : title);
    QFontDialog* fd = new QFontDialog(initial, &dlg);
    fd->setProperty("msgContent", QVariant(true));
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
    body->setProperty("msgContent", QVariant(true));
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(QColor(35, 155, 249), "info", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setObjectName("DialogTextLabel");
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
    body->setProperty("msgContent", QVariant(true));
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(QColor(255, 166, 0), "warning", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setObjectName("DialogTextLabel");
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
    body->setProperty("msgContent", QVariant(true));
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(QColor(242, 94, 67), "cross", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setObjectName("DialogTextLabel");
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
    body->setProperty("msgContent", QVariant(true));
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(QColor(76, 175, 80), "check", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setObjectName("DialogTextLabel");
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
    body->setProperty("msgContent", QVariant(true));
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(QColor(35, 155, 249), "question", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setObjectName("DialogTextLabel");
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
    body->setProperty("msgContent", QVariant(true));
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
    body->setProperty("msgContent", QVariant(true));
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
    body->setProperty("msgContent", QVariant(true));
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
