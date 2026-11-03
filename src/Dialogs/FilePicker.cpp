/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */

#include "FilePicker.h"
#include <basewidgets/Core/StyleManagerAdapter.h>

#include <QFileSystemModel>
#include <QTreeView>
#include <QListView>
#include <QStackedWidget>
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
#include <QFileIconProvider>
#include <QItemSelectionModel>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QDate>
#include <QDateTime>
#include <QHeaderView>
#include <QDebug>
#include <QStyle>
#include <QAction>
#include <QMenu>

namespace basewidgets {

class PickerFileModel : public QFileSystemModel {
public:
    using QFileSystemModel::QFileSystemModel;
    QVariant headerData(int section,Qt::Orientation orientation,int role) const override {
        if(orientation==Qt::Horizontal && role==Qt::DisplayRole) {
            switch(section) { case 0:return tr("名称"); case 1:return tr("大小"); case 2:return tr("类型"); case 3:return tr("修改时间"); }
        }
        return QFileSystemModel::headerData(section,orientation,role);
    }
};

static QStringList parseNameFilter(const QString& filter)
{
    // Format: "Images (*.png *.jpg)" or "*.txt"
    QString f = filter;
    int l = f.indexOf('(');
    int r = f.indexOf(')');
    if (l >= 0 && r > l) f = f.mid(l + 1, r - l - 1);
    f = f.replace(';', ' ');
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList tokens = f.split(' ', Qt::SkipEmptyParts);
#else
    QStringList tokens = f.split(' ', QString::SkipEmptyParts);
#endif
    for (QString& t : tokens) t = t.trimmed();
    return tokens;
}

FilePickerWidget::FilePickerWidget(QWidget* parent,
                                   const QString& initialDir,
                                   const QString& filter,
                                   Mode mode)
    : QWidget(parent), m_mode(mode)
{
    setObjectName("FilePicker");
    // Construct controls before filesystem callbacks can reach them.
    setupUi(initialDir, filter);
}

QString FilePickerWidget::selectedPath() const
{
    // Get the path from the name edit field, handling absolute and relative paths
    QString name = m_nameEdit->text();
    if (m_mode == SaveFile) {
        name = name.trimmed();
    }
    if (name.isEmpty()) return m_mode==OpenDirectory?m_model->rootPath():m_selectedPath;
    
    QFileInfo fi(name);
    if (fi.isAbsolute()) return name;
    
    return QDir(m_model->rootPath()).filePath(name);
}

QStringList FilePickerWidget::selectedPaths() const
{
    // Return list of selected paths; if single selection, return it as a list
    if (m_selectedPaths.isEmpty()) {
        QString s = selectedPath();
        if (!s.isEmpty()) return QStringList() << s;
        return QStringList();
    }
    return m_selectedPaths;
}

void FilePickerWidget::setupUi(const QString& initialDir, const QString& filter)
{
    // Setup main layout with zero margins for a clean look
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- 1. Top Bar (Ubuntu-like Header) ---
    // The top bar contains navigation buttons (Back, Forward, Up, Home) and address bar
    m_topBar = new QFrame(this);
    m_topBar->setObjectName("FilePickerTopBar");
    m_topBar->setFixedHeight(qMax(40,fontMetrics().height()+16));

    auto* topLayout = new QHBoxLayout(m_topBar);
    topLayout->setContentsMargins(5, 5, 5, 5);
    topLayout->setSpacing(8);

    // Nav Buttons
    auto createNavBtn = [this](const QString& text, const QString& iconName) {
        Q_UNUSED(text);
        QToolButton* btn = new QToolButton(this);
        btn->setIcon(QIcon(QString("bwicon:buttons/%1.svg").arg(iconName)));
        btn->setAutoRaise(true);
        btn->setProperty("iconOnly", QVariant(true));
        return btn;
    };

    btnBack = createNavBtn("<", "arrow_left");
    btnBack->setToolTip(tr("返回"));
    
    btnForward = createNavBtn(">", "arrow_right");
    btnForward->setToolTip(tr("前进"));

    btnUp = createNavBtn("^", "angle_up");
    btnUp->setToolTip(tr("上一级"));

    btnHome = createNavBtn("Home", "home");
    btnHome->setToolTip(tr("家目录"));

    btnRefresh = createNavBtn("R", "arrow_refresh");
    btnRefresh->setToolTip(tr("刷新"));
    btnRefresh->setObjectName("FilePickerRefresh");

    // Address Bar
    addrEdit = new QLineEdit(this);
    addrEdit->setPlaceholderText(tr("输入文件夹路径…"));
    addrEdit->setFixedHeight(30);
    addrEdit->setMinimumWidth(80);

    btnGo = createNavBtn("跳转", "arrow_right");
    
    // View Mode Toggle
    btnViewMode = createNavBtn("View", "table_list");
    btnViewMode->setCheckable(true);
    btnViewMode->setToolTip(tr("视图切换"));

    topLayout->addWidget(btnBack);
    topLayout->addWidget(btnForward);
    topLayout->addWidget(btnUp);
    topLayout->addWidget(btnHome);
    topLayout->addWidget(addrEdit, 1); // Expand
    topLayout->addWidget(btnGo);
    topLayout->addWidget(btnRefresh);
    topLayout->addWidget(btnViewMode);

    mainLayout->addWidget(m_topBar);

    // --- 2. Main Area (Splitter: Sidebar + View) ---
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setHandleWidth(1);
    
    // Sidebar
    m_sidebar = new QListWidget(m_splitter);
    m_sidebar->setFrameShape(QFrame::NoFrame);
    m_sidebar->setFixedWidth(136);
    m_sidebar->setIconSize(QSize(20, 20));

    // Views
    m_viewStack = new QStackedWidget(m_splitter);
    
    m_model = new PickerFileModel(this);
    m_model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs);
    m_model->iconProvider()->setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);
    if(m_mode==OpenDirectory)m_model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    // Tree View (List Mode)
    m_treeView = new QTreeView(m_viewStack);
    m_treeView->setModel(m_model);
    m_treeView->setObjectName("FilePickerList");
    m_treeView->setAccessibleName(tr("文件列表"));
    m_treeView->setRootIsDecorated(false);
    m_treeView->setUniformRowHeights(true);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeView->setFrameShape(QFrame::NoFrame);
    m_treeView->setSelectionMode(m_mode == OpenFiles ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
    m_treeView->setSortingEnabled(true);
    m_treeView->setColumnWidth(0, 250); // Name
    m_treeView->setColumnWidth(1, 80);  // Size
    m_treeView->setColumnWidth(2, 72);  // Type
    m_treeView->header()->setStretchLastSection(false);
    m_treeView->header()->setSectionResizeMode(0,QHeaderView::Stretch);
    m_treeView->setColumnWidth(3,125);
    
    // Icon View (Grid Mode)
    m_iconView = new QListView(m_viewStack);
    m_iconView->setModel(m_model);
    m_iconView->setViewMode(QListView::IconMode);
    m_iconView->setFrameShape(QFrame::NoFrame);

    m_iconView->setGridSize(QSize(90, 110));
    m_iconView->setIconSize(QSize(64, 64));
    m_iconView->setResizeMode(QListView::Adjust);
    m_iconView->setUniformItemSizes(false); 
    m_iconView->setWordWrap(true);
    m_iconView->setTextElideMode(Qt::ElideMiddle);
    m_iconView->setSelectionMode(m_treeView->selectionMode());
    auto* unusedSelection=m_iconView->selectionModel();
    m_iconView->setSelectionModel(m_treeView->selectionModel());
    unusedSelection->deleteLater();
    // Ensure item text is centered and has padding

    m_viewStack->addWidget(m_treeView);
    m_viewStack->addWidget(m_iconView);

    m_splitter->addWidget(m_sidebar);
    m_splitter->addWidget(m_viewStack);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(m_splitter, 1);

    // --- 3. Bottom Bar (Inputs) ---
    // Only needed for picking files, but useful to see current selection
    m_bottomBar = new QFrame(this);
    m_bottomBar->setObjectName("FilePickerBottomBar");
    // Allow natural height expansion
    m_bottomBar->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    
    // Use Grid Layout for better alignment and height control
    auto* bottomLayout = new QGridLayout(m_bottomBar);
    bottomLayout->setContentsMargins(8, 6, 8, 4);
    bottomLayout->setVerticalSpacing(4);
    bottomLayout->setHorizontalSpacing(10);
    
    QLabel* lblName = new QLabel(tr("文件名:"), this);
    
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setObjectName("FilePickerName");
    lblName->setBuddy(m_nameEdit);
    m_nameEdit->setFixedHeight(qMax(28,fontMetrics().height()+8));

    QLabel* lblType = new QLabel(tr("文件类型:"), this);

    m_filterCombo = new QComboBox(this);
    m_filterCombo->setObjectName("FilePickerFilter");
    lblType->setBuddy(m_filterCombo);
    m_filterCombo->setFixedHeight(qMax(28,fontMetrics().height()+8));
    // Remove fixed width constraints to let it expand in grid
    m_filterCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Parse filter
    QStringList filters;
    if (!filter.isEmpty()) {
        filters = filter.split(";;");
    }

    if(filters.isEmpty())filters << tr("所有文件 (*)");

    m_filterCombo->addItems(filters);

    // Apply first filter if provided, otherwise default to All Files or first available
    if (!filters.isEmpty()) {
        QStringList tokens = parseNameFilter(filters.first());
        m_model->setNameFilters(tokens);
        m_model->setNameFilterDisables(false);
    }

    bottomLayout->addWidget(lblName, 0, 0);
    bottomLayout->addWidget(m_nameEdit, 0, 1);
    
    // Row 1: Type Label | Type Combo
    lblType->hide();
    m_filterCombo->setToolTip(tr("文件类型"));
    bottomLayout->addWidget(m_filterCombo, 0, 2);
    bottomLayout->setColumnStretch(1,2);
    bottomLayout->setColumnStretch(2,1);
    m_filterCombo->setMinimumWidth(80);
    m_filterCombo->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Fixed);
    
    m_statusLabel = new QLabel(this);
    m_statusLabel->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Fixed);
    m_statusLabel->setFixedHeight(fontMetrics().height()+4);
    bottomLayout->addWidget(m_statusLabel, 1, 0, 1, 3);

    if (m_mode == OpenDirectory) {
        m_filterCombo->setVisible(false);
        lblType->setVisible(false);
    }

    mainLayout->addWidget(m_bottomBar);

    // --- Connections ---
    
    // Sidebar
    connect(m_sidebar, &QListWidget::itemClicked, this, &FilePickerWidget::onSidebarItemClicked);
    
    // Top Bar
    connect(btnBack, &QToolButton::clicked, [this](){
        if (m_histIndex > 0) {
            m_histIndex--;
            updateView();
        }
    });
    connect(btnForward, &QToolButton::clicked, [this](){
        if (m_histIndex < m_history.size() - 1) {
            m_histIndex++;
            updateView();
        }
    });
    connect(btnUp, &QToolButton::clicked, [this](){
        QDir d = QDir(m_model->rootPath());
        if (d.cdUp()) {
            onPathChanged(d.absolutePath());
        }
    });
    connect(btnHome, &QToolButton::clicked, [this](){
        onPathChanged(QDir::homePath());
    });
    connect(btnRefresh, &QToolButton::clicked, [this](){
        reloadDirectory();
    });
    connect(btnGo, &QToolButton::clicked, [this](){
        QString p = addrEdit->text();
        if (QFileInfo::exists(p) && QFileInfo(p).isDir()) {
            onPathChanged(p);
        }
    });
    connect(addrEdit, &QLineEdit::returnPressed, btnGo, &QToolButton::click);
    
    connect(btnViewMode, &QToolButton::toggled, [this](bool checked){
        if (checked) { // Grid
            m_viewStack->setCurrentWidget(m_iconView);
            m_isGrid = true;
            btnViewMode->setIcon(QIcon(QString("bwicon:buttons/table_cells_large.svg")));
        } else { // List
            m_viewStack->setCurrentWidget(m_treeView);
            m_isGrid = false;
            btnViewMode->setIcon(QIcon(QString("bwicon:buttons/table_list.svg")));
        }
    });

    // Views
    connect(m_treeView, &QTreeView::doubleClicked, this, &FilePickerWidget::onViewDoubleClicked);
    connect(m_treeView, &QTreeView::clicked, this, &FilePickerWidget::onViewClicked);
    connect(m_iconView, &QListView::doubleClicked, this, &FilePickerWidget::onViewDoubleClicked);
    connect(m_iconView, &QListView::clicked, this, &FilePickerWidget::onViewClicked);

    // Filter
    connect(m_filterCombo, &QComboBox::currentTextChanged, this, [this](const QString& text){
        QStringList tokens = parseNameFilter(text);
        m_model->setNameFilters(tokens);
    });

    // --- Init ---
    
    // Set a reasonable minimum size for the widget
    setMinimumSize(320, 160);

    // Populate Sidebar
    auto addPlace = [this](const QString& name, const QString& path, QStyle::StandardPixmap icon){
        QListWidgetItem* item = new QListWidgetItem(m_sidebar);
        item->setText(name);
        item->setData(Qt::UserRole, path);
        item->setIcon(style()->standardIcon(icon));
    };
    
    addPlace(tr("主目录"), QDir::homePath(), QStyle::SP_DirHomeIcon);
    addPlace(tr("桌面"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), QStyle::SP_DesktopIcon);
    addPlace(tr("文档"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QStyle::SP_DirIcon);
    addPlace(tr("下载"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), QStyle::SP_DirIcon);
    addPlace(tr("音乐"), QStandardPaths::writableLocation(QStandardPaths::MusicLocation), QStyle::SP_DirIcon);
    addPlace(tr("图片"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), QStyle::SP_DirIcon);
    addPlace(tr("视频"), QStandardPaths::writableLocation(QStandardPaths::MoviesLocation), QStyle::SP_DirIcon);
    
    // Separator
    QListWidgetItem* separator = new QListWidgetItem(m_sidebar);
    separator->setFlags(Qt::NoItemFlags);
    separator->setSizeHint(QSize(20, 10)); // Spacer
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setEnabled(false);
    m_sidebar->setItemWidget(separator, line);
    
    // Avoid synchronous mountedVolumes(): disconnected/network mounts can block
    // the GUI for seconds. Drives remain reachable through the editable location.
    addPlace(tr("文件系统"),QDir::rootPath(),QStyle::SP_DriveHDIcon);
    connectModelSignals();
    connect(m_nameEdit,&QLineEdit::textEdited,this,[this] { m_selectedPaths.clear(); m_selectedPath.clear(); });
    addrEdit->setAccessibleName(tr("当前位置"));
    m_nameEdit->setAccessibleName(tr("文件名"));
    m_filterCombo->setAccessibleName(tr("文件类型"));
    btnGo->setToolTip(tr("转到输入的位置"));
    for(auto* button:{btnBack,btnForward,btnUp,btnHome,btnRefresh,btnGo,btnViewMode})button->setAccessibleName(button->toolTip());

    // Set Initial Dir
    QString startDir = initialDir;
    if (startDir.isEmpty()) startDir = QDir::homePath();
    QFileInfo initial(startDir);
    if(initial.isFile() || (!initial.exists() && m_mode==SaveFile)) {
        onPathChanged(initial.absolutePath()); m_nameEdit->setText(initial.fileName());
    } else onPathChanged(initial.isDir()?initial.absoluteFilePath():QDir::homePath());
}

