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
#include <QStorageInfo>
#include <QStandardPaths>
#include <QDate>
#include <QDateTime>
#include <QHeaderView>
#include <QDebug>
#include <QStyle>
#include <QAction>
#include <QMenu>

namespace basewidgets {

static QStringList parseNameFilter(const QString& filter)
{
    // Format: "Images (*.png *.jpg)" or "*.txt"
    QString f = filter;
    int l = f.indexOf('(');
    int r = f.indexOf(')');
    if (l >= 0 && r > l) f = f.mid(l + 1, r - l - 1);
    f = f.replace(';', ' ');
    QStringList tokens = f.split(' ', QString::SkipEmptyParts);
    for (QString& t : tokens) t = t.trimmed();
    return tokens;
}

FilePickerWidget::FilePickerWidget(QWidget* parent,
                                   const QString& initialDir,
                                   const QString& filter,
                                   Mode mode)
    : QWidget(parent), m_mode(mode)
{
    // Initialize the UI components for the Ubuntu-style file picker
    setupUi(initialDir, filter);
}

QString FilePickerWidget::selectedPath() const
{
    // Get the path from the name edit field, handling absolute and relative paths
    QString name = m_nameEdit->text();
    if (m_mode == SaveFile) {
        name = name.trimmed();
    }
    if (name.isEmpty()) return m_selectedPath;
    
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
    m_topBar->setFixedHeight(50);

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

    // Address Bar
    addrEdit = new QLineEdit(this);
    addrEdit->setPlaceholderText(tr("Enter location..."));
    addrEdit->setFixedHeight(30);

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
    m_sidebar->setFixedWidth(150);
    m_sidebar->setIconSize(QSize(20, 20));

    // Views
    m_viewStack = new QStackedWidget(m_splitter);
    
    m_model = new QFileSystemModel(this);
    m_model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs);
    m_model->setRootPath(QDir::rootPath());

    // Tree View (List Mode)
    m_treeView = new QTreeView(m_viewStack);
    m_treeView->setModel(m_model);
    m_treeView->setFrameShape(QFrame::NoFrame);
    m_treeView->setSelectionMode(m_mode == OpenFiles ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
    m_treeView->setSortingEnabled(true);
    m_treeView->setColumnWidth(0, 250); // Name
    m_treeView->setColumnWidth(1, 80);  // Size
    m_treeView->setColumnWidth(2, 80);  // Type
    m_treeView->header()->setStretchLastSection(true);
    
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
    // Ensure item text is centered and has padding

    m_viewStack->addWidget(m_treeView);
    m_viewStack->addWidget(m_iconView);

    m_splitter->addWidget(m_sidebar);
    m_splitter->addWidget(m_viewStack);
    //m_splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(m_splitter, 1);

    // --- 3. Bottom Bar (Inputs) ---
    // Only needed for picking files, but useful to see current selection
    m_bottomBar = new QFrame(this);
    m_bottomBar->setObjectName("FilePickerBottomBar");
    // Allow natural height expansion
    m_bottomBar->setMinimumHeight(80);
    
    // Use Grid Layout for better alignment and height control
    auto* bottomLayout = new QGridLayout(m_bottomBar);
    bottomLayout->setContentsMargins(15, 15, 15, 15);
    bottomLayout->setVerticalSpacing(12);
    bottomLayout->setHorizontalSpacing(10);
    
    QLabel* lblName = new QLabel(tr("文件名:"), this);
    
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setFixedHeight(34);

    QLabel* lblType = new QLabel(tr("文件类型:"), this);

    m_filterCombo = new QComboBox(this);
    m_filterCombo->setFixedHeight(34);
    // Remove fixed width constraints to let it expand in grid
    m_filterCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Parse filter
    QStringList filters;
    if (!filter.isEmpty()) {
        filters = filter.split(";;");
    }

    // Add common default filters if not present (helps user discover types)
    auto addFilterIfMissing = [&](const QString& name, const QString& ext) {
        bool found = false;
        for (const auto& f : filters) {
            if (f.contains(ext, Qt::CaseInsensitive) || f.contains(name, Qt::CaseInsensitive)) {
                found = true; 
                break;
            }
        }
        if (!found) filters << QString("%1 (%2)").arg(name, ext);
    };

    // Always add these common types for better UX
    addFilterIfMissing(tr("Images"), "*.png *.jpg *.jpeg *.bmp *.gif *.svg *.webp");
    addFilterIfMissing(tr("Documents"), "*.txt *.pdf *.doc *.docx *.md *.json *.xml");
    addFilterIfMissing(tr("Code"), "*.cpp *.h *.c *.hpp *.py *.js *.ts *.html *.css");
    
    // Ensure "All Files" is at the end
    bool hasAll = false;
    for (const auto& f : filters) {
        if (f.contains("(*)") || f.contains("*.*")) hasAll = true;
    }
    if (!hasAll) filters << tr("All Files (*)");

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
    bottomLayout->addWidget(lblType, 1, 0);
    bottomLayout->addWidget(m_filterCombo, 1, 1);
    
    m_statusLabel = new QLabel(this);
    bottomLayout->addWidget(m_statusLabel, 2, 0, 1, 2, Qt::AlignLeft);

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
        // Refresh model
        m_model->setRootPath(m_model->rootPath()); // Trigger re-scan
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
    connect(m_filterCombo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), [this](const QString& text){
        QStringList tokens = parseNameFilter(text);
        m_model->setNameFilters(tokens);
    });

