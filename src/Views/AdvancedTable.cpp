/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Views/AdvancedTable.h>

#include <QScrollBar>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/LineEdit.h>
#include <basewidgets/Inputs/ComboBox.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Inputs/SpinBox.h>
#include <basewidgets/Inputs/ToolButton.h>
#include <basewidgets/Views/TableView.h>
#include <basewidgets/Dialogs/DialogUtils.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QTextStream>
#include <QClipboard>
#include <QApplication>
#include <QTimer>
#include <QSignalBlocker>
#include <QSet>
#include <algorithm>
#include <functional>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QRegExp>
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace basewidgets {

static QString escapeCsvField(const QString& v, const QString& delim)
{
    const bool needsQuote = v.contains('"') || v.contains('\n') || v.contains('\r') || (!delim.isEmpty() && v.contains(delim));
    if (!needsQuote) return v;
    QString out = v;
    out.replace('"', "\"\"");
    return QString("\"%1\"").arg(out);
}

AdvancedTable::AdvancedTable(QWidget* parent)
    : StyledWidget(parent)
{
    initUi();
    bindSignals();
}

AdvancedTable::AdvancedTable(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : StyledWidget(manager, parent)
{
    initUi();
    bindSignals();
}

AdvancedTable::~AdvancedTable() = default;

void AdvancedTable::initUi()
{
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 8, 8, 8);
    outer->setSpacing(10);

    // 顶部筛选、列选择、每页条数、跳页与列可见性
    auto* topRow = new QHBoxLayout();
    m_filterLabel = new Label("过滤：", this);
    m_filterEdit = new LineEdit(this);
    m_filterEdit->setPlaceholderText("输入关键字，支持任意列匹配");
    m_columnLabel = new Label("列：", this);
    m_columnCombo = new ComboBox(this);
    m_pageSizeLabel = new Label("每页：", this);
    m_pageSizeCombo = new ComboBox(this);
    m_pageSizeCombo->addItems({"10", "20", "50", "100"});
    m_pageSizeCombo->setCurrentText("20");
    m_jumpLabel = new Label("跳页：", this);
    m_jumpSpin = new SpinBox(this);
    m_jumpSpin->setMinimum(1);
    m_jumpSpin->setMaximum(1);
    m_jumpSpin->setValue(1);
    m_columnsBtn = new ToolButton(this);
    m_columnsBtn->setText("列可见性");
    m_columnsMenu = new QMenu(m_columnsBtn);
    m_columnsBtn->setMenu(m_columnsMenu);
    m_columnsBtn->setPopupMode(QToolButton::InstantPopup);
    topRow->addWidget(m_filterLabel);
    topRow->addWidget(m_filterEdit, 1);
    topRow->addSpacing(10);
    topRow->addWidget(m_columnLabel);
    topRow->addWidget(m_columnCombo);
    topRow->addSpacing(10);
    topRow->addWidget(m_pageSizeLabel);
    topRow->addWidget(m_pageSizeCombo);
    topRow->addSpacing(10);
    topRow->addWidget(m_jumpLabel);
    topRow->addWidget(m_jumpSpin);
    topRow->addSpacing(10);
    topRow->addWidget(m_columnsBtn);
    topRow->addStretch();

    // 模型链
    m_sortFilter = new QSortFilterProxyModel(this);
    m_sortFilter->setFilterCaseSensitivity(m_filterCs);
    m_sortFilter->setFilterKeyColumn(-1);
    m_sortFilter->setDynamicSortFilter(true);

    m_paged = new PagedProxyModel(this);
    m_paged->setSourceModel(m_sortFilter);
    m_paged->setPageSize(20);

    // 表格视图
    m_tableView = new TableView(this);
    m_tableView->setModel(m_paged);
    m_tableView->setSortingEnabled(true);
    m_tableView->sortByColumn(0, Qt::AscendingOrder);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionsMovable(true);
    m_tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 底部分页工具栏
    auto* bottomRow = new QVBoxLayout();
    auto* summaryRow = new QHBoxLayout();
    auto* navigationRow = new QHBoxLayout();
    m_prevBtn = new PushButton("上一页", this);
    m_nextBtn = new PushButton("下一页", this);
    m_exportBtn = new PushButton("导出CSV", this);
    m_prevBtn->setObjectName("PrevPageBtn");
    m_nextBtn->setObjectName("NextPageBtn");
    m_exportBtn->setObjectName("ExportCsvBtn");
    m_nextBtn->setProperty("bwPrimary", true);
    m_nextBtn->setProperty("primary", true);
    m_pageSizeBottom = new ComboBox(this);
    m_pageSizeBottom->addItems({"10", "20", "50", "100"});
    m_pageSizeBottom->setCurrentText("20");
    QObject::connect(m_pageSizeBottom, static_cast<void(ComboBox::*)(const QString&)>(&ComboBox::currentTextChanged), [this](const QString& s){
        m_paged->setPageSize(s.toInt());
        if (m_pageSizeCombo) m_pageSizeCombo->setCurrentText(s);
    });

    m_totalLabel = new Label(this);
    m_totalLabel->setText("共0条数据");
    m_totalLabel->setWordWrap(true);

    m_firstBtn = new PushButton("首页", this);
    m_lastBtn = new PushButton("尾页", this);
    m_firstBtn->setObjectName("FirstPageBtn");
    m_lastBtn->setObjectName("LastPageBtn");
    for (auto* nav : {m_firstBtn, m_lastBtn}) {
        nav->setProperty("page", true);
    }

    m_pageNumbers = new QWidget(this);
    auto* numsLayout = new QHBoxLayout(m_pageNumbers);
    numsLayout->setContentsMargins(0,0,0,0);
    numsLayout->setSpacing(6);
    numsLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_pageScroll = new QScrollArea(this);
    m_pageScroll->setObjectName("PageScroll");
    m_pageScroll->setWidgetResizable(true);
    m_pageScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pageScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pageScroll->setFrameShape(QFrame::NoFrame);
    m_pageScroll->setWidget(m_pageNumbers);
    m_pageScroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_pageScroll->setFixedHeight(m_prevBtn->sizeHint().height()+4);
    if (m_pageScroll->viewport()) m_pageScroll->viewport()->setAutoFillBackground(false);

    m_pageJumpBottom = new SpinBox(this);
    m_pageJumpBottom->setMinimum(1);
    QObject::connect(m_pageJumpBottom, static_cast<void(SpinBox::*)(int)>(&SpinBox::valueChanged), [this](int val){
        m_paged->setCurrentPage(qMax(0, val - 1));
    });

    bottomRow->setContentsMargins(6, 6, 6, 6);
    bottomRow->setSpacing(8);
    m_pageSizeBottomLabel = new Label("每页", this);
    m_pageJumpBottomLabel = new Label("跳转", this);
    summaryRow->addWidget(m_pageSizeBottomLabel);
    summaryRow->addWidget(m_pageSizeBottom);
    summaryRow->addWidget(m_totalLabel,1);
    summaryRow->addWidget(m_pageJumpBottomLabel);
    summaryRow->addWidget(m_pageJumpBottom);
    summaryRow->addWidget(m_exportBtn);
    navigationRow->addWidget(m_firstBtn);
    navigationRow->addWidget(m_prevBtn);
    navigationRow->addWidget(m_pageScroll,1);
    navigationRow->addWidget(m_nextBtn);
    navigationRow->addWidget(m_lastBtn);
    bottomRow->addLayout(summaryRow);
    bottomRow->addLayout(navigationRow);

    m_topBar = new QWidget(this);
    m_topBar->setLayout(topRow);
    // 移除顶部重复的分页功能（每页与跳页），保留下部
    if (m_pageSizeLabel) m_pageSizeLabel->setVisible(false);
    if (m_pageSizeCombo) m_pageSizeCombo->setVisible(false);
    if (m_jumpLabel) m_jumpLabel->setVisible(false);
    if (m_jumpSpin) m_jumpSpin->setVisible(false);
    outer->addWidget(m_topBar);
    outer->addWidget(m_tableView,1);
    m_bottomBar = new QWidget(this);
    m_bottomBar->setProperty("pagination", true);
    m_bottomBar->setLayout(bottomRow);
    m_bottomBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    outer->addWidget(m_bottomBar);
}

void AdvancedTable::bindSignals()
{
    // 过滤输入防抖
    m_filterTimer = new QTimer(this);
    m_filterTimer->setSingleShot(true);
    m_filterTimer->setInterval(250);
    QObject::connect(m_filterEdit, &LineEdit::textChanged, [this](const QString&){
        m_filterTimer->start();
    });
    QObject::connect(m_filterTimer, &QTimer::timeout, [this](){
        applyFilter();
    });

    QObject::connect(m_pageSizeCombo, static_cast<void(ComboBox::*)(const QString&)>(&ComboBox::currentTextChanged), [this](const QString& s){
        m_paged->setPageSize(s.toInt());
        if (m_pageSizeBottom) m_pageSizeBottom->setCurrentText(s);
    });
    QObject::connect(m_columnCombo, static_cast<void(ComboBox::*)(int)>(&ComboBox::currentIndexChanged), [this](int idx){
        m_sortFilter->setFilterKeyColumn(idx - 1);
    });
    QObject::connect(m_firstBtn, &PushButton::clicked, [this](){
        if (m_paged) m_paged->setCurrentPage(0);
    });
    QObject::connect(m_lastBtn, &PushButton::clicked, [this](){
        if (!m_paged) return;
        m_paged->setCurrentPage(qMax(0, m_paged->pageCount() - 1));
    });
    QObject::connect(m_prevBtn, &PushButton::clicked, [this](){
        m_paged->setCurrentPage(m_paged->currentPage() - 1);
    });
    QObject::connect(m_nextBtn, &PushButton::clicked, [this](){
        m_paged->setCurrentPage(m_paged->currentPage() + 1);
    });
    QObject::connect(m_jumpSpin, static_cast<void(SpinBox::*)(int)>(&SpinBox::valueChanged), [this](int val){
        m_paged->setCurrentPage(val - 1);
    });
    QObject::connect(m_paged, &PagedProxyModel::pageChanged, [this](){
        emit pageChanged(m_paged->currentPage(), m_paged->pageCount());
        if (m_totalLabel) {
            m_totalLabel->setText(QString("共%5条数据 | 当前页 %1 / %2 | 当前行 %3~%4")
                                  .arg(m_paged->currentPage() + 1)
                                  .arg(qMax(1,m_paged->pageCount()))
                                  .arg(m_paged->totalRows() ? m_paged->currentPage() * m_paged->pageSize() + 1 : 0)
                                  .arg(qMin((m_paged->currentPage() + 1) * m_paged->pageSize(), m_paged->totalRows()))
                                  .arg(m_paged->totalRows()));
        }
        const QSignalBlocker jumpBlock(m_jumpSpin);
        const QSignalBlocker bottomBlock(m_pageJumpBottom);
        m_jumpSpin->setMaximum(qMax(1, m_paged->pageCount()));
        m_jumpSpin->setValue(m_paged->currentPage() + 1);
        m_pageJumpBottom->setMaximum(qMax(1,m_paged->pageCount()));
        m_pageJumpBottom->setValue(m_paged->currentPage()+1);
        m_prevBtn->setEnabled(m_paged->currentPage() > 0);
        m_nextBtn->setEnabled(m_paged->currentPage() + 1 < m_paged->pageCount());
        if (m_pageJumpBottom) m_pageJumpBottom->setMaximum(qMax(1, m_paged->pageCount()));
        rebuildPaginationUi();
    });

    // 右键菜单：复制选中行为 CSV
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(m_tableView, &TableView::customContextMenuRequested, [this](const QPoint& pos){
        QMenu menu(m_tableView);
        QAction* copyRow = menu.addAction("复制选中行为CSV");
        emit prepareContextMenu(&menu, m_tableView->selectionModel()->selectedRows());
        QAction* chosen = menu.exec(m_tableView->viewport()->mapToGlobal(pos));
        if (chosen == copyRow && m_source) {
            QString out;
            QTextStream ts(&out);
            QSet<int> selectedRows;
            for (const QModelIndex& index : m_tableView->selectionModel()->selectedIndexes()) selectedRows.insert(index.row());
            QList<int> rows = selectedRows.values();
            std::sort(rows.begin(),rows.end());
            for (int r : rows) {
                for (int c = 0; c < m_source->columnCount(); ++c) {
                    const QModelIndex filtered = m_paged->mapToSource(m_paged->index(r,c));
                    const QModelIndex source = m_sortFilter->mapToSource(filtered);
                    ts << escapeCsvField(m_source->data(source).toString(),m_exportDelimiter);
                    if (c + 1 < m_source->columnCount()) ts << m_exportDelimiter;
                }
                ts << "\n";
            }
            QApplication::clipboard()->setText(out);
        }
    });

    // 导出CSV（当前过滤结果）
    QObject::connect(m_exportBtn, &PushButton::clicked, [this](){
        const QString fileName = DialogUtils::saveFile(this, tr("导出为CSV"), QString(), "CSV (*.csv)");
        if (fileName.isEmpty()) return;
        const bool ok = exportCsvToFile(fileName, true);
        if (ok) emit exportCompleted(fileName, m_sortFilter ? m_sortFilter->rowCount() : 0);
        else emit exportFailed(fileName, tr("写入失败或文件不可用"));
    });

    rebuildPaginationUi();
}

void AdvancedTable::setSourceModel(QAbstractItemModel* source)
{
    m_source = source;
    m_sortFilter->setSourceModel(source);
    rebuildColumnUi();
}

QAbstractItemModel* AdvancedTable::sourceModel() const
{
    return m_source;
}

void AdvancedTable::rebuildColumnUi()
{
    if (!m_source) return;
    // 列可见性菜单
    qDeleteAll(m_columnsMenu->actions());
    m_columnsMenu->clear();
    const QStringList headers = currentHeaderLabels();
    for (int c = 0; c < m_source->columnCount(); ++c) {
        QAction* act = new QAction(headers[c], m_columnsMenu);
        act->setCheckable(true);
        act->setChecked(!m_tableView->isColumnHidden(c));
        QObject::connect(act, &QAction::toggled, [this, c](bool checked){
            m_tableView->setColumnHidden(c, !checked);
        });
        m_columnsMenu->addAction(act);
    }
    // 列选择下拉
    m_columnCombo->clear();
    m_columnCombo->addItem("全部列");
    for (const QString& h : headers) m_columnCombo->addItem(h);
    m_columnCombo->setCurrentIndex(0);
}

void AdvancedTable::rebuildPaginationUi()
{
    if (!m_pageNumbers) return;
    auto* layout = qobject_cast<QHBoxLayout*>(m_pageNumbers->layout());
    if (!layout) return;
    m_pageButtons.clear();
    for (auto* b : m_pageButtonPool) { if (b) b->setVisible(false); }
    for (auto* l : m_ellipsisPool) { if (l) l->setVisible(false); }

    while (layout->count() > 0) {
        auto* item = layout->takeAt(0);
        if (!item) continue;
        QWidget* w = item->widget();
        delete item;
        if (!w) continue;

        if (auto* btn = qobject_cast<PushButton*>(w)) {
            btn->setVisible(false);
            m_pageButtonPool.append(btn);
            continue;
        }
        if (auto* lbl = qobject_cast<Label*>(w)) {
            lbl->setVisible(false);
            m_ellipsisPool.append(lbl);
            continue;
        }
        w->deleteLater();
    }

    int pc = m_paged ? m_paged->pageCount() : 0;
    pc = qMax(1, pc);
    int cur = m_paged ? m_paged->currentPage() + 1 : 1;
    cur = qBound(1, cur, pc);

    if (!m_paginationCfg.windowSize) m_paginationCfg.windowSize = 7;
    if (!m_paginationCfg.sideCount) m_paginationCfg.sideCount = 2;
    const int window = qMax(3, m_paginationCfg.windowSize);
    const int side = qMax(1, m_paginationCfg.sideCount);

    m_firstBtn->setEnabled(cur > 1);
    m_lastBtn->setEnabled(cur < pc);

    auto takePageBtn = [this]() -> PushButton* {
        PushButton* btn = nullptr;
        while (!m_pageButtonPool.isEmpty() && !btn) {
            btn = m_pageButtonPool.takeLast();
            if (!btn) continue;
            if (btn->parentWidget() != m_pageNumbers) btn->setParent(m_pageNumbers);
        }
        if (!btn) {
            btn = new PushButton(QString(), m_pageNumbers);
            btn->setProperty("page", true);
            QObject::connect(btn, &PushButton::clicked, [this, btn](){
                if (!m_paged) return;
                bool ok = false;
                int n = btn->property("pageNumber").toInt(&ok);
                if (ok) m_paged->setCurrentPage(n - 1);
            });
        }
        return btn;
    };

    auto takeEllipsis = [this]() -> Label* {
        Label* lbl = nullptr;
        while (!m_ellipsisPool.isEmpty() && !lbl) {
            lbl = m_ellipsisPool.takeLast();
            if (!lbl) continue;
            if (lbl->parentWidget() != m_pageNumbers) lbl->setParent(m_pageNumbers);
        }
        if (!lbl) {
            lbl = new Label("...", m_pageNumbers);
        }
        lbl->setText("...");
        return lbl;
    };

    auto addBtn = [this, layout, &takePageBtn](int n, bool active){
        auto* btn = takePageBtn();
        btn->setText(QString::number(n));
        btn->setFixedWidth(btn->fontMetrics().horizontalAdvance(btn->text())+24);
        btn->setProperty("pageNumber", n);
        btn->setProperty("active", active);
        btn->setVisible(true);
        layout->addWidget(btn);
        m_pageButtons.append(btn);
    };

    auto addEllipsis = [layout, &takeEllipsis](){
        auto* ellipsis = takeEllipsis();
        ellipsis->setVisible(true);
        layout->addWidget(ellipsis);
    };

    if (pc <= window) {
        for (int i = 1; i <= pc; ++i) addBtn(i, i == cur);
    } else {
        addBtn(1, cur == 1);
        if (cur > side + 2) addEllipsis();
        int start = qMax(2, cur - side);
        int end = qMin(pc - 1, cur + side);
        for (int i = start; i <= end; ++i) addBtn(i, i == cur);
        if (cur < pc - (side + 1)) addEllipsis();
        addBtn(pc, cur == pc);
    }

    // 将当前页居中到滚动区域
    if (m_pageScroll) {
        // 查找active按钮
        PushButton* activeBtn = nullptr;
        for (auto* b : m_pageButtons) { if (b->property("active").toBool()) { activeBtn = b; break; } }
        if (activeBtn) {
            auto* hsb = m_pageScroll->horizontalScrollBar();
            int target = activeBtn->x() + activeBtn->width()/2 - m_pageScroll->viewport()->width()/2;
            hsb->setValue(qBound(0, target, hsb->maximum()));
        }
    }
}

void AdvancedTable::applyFilter()
{
    if (!m_sortFilter) return;

    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (m_filterCs == Qt::CaseInsensitive) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }

    if (m_filterMode == FilterMode::RegularExpression) {
        m_filterRe = QRegularExpression(m_filterEdit ? m_filterEdit->text() : QString(), options);
    } else {
        const QString text = m_filterEdit ? m_filterEdit->text() : QString();
        m_filterRe = QRegularExpression(QRegularExpression::escape(text), options);
    }

    m_sortFilter->setFilterRegularExpression(m_filterRe);
    emit filterChanged(m_filterEdit ? m_filterEdit->text() : QString(), m_sortFilter->filterKeyColumn());
}

