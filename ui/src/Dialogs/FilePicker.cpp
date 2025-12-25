#include "FilePicker.h"

#include <QFileSystemModel>
#include <QTreeView>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QListWidget>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QPixmap>
#include <QSplitter>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QStorageInfo>

namespace QtAdvancedUI {

static QStringList parseNameFilter(const QString& filter)
{
    // 支持 "Images (*.png *.jpg)" 或 "*.png;*.jpg" 或 "*.png *.jpg"
    QString f = filter;
    int l = f.indexOf('(');
    int r = f.indexOf(')');
    if (l >= 0 && r > l) f = f.mid(l + 1, r - l - 1);
    f = f.replace(';', ' ');
    QStringList tokens = f.split(' ', QString::SkipEmptyParts);
    for (QString& t : tokens) t = t.trimmed();
    return tokens;
}

QWidget* FilePicker::makeFilePickerBody(QWidget* parent,
                                        const QString& initialDir,
                                        const QString& filter,
                                        QString* selectedPathOut,
                                        bool directoryOnly,
                                        bool multiSelect,
                                        bool saveMode,
                                        QTreeView** outView,
                                        QFileSystemModel** outModel,
                                        QLineEdit** outEdit)
{
    QWidget* body = new QWidget(parent);
    auto* v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);

    // 顶部导航行：查看位置 + 后退/前进/上级/新建/删除
    auto* navRow = new QHBoxLayout();
    QToolButton* btnBack = new QToolButton(body); btnBack->setText("←");
    QToolButton* btnForward = new QToolButton(body); btnForward->setText("→");
    QToolButton* btnUp = new QToolButton(body); btnUp->setText("↑");
    QToolButton* btnNewDirTop = new QToolButton(body); btnNewDirTop->setText("＋");
    QToolButton* btnDeleteTop = new QToolButton(body); btnDeleteTop->setText(QObject::tr("删"));
    QComboBox* addrBox = new QComboBox(body); addrBox->setEditable(true);
    QLineEdit* addrEdit = addrBox->lineEdit();
    QCheckBox* showHidden = new QCheckBox(QObject::tr("显示隐藏"), body);
    navRow->addWidget(new QLabel(QObject::tr("查看位置:"), body));
    navRow->addWidget(addrBox, 1);
    navRow->addWidget(btnBack);
    navRow->addWidget(btnForward);
    navRow->addWidget(btnUp);
    navRow->addWidget(btnNewDirTop);
    navRow->addWidget(btnDeleteTop);
    navRow->addWidget(showHidden);
    v->addLayout(navRow);

    // 主区：常用位置 + 视图 + 预览
    QSplitter* splitter = new QSplitter(Qt::Horizontal, body);
    splitter->setChildrenCollapsible(false);