void FilePickerWidget::connectModelSignals()
{
    connect(m_model,&QFileSystemModel::directoryLoaded,this,[this](const QString& path) {
        if(path==m_model->rootPath())m_statusLabel->setText(tr("%1 个项目").arg(m_model->rowCount(m_treeView->rootIndex())));
    });
    connect(m_treeView->selectionModel(),&QItemSelectionModel::selectionChanged,this,[this] {
        const auto rows=m_treeView->selectionModel()->selectedRows(0);
        if(rows.isEmpty()) { m_selectedPaths.clear(); m_selectedPath.clear(); if(m_mode!=SaveFile)m_nameEdit->clear(); return; }
        onViewClicked(rows.first());
    });
}

void FilePickerWidget::reloadDirectory()
{
    const QString path=m_model->rootPath();
    auto* oldModel=m_model;
    auto* oldSelection=m_treeView->selectionModel();
    disconnect(oldModel,nullptr,this,nullptr);
    disconnect(oldSelection,nullptr,this,nullptr);
    m_model=new PickerFileModel(this);
    m_model->iconProvider()->setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);
    m_model->setFilter(oldModel->filter());
    m_model->setNameFilters(oldModel->nameFilters());
    m_model->setNameFilterDisables(false);
    m_treeView->setModel(m_model); m_iconView->setModel(m_model);
    auto* unusedSelection=m_iconView->selectionModel();
    m_iconView->setSelectionModel(m_treeView->selectionModel());
    unusedSelection->deleteLater(); oldSelection->deleteLater();
    connectModelSignals(); setRoot(path);
    m_statusLabel->setText(tr("正在刷新…"));
    oldModel->deleteLater();
}