QStringList AdvancedTable::currentHeaderLabels() const
{
    QStringList headers;
    if (!m_source) return headers;
    for (int c = 0; c < m_source->columnCount(); ++c) {
        QString h = m_source->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString();
        if (h.isEmpty()) h = QString("列%1").arg(c + 1);
        headers << h;
    }
    return headers;
}

void AdvancedTable::setPageSize(int size)
{
    if (m_paged) m_paged->setPageSize(size);
    const QString text = QString::number(size);
    if (m_pageSizeCombo) m_pageSizeCombo->setCurrentText(text);
    if (m_pageSizeBottom) m_pageSizeBottom->setCurrentText(text);
}

int AdvancedTable::pageSize() const
{
    return m_paged ? m_paged->pageSize() : 0;
}

void AdvancedTable::setPageSizeOptions(const QStringList& sizes)
{
    if (m_pageSizeCombo) {
        m_pageSizeCombo->clear();
        m_pageSizeCombo->addItems(sizes);
    }
    if (m_pageSizeBottom) {
        m_pageSizeBottom->clear();
        m_pageSizeBottom->addItems(sizes);
    }
    if (m_paged) setPageSize(m_paged->pageSize());
}

void AdvancedTable::setFilterText(const QString& text)
{
    if (m_filterTimer) m_filterTimer->stop();
    if (m_filterEdit) m_filterEdit->setText(text);
    applyFilter();
}