    // 左侧面板：常用位置 + 驱动器列表
    QListWidget* places = new QListWidget(splitter);
    places->setMinimumWidth(180);
    auto addPlace = [&](const QString& name, const QString& path){
        QListWidgetItem* it = new QListWidgetItem(name, places);
        it->setData(Qt::UserRole, path);
    };
    addPlace(QObject::tr("桌面"), QDir::homePath() + "/Desktop");
    addPlace(QObject::tr("文档"), QDir::homePath() + "/Documents");
    addPlace(QObject::tr("下载"), QDir::homePath() + "/Downloads");
    addPlace(QObject::tr("家目录"), QDir::homePath());
    // 分隔标题：磁盘
    QListWidgetItem* sep = new QListWidgetItem(QObject::tr("——  磁盘  ——"), places);
    sep->setFlags(Qt::ItemIsEnabled);
    sep->setData(Qt::UserRole, QString());
    // 枚举可用驱动器（Windows：C:\、D:\ 等；其他平台：挂载点）
    const auto volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo& si : volumes) {
        if (!si.isValid() || si.isReadOnly()) {
            // 只读也可浏览，仍然加入
        }
        QString root = si.rootPath();
        if (root.isEmpty()) continue;
        QString label = si.displayName().isEmpty() ? root : QString("%1 (%2)").arg(si.displayName(), root);
        addPlace(label, root);
    }

    QFileSystemModel* model = new QFileSystemModel(splitter);
    model->setReadOnly(false);
    QString rootPath = initialDir.isEmpty() ? QDir::homePath() : initialDir;
    model->setRootPath(rootPath);
    model->setFilter(directoryOnly ? (QDir::AllDirs | QDir::NoDotAndDotDot) : (QDir::AllEntries | QDir::NoDotAndDotDot));
    QStringList nameFilters = parseNameFilter(filter);
    if (!nameFilters.isEmpty()) {
        model->setNameFilters(nameFilters);
        model->setNameFilterDisables(false);
    }

    QTreeView* view = new QTreeView(splitter);
    view->setModel(model);
    view->setRootIndex(model->index(model->rootPath()));
    view->setSelectionMode(multiSelect ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
    view->setHeaderHidden(false);
    view->setSortingEnabled(true);
    view->sortByColumn(0, Qt::AscendingOrder);

    QWidget* previewPane = new QWidget(splitter);
    auto* pv = new QVBoxLayout(previewPane);
    pv->setContentsMargins(6,6,6,6); pv->setSpacing(6);
    QLabel* previewIcon = new QLabel(previewPane);
    previewIcon->setFixedSize(160, 120);
    previewIcon->setFrameShape(QFrame::Box);
    QLabel* previewInfo = new QLabel(previewPane);
    previewInfo->setWordWrap(true);
    pv->addWidget(previewIcon, 0, Qt::AlignTop);
    pv->addWidget(previewInfo);
    previewPane->setLayout(pv);

    splitter->addWidget(places);
    splitter->addWidget(view);
    splitter->addWidget(previewPane);
    splitter->setStretchFactor(1, 1);
    v->addWidget(splitter, 1);

    // 底部：文件名 + 文件类型
    QLineEdit* nameEdit = new QLineEdit(body);
    QComboBox* typeBox = new QComboBox(body);
    auto* nameRow = new QHBoxLayout();
    nameRow->addWidget(new QLabel(QObject::tr("文件名:"), body));
    nameRow->addWidget(nameEdit, 1);
    nameRow->addWidget(new QLabel(QObject::tr("文件类型:"), body));
    nameRow->addWidget(typeBox);
    v->addLayout(nameRow);

    // 更新预览
    auto updatePreview = [&](const QString& path){
        if (path.isEmpty()) { previewIcon->setPixmap(QPixmap()); previewInfo->setText(QString()); return; }
        QFileInfo fi(path);
        previewInfo->setText(QString("%1\n%2\n%3")
                             .arg(fi.fileName())
                             .arg(fi.isDir() ? QObject::tr("文件夹") : QObject::tr("文件"))
                             .arg(fi.absolutePath()));
        if (fi.isFile()) {
            const QString ext = fi.suffix().toLower();
            if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "gif") {
                QPixmap pm(path);
                previewIcon->setPixmap(pm.scaled(previewIcon->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                previewIcon->setPixmap(QPixmap());
            }
        } else {
            previewIcon->setPixmap(QPixmap());
        }
    };

    // 地址设置与导航
    // 历史记录：后退/前进
    QVector<QString> history; int histIndex = -1;
    auto updateNavButtons = [&](){
        btnBack->setEnabled(histIndex > 0);
        btnForward->setEnabled(histIndex >= 0 && histIndex < history.size() - 1);
    };
    auto pushHistory = [&](const QString& ap){
        if (history.isEmpty() || history.value(histIndex) != ap) {
            // 截断前进栈
            while (history.size() > histIndex + 1 && histIndex >= 0) history.removeLast();
            history.append(ap);
            histIndex = history.size() - 1;
        }
        updateNavButtons();
        // 记录到地址下拉
        int idx = addrBox->findText(ap);
        if (idx < 0) addrBox->addItem(ap);
        addrBox->setEditText(ap);
    };

    auto setRoot = [&](const QString& p){
        QString ap = QDir(p).absolutePath();
        if (ap.isEmpty()) return;
        if (!QDir(ap).exists()) return;
        addrBox->setEditText(ap);
        model->setRootPath(ap);
        view->setRootIndex(model->index(ap));
        pushHistory(ap);
    };
    addrBox->setEditText(rootPath);
    pushHistory(rootPath);
    QObject::connect(addrEdit, &QLineEdit::returnPressed, [=](){ setRoot(addrEdit->text()); });
    QObject::connect(btnUp, &QToolButton::clicked, [=](){ QDir d(addrEdit->text()); d.cdUp(); setRoot(d.absolutePath()); });
    QObject::connect(btnBack, &QToolButton::clicked, [&](){ if (histIndex > 0) { --histIndex; setRoot(history[histIndex]); } });
    QObject::connect(btnForward, &QToolButton::clicked, [&](){ if (histIndex >= 0 && histIndex < history.size() - 1) { ++histIndex; setRoot(history[histIndex]); } });
    QObject::connect(places, &QListWidget::itemClicked, [=](QListWidgetItem* it){
        const QString path = it->data(Qt::UserRole).toString();
        if (path.isEmpty()) return;
        setRoot(path);
    });

    QObject::connect(showHidden, &QCheckBox::toggled, [=](bool on){
        QDir::Filters f = model->filter();
        if (on) f |= QDir::Hidden; else f &= ~QDir::Hidden;
        model->setFilter(f);
    });

    // 选中项变化
    QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection&, const QItemSelection&){
        auto* sel = view->selectionModel();
        if (!sel) return;
        const auto idxs = sel->selectedIndexes();
        QString path;
        for (const QModelIndex& idx : idxs) {
            if (idx.column() == 0) { path = model->filePath(idx); break; }
        }
        updatePreview(path);
        if (!saveMode && !multiSelect && selectedPathOut) *selectedPathOut = path;
        if (!path.isEmpty()) nameEdit->setText(QFileInfo(path).fileName());
    });

    // 双击进入目录或选中文件
    QObject::connect(view, &QTreeView::doubleClicked, [=](const QModelIndex& idx){
        QFileInfo fi(model->filePath(idx));
        if (fi.isDir()) setRoot(fi.absoluteFilePath());
        else if (!saveMode && !multiSelect && selectedPathOut) *selectedPathOut = fi.absoluteFilePath();
    });

    // 输出指针
    if (outView) *outView = view;
    if (outModel) *outModel = model;
    if (outEdit) *outEdit = nameEdit;

    // 操作：新建文件夹（顶部按钮）
    auto createNewDir = [=](){
        const QString base = addrEdit->text();
        if (base.isEmpty()) return;
        bool ok = false;
        const QString name = QInputDialog::getText(body, QObject::tr("新建文件夹"), QObject::tr("名称:"), QLineEdit::Normal, QObject::tr("新建文件夹"), &ok);
        if (!ok || name.trimmed().isEmpty()) return;
        QDir dir(base);
        if (dir.exists(name)) {
            QMessageBox::warning(body, QObject::tr("已存在"), QObject::tr("同名文件夹已存在。"));
            return;
        }
        if (!dir.mkdir(name)) {
            QMessageBox::critical(body, QObject::tr("失败"), QObject::tr("创建文件夹失败。"));
            return;
        }
        setRoot(dir.filePath(name));
    };
    QObject::connect(btnNewDirTop, &QToolButton::clicked, createNewDir);

    // 操作：删除（文件或文件夹）
    QObject::connect(btnDeleteTop, &QToolButton::clicked, [=](){
        auto* sel = view->selectionModel();
        if (!sel) return;
        const auto idxs = sel->selectedIndexes();
        QModelIndex target;
        for (const QModelIndex& idx : idxs) if (idx.column() == 0) { target = idx; break; }
        if (!target.isValid()) return;
        const QString path = model->filePath(target);
        QFileInfo fi(path);
        const QString msg = fi.isDir() ? QObject::tr("确认删除文件夹及其内容？") : QObject::tr("确认删除该文件？");
        if (QMessageBox::question(body, QObject::tr("删除"), msg) != QMessageBox::Yes) return;
        bool ok = false;
        if (fi.isDir()) {
            QDir d(fi.absoluteFilePath());
            ok = d.removeRecursively();
        } else {
            QFile f(fi.absoluteFilePath());
            ok = f.remove();
        }
        if (!ok) QMessageBox::critical(body, QObject::tr("失败"), QObject::tr("删除失败。"));
    });

    // 文件类型下拉：解析并应用过滤
    QString displayFilter = filter.isEmpty() ? QObject::tr("所有文件 (*)") : filter;
    if (!nameFilters.isEmpty()) {
        typeBox->addItem(displayFilter);
        for (const QString& nf : nameFilters) {
            if (nf != "*") typeBox->addItem(nf);
        }
    } else {
        typeBox->addItem(QObject::tr("所有文件 (*)"));
    }
    QObject::connect(typeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int idx){
        if (idx <= 0 && !nameFilters.isEmpty()) {
            model->setNameFilters(nameFilters);
            model->setNameFilterDisables(false);
        } else {
            QStringList nf;
            const QString text = typeBox->currentText();
            if (text == QObject::tr("所有文件 (*)")) nf = QStringList("*");
            else nf = QStringList(text);
            model->setNameFilters(nf);
            model->setNameFilterDisables(false);
        }
    });

    body->setLayout(v);
    return body;
}

} // namespace QtAdvancedUI