void FilePickerWidget::onPathChanged(const QString& path)
{
    // Normalize path separators to native format (e.g., backslashes on Windows)
    const QFileInfo target(path);
    if(!target.exists()||!target.isDir()) { m_statusLabel->setText(tr("文件夹不存在：%1").arg(path)); return; }
    QString cleanPath = QDir::cleanPath(target.absoluteFilePath());
    
    // Check if we are navigating to the current path; if so, just refresh without adding to history
    if (m_histIndex >= 0 && m_histIndex < m_history.size() && m_history[m_histIndex] == cleanPath) {
        updateView();
        return;
    }

    // If we are in the middle of the history stack and navigate to a new path,
    // truncate the forward history to branch off from here
    if (m_histIndex < m_history.size() - 1) {
        m_history.resize(m_histIndex + 1);
    }
    
    // Add new path to history and update index
    m_history.append(cleanPath);
    m_histIndex++;
    
    // Synchronize the view with the new state
    updateView();
}

void FilePickerWidget::updateView()
{
    if (m_histIndex < 0 || m_histIndex >= m_history.size()) return;
    
    QString path = m_history[m_histIndex];
    // Set the root index for both TreeView and ListView to the current directory
    setRoot(path);
    
    // Update Address Bar and Navigation Buttons state
    addrEdit->setText(path);
    btnBack->setEnabled(m_histIndex > 0);
    btnForward->setEnabled(m_histIndex < m_history.size() - 1);
    
    m_statusLabel->setText(tr("%1 个项目").arg(m_model->rowCount(m_treeView->rootIndex())));
    btnUp->setEnabled(!QDir(path).isRoot());
}