void AdvancedTable::setFilterColumn(int column)
{
    m_sortFilter->setFilterKeyColumn(column);
    // 同步下拉
    int comboIndex = column + 1; // -1 => 0
    if (comboIndex >= 0 && comboIndex < m_columnCombo->count()) {
        m_columnCombo->setCurrentIndex(comboIndex);
    }
}

bool AdvancedTable::exportCsvToFile(const QString& filePath, bool includeHeader) const
{
    CsvExportOptions opt;
    opt.filePath = filePath;
    opt.includeHeader = includeHeader;
    opt.delimiter = m_exportDelimiter;
    return exportCsv(opt);
}

void AdvancedTable::applyStylesheet()
{
    // AdvancedTable 自身不强制覆盖样式；保持与 StyledWidget 一致
}

QHeaderView* AdvancedTable::header() const
{
    return m_tableView ? m_tableView->horizontalHeader() : nullptr;
}

void AdvancedTable::setFilterCaseSensitivity(Qt::CaseSensitivity cs)
{
    m_filterCs = cs;
    applyFilter();
}

Qt::CaseSensitivity AdvancedTable::filterCaseSensitivity() const
{
    return m_filterCs;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void AdvancedTable::setFilterRegExp(const QRegExp& exp)
{
    m_filterMode = FilterMode::RegularExpression;
    m_filterCs = exp.caseSensitivity();
    if (m_filterTimer) m_filterTimer->stop();
    if (m_filterEdit) m_filterEdit->setText(exp.pattern());
    if (m_sortFilter) m_sortFilter->setFilterRegExp(exp);
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (m_filterCs == Qt::CaseInsensitive) options |= QRegularExpression::CaseInsensitiveOption;
    m_filterRe = QRegularExpression(exp.pattern(), options);
    emit filterChanged(m_filterEdit ? m_filterEdit->text() : QString(), m_sortFilter->filterKeyColumn());
}
#endif

void AdvancedTable::setFilterMode(FilterMode mode)
{
    if (m_filterMode == mode) return;
    m_filterMode = mode;
    applyFilter();
}

AdvancedTable::FilterMode AdvancedTable::filterMode() const
{
    return m_filterMode;
}

void AdvancedTable::setFilterRegularExpression(const QRegularExpression& re)
{
    m_filterMode = FilterMode::RegularExpression;
    m_filterRe = re;
    if (m_filterTimer) m_filterTimer->stop();
    if (m_filterEdit) m_filterEdit->setText(re.pattern());
    if (m_sortFilter) m_sortFilter->setFilterRegularExpression(m_filterRe);
    emit filterChanged(m_filterEdit ? m_filterEdit->text() : QString(), m_sortFilter->filterKeyColumn());
}

QRegularExpression AdvancedTable::filterRegularExpression() const
{
    return m_filterRe;
}

void AdvancedTable::setPaginationConfig(const PaginationConfig& cfg)
{
    m_paginationCfg = cfg;
    rebuildPaginationUi();
}

AdvancedTable::PaginationConfig AdvancedTable::paginationConfig() const
{
    return m_paginationCfg;
}

bool AdvancedTable::exportCsv(const CsvExportOptions& options, QString* error) const
{
    if (options.filePath.isEmpty()) {
        if (error) *error = tr("文件路径为空");
        return false;
    }

    if (!m_sortFilter || !m_paged) {
        if (error) *error = tr("模型未初始化");
        return false;
    }

    QFile f(options.filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) *error = tr("文件不可写");
        return false;
    }

    if (options.includeUtf8Bom) {
        f.write("\xEF\xBB\xBF", 3);
    }

    QTextStream ts(&f);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    ts.setCodec("UTF-8");
#else
    ts.setEncoding(QStringConverter::Utf8);
#endif

    const QString delimiter = options.delimiter.isEmpty() ? m_exportDelimiter : options.delimiter;

    QList<int> cols = options.columns;
    const int colCount = m_sortFilter->columnCount();
    if (cols.isEmpty()) {
        cols.reserve(colCount);
        for (int c = 0; c < colCount; ++c) cols.append(c);
    } else {
        cols.erase(std::remove_if(cols.begin(), cols.end(), [colCount](int c){ return c < 0 || c >= colCount; }), cols.end());
    }

    if (options.includeHeader) {
        const QStringList headers = currentHeaderLabels();
        for (int i = 0; i < cols.size(); ++i) {
            const int c = cols[i];
            const QString h = (c >= 0 && c < headers.size()) ? headers[c] : QString();
            ts << escapeCsvField(h, delimiter);
            if (i + 1 < cols.size()) ts << delimiter;
        }
        ts << "\n";
    }

    auto writeRow = [&](int r, const std::function<QVariant(int,int)>& cell){
        for (int i = 0; i < cols.size(); ++i) {
            const int c = cols[i];
            const QString v = cell(r, c).toString();
            ts << escapeCsvField(v, delimiter);
            if (i + 1 < cols.size()) ts << delimiter;
        }
        ts << "\n";
    };

    if (options.scope == ExportScope::Selection) {
        const auto rows = m_tableView && m_tableView->selectionModel()
                              ? m_tableView->selectionModel()->selectedRows()
                              : QModelIndexList();
        QList<int> rowNums;
        rowNums.reserve(rows.size());
        for (const auto& idx : rows) rowNums.append(idx.row());
        std::sort(rowNums.begin(), rowNums.end());
        rowNums.erase(std::unique(rowNums.begin(), rowNums.end()), rowNums.end());

        for (int r : rowNums) {
            writeRow(r, [&](int rr, int cc){
                return m_paged->data(m_paged->index(rr, cc), Qt::DisplayRole);
            });
        }
        return true;
    }

    if (options.scope == ExportScope::CurrentPage) {
        const int rows = m_paged->rowCount();
        for (int r = 0; r < rows; ++r) {
            writeRow(r, [&](int rr, int cc){
                return m_paged->data(m_paged->index(rr, cc), Qt::DisplayRole);
            });
        }
        return true;
    }

    const int rows = m_sortFilter->rowCount();
    for (int r = 0; r < rows; ++r) {
        writeRow(r, [&](int rr, int cc){
            return m_sortFilter->data(m_sortFilter->index(rr, cc), Qt::DisplayRole);
        });
    }
    return true;
}