    // --- Init ---
    
    // Set a reasonable minimum size for the widget
    setMinimumSize(850, 550);

    // Populate Sidebar
    auto addPlace = [this](const QString& name, const QString& path, QStyle::StandardPixmap icon){
        QListWidgetItem* item = new QListWidgetItem(m_sidebar);
        item->setText(name);
        item->setData(Qt::UserRole, path);
        item->setIcon(style()->standardIcon(icon));
    };
    
    addPlace(tr("Home"), QDir::homePath(), QStyle::SP_DirHomeIcon);
    addPlace(tr("Desktop"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), QStyle::SP_DesktopIcon);
    addPlace(tr("Documents"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QStyle::SP_DirIcon);
    addPlace(tr("Downloads"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), QStyle::SP_DirIcon);
    addPlace(tr("Music"), QStandardPaths::writableLocation(QStandardPaths::MusicLocation), QStyle::SP_DirIcon);
    addPlace(tr("Pictures"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), QStyle::SP_DirIcon);
    addPlace(tr("Videos"), QStandardPaths::writableLocation(QStandardPaths::MoviesLocation), QStyle::SP_DirIcon);
    
    // Separator
    QListWidgetItem* separator = new QListWidgetItem(m_sidebar);
    separator->setFlags(Qt::NoItemFlags);
    separator->setSizeHint(QSize(20, 10)); // Spacer
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setEnabled(false);
    m_sidebar->setItemWidget(separator, line);
    
    // Drives
    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady()) {
            QListWidgetItem* item = new QListWidgetItem(m_sidebar);
            QString label = storage.name();
            if (label.isEmpty()) label = storage.rootPath();
            item->setText(label);
            item->setData(Qt::UserRole, storage.rootPath());
            item->setIcon(style()->standardIcon(QStyle::SP_DriveHDIcon));
        }
    }

    // Set Initial Dir
    QString startDir = initialDir;
    if (startDir.isEmpty()) startDir = QDir::homePath();
    onPathChanged(startDir);
}

void FilePickerWidget::onPathChanged(const QString& path)
{
    // Normalize path separators to native format (e.g., backslashes on Windows)
    QString cleanPath = QDir::toNativeSeparators(path);
    
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
    
    // Update Status Bar with item count
    QDir d(path);
    int count = d.entryList(QDir::AllEntries | QDir::NoDotAndDotDot).count();
    if (m_statusLabel) m_statusLabel->setText(tr("%1 个项目").arg(count));
}

void FilePickerWidget::setRoot(const QString& path)
{
    // Map the file system path to the model index
    QModelIndex index = m_model->index(path);
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
    } else {
        // Activate file
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
             uniquePaths.insert(m_model->filePath(idx));
        }
        m_selectedPaths = uniquePaths.values();
        
        emit selectionChanged(path);
    } else {
        // Directory selected logic if needed
        if (m_mode == OpenDirectory) {
            m_nameEdit->setText(m_model->fileName(index));
            m_selectedPath = path;
            emit selectionChanged(path);
        }
    }
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