void FilePickerWidget::setRoot(const QString& path)
{
    // Map the file system path to the model index
    m_treeView->selectionModel()->clearSelection();
    m_selectedPath.clear(); m_selectedPaths.clear();
    if(m_mode!=SaveFile)m_nameEdit->clear();
    QModelIndex index = m_model->setRootPath(path);
    if (index.isValid()) {
        // Update both views to show the content of the new root directory
        m_treeView->setRootIndex(index);
        m_iconView->setRootIndex(index);
        // Ensure the model populates this directory
        m_model->setRootPath(path);
    }
}

void FilePickerWidget::onSidebarItemClicked(QListWidgetItem* item)
{
    if (!item) return;
    // Sidebar items store the target path in UserRole
    QString path = item->data(Qt::UserRole).toString();
    if (!path.isEmpty()) {
        onPathChanged(path);
    }
}

void FilePickerWidget::onViewDoubleClicked(const QModelIndex& index)
{
    if (m_model->isDir(index)) {
        QString path = m_model->filePath(index);
        onPathChanged(path);
    } else if(m_mode!=OpenDirectory) {
        // Activate file
        onViewClicked(index);
        m_selectedPath = m_model->filePath(index);
        emit selectionChanged(m_selectedPath);
        emit fileActivated(m_selectedPath);
    }
}