void AdvancedTable::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    if (m_tableView) m_tableView->setSelectionMode(mode);
}

void AdvancedTable::setSelectionBehavior(QAbstractItemView::SelectionBehavior behavior)
{
    if (m_tableView) m_tableView->setSelectionBehavior(behavior);
}

void AdvancedTable::setEditTriggers(QAbstractItemView::EditTriggers triggers)
{
    if (m_tableView) m_tableView->setEditTriggers(triggers);
}


void AdvancedTable::setExportDelimiter(const QString& delim)
{
    m_exportDelimiter = delim.isEmpty() ? "," : delim;
}

QString AdvancedTable::exportDelimiter() const
{
    return m_exportDelimiter;
}

void AdvancedTable::setTopBarVisible(bool visible)
{
    if (m_topBar) m_topBar->setVisible(visible);
}

void AdvancedTable::setExportButtonVisible(bool visible)
{
    if (m_exportBtn) m_exportBtn->setVisible(visible);
}

void AdvancedTable::setColumnVisibilityMenuVisible(bool visible)
{
    if (m_columnsBtn) m_columnsBtn->setVisible(visible);
}

void AdvancedTable::setFilterControlVisible(bool visible)
{
    if (m_filterLabel) m_filterLabel->setVisible(visible);
    if (m_filterEdit) m_filterEdit->setVisible(visible);
}

void AdvancedTable::setColumnSelectorVisible(bool visible)
{
    if (m_columnLabel) m_columnLabel->setVisible(visible);
    if (m_columnCombo) m_columnCombo->setVisible(visible);
}

void AdvancedTable::setPageSizeSelectorVisible(bool visible)
{
    if (m_pageSizeLabel) m_pageSizeLabel->setVisible(visible);
    if (m_pageSizeCombo) m_pageSizeCombo->setVisible(visible);
    if (m_pageSizeBottomLabel) m_pageSizeBottomLabel->setVisible(visible);
    if (m_pageSizeBottom) m_pageSizeBottom->setVisible(visible);
}

void AdvancedTable::setJumpControlVisible(bool visible)
{
    if (m_jumpLabel) m_jumpLabel->setVisible(visible);
    if (m_jumpSpin) m_jumpSpin->setVisible(visible);
    if (m_pageJumpBottomLabel) m_pageJumpBottomLabel->setVisible(visible);
    if (m_pageJumpBottom) m_pageJumpBottom->setVisible(visible);
}

void AdvancedTable::setPaginationBarVisible(bool visible)
{
    if (m_bottomBar) m_bottomBar->setVisible(visible);
}

} // namespace basewidgets