void FilePickerWidget::onViewClicked(const QModelIndex& index)
{
    QString path = m_model->filePath(index);
    if (!m_model->isDir(index)) {
        m_nameEdit->setText(m_model->fileName(index));
        m_selectedPath = path;
        
        // Handle multi-select
        m_selectedPaths.clear();
        QModelIndexList list = m_isGrid ? m_iconView->selectionModel()->selectedIndexes() 
                                        : m_treeView->selectionModel()->selectedIndexes();
        // Filter out columns other than 0 for tree view
        QSet<QString> uniquePaths;
        for (const auto& idx : list) {
             if(idx.column()==0 && !m_model->isDir(idx))uniquePaths.insert(m_model->filePath(idx));
        }
        m_selectedPaths = uniquePaths.values();
        
        emit selectionChanged(path);
    } else {
        m_selectedPaths.clear(); m_selectedPath.clear();
        if(m_mode!=SaveFile)m_nameEdit->clear();
        // Directory selected logic if needed
        if (m_mode == OpenDirectory) {
            m_nameEdit->setText(m_model->fileName(index));
            m_selectedPath = path;
            emit selectionChanged(path);
        }
    }
}

void FilePickerWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    const bool compact=width()<600;
    m_sidebar->setVisible(!compact);
    m_splitter->setSizes({compact?0:136,qMax(1,width()-(compact?0:137))});
    btnHome->setVisible(!compact); btnForward->setVisible(!compact);
    btnViewMode->setVisible(width()>=450);
    m_treeView->setColumnHidden(2,width()<700);
    m_treeView->setColumnHidden(3,width()<500);
}

bool FilePickerWidget::validateSelection()
{
    const QStringList paths=m_mode==OpenFiles?selectedPaths():QStringList{selectedPath()};
    bool valid=!paths.isEmpty();
    for(const auto& path:paths) {
        const QFileInfo info(path);
        valid=valid && !path.trimmed().isEmpty();
        if(m_mode==OpenDirectory)valid=valid && info.isDir();
        else if(m_mode==SaveFile)valid=valid && !info.isDir() && QFileInfo(info.absolutePath()).isDir();
        else valid=valid && info.isFile();
    }
    if(!valid) { m_statusLabel->setText(tr("请选择有效的%1").arg(m_mode==OpenDirectory?tr("文件夹"):tr("文件"))); m_nameEdit->setFocus(); }
    return valid;
}

void FilePickerWidget::toggleViewMode()
{
    btnViewMode->click();
}


// Deprecated wrapper implementation
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
    // This is a legacy helper. We should construct the new widget.
    // However, the caller might expect specific pointers (outView, outModel) which we might not expose directly anymore
    // or they might be different widgets (QListView vs QTreeView).
    
    FilePickerWidget::Mode mode = FilePickerWidget::OpenFile;
    if (directoryOnly) mode = FilePickerWidget::OpenDirectory;
    else if (saveMode) mode = FilePickerWidget::SaveFile;
    else if (multiSelect) mode = FilePickerWidget::OpenFiles;
    
    auto* w = new FilePickerWidget(parent, initialDir, filter, mode);
    
    // Map internal widgets to output pointers if requested (Best Effort)
    if (outView) *outView = w->findChild<QTreeView*>();
    if (outModel) *outModel = w->findChild<QFileSystemModel*>();
    if (outEdit) *outEdit = w->findChild<QLineEdit*>();
    
    // Hook up the output path to the pointer if provided (needs a signal connection, but this function just returns the widget)
    // The caller is responsible for connecting signals or checking state after execution.
    // Since this is usually used in a DialogUtils context, we rely on DialogUtils to call selectedPath() on the widget.
    
    return w;
}

} // namespace basewidgets
