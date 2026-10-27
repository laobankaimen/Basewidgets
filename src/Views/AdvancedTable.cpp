/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Views/AdvancedTable.h>
#include "TableRowChecks_p.h"

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
#include <QSaveFile>
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

AdvancedTable::~AdvancedTable()
{
    // Views and proxies are children: disconnect our callbacks before QObject tears them down.
    for (const auto& connection : m_sourceConnections) disconnect(connection);
    disconnect(m_paged, nullptr, this, nullptr);
}

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
    auto* checkView = new RowCheckTableView(this);
    m_tableView = checkView;
    checkView->rowChecks->isChecked=[this](int row) { return isRowChecked(mapToSource(m_paged->index(row,0))); };
    checkView->rowChecks->isCheckable=[this](int row) { return isRowCheckable(mapToSource(m_paged->index(row,0))); };
    checkView->rowChecks->toggle=[this](int row) {
        const auto source=mapToSource(m_paged->index(row,0)); setRowChecked(source,!isRowChecked(source));
    };
    connect(checkView->allCheck,&QCheckBox::clicked,this,[this] {
        // Derive the operation from model state, not QCheckBox's three-state click cycle.
        setCurrentPageChecked(currentPageCheckState()!=Qt::Checked);
    });
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
    m_pageSizeBottom = new ComboBox(this);
    m_pageSizeBottom->addItems({"10", "20", "50", "100"});
    m_pageSizeBottom->setCurrentText("20");
    QObject::connect(m_pageSizeBottom, static_cast<void(ComboBox::*)(const QString&)>(&ComboBox::currentTextChanged), [this](const QString& s){
        m_paged->setPageSize(s.toInt());
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
    connect(m_paged, &PagedProxyModel::pageChanged, this, [this] {
        updatePageInfo(); emit pageChanged(m_paged->currentPage(), m_paged->pageCount());
    });
    // 保留可见记录的选择，避免排序把选中状态转移到另一个业务记录。
    connect(m_paged, &QAbstractItemModel::modelAboutToBeReset, this, [this] {
        m_savedSelection.clear();
        for (const auto& index : selectedSourceIndexes()) m_savedSelection.append(index);
        m_savedCurrent = mapToSource(m_tableView->currentIndex());
    });
    connect(m_paged, &QAbstractItemModel::modelReset, this, [this] {
        QItemSelection selection;
        for (const auto& source : m_savedSelection) {
            const auto index = mapFromSource(source);
            if (index.isValid()) selection.select(index,index);
        }
        m_tableView->selectionModel()->select(selection,QItemSelectionModel::ClearAndSelect);
        m_tableView->selectionModel()->setCurrentIndex(mapFromSource(m_savedCurrent),QItemSelectionModel::NoUpdate);
        m_savedSelection.clear(); m_savedCurrent = QPersistentModelIndex();
    });
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableView, &TableView::customContextMenuRequested, this, &AdvancedTable::showContextMenu);
    connect(m_tableView, &TableView::activated, this, [this](const QModelIndex& index) {
        emit sourceActivated(mapToSource(index));
    });

    // 导出CSV（当前过滤结果）
    QObject::connect(m_exportBtn, &PushButton::clicked, [this](){
        const QString fileName = DialogUtils::saveFile(this, tr("导出为CSV"), QString(), "CSV (*.csv)");
        if (fileName.isEmpty()) return;
        const bool ok = exportCsvToFile(fileName, true);
        if (ok) emit exportCompleted(fileName, m_sortFilter ? m_sortFilter->rowCount() : 0);
        else emit exportFailed(fileName, tr("写入失败或文件不可用"));
    });

    connect(m_paged,&QAbstractItemModel::modelReset,this,&AdvancedTable::refreshCheckedRows);
    connect(m_paged,&QAbstractItemModel::dataChanged,this,[this] { refreshCheckedRows(); });
    refreshCheckedRows();
    rebuildPaginationUi();
}

void AdvancedTable::setSourceModel(QAbstractItemModel* source)
{
    if (source == m_source) return;
    // Reject our own model chain (including indirect cycles).
    for (auto* model=source; model;) {
        if (model==m_sortFilter || model==m_paged) return;
        auto* proxy=qobject_cast<QAbstractProxyModel*>(model);
        model=proxy ? proxy->sourceModel() : nullptr;
    }
    for (const auto& connection : m_sourceConnections) disconnect(connection);
    m_sourceConnections.clear(); m_topPins.clear(); m_bottomPins.clear();
    clearCheckedRows();
    m_paged->setPinnedIndexes({},{});
    m_source = source;
    m_sortFilter->setSourceModel(source);
    if (source) {
        auto refresh=[this] { scheduleColumnRefresh(); };
        m_sourceConnections << connect(source,&QAbstractItemModel::rowsInserted,this,[this] { refreshCheckedRows(); })
            << connect(source,&QAbstractItemModel::rowsRemoved,this,[this] { refreshCheckedRows(); })
            << connect(source,&QAbstractItemModel::layoutChanged,this,[this] { refreshCheckedRows(); })
            << connect(source,&QAbstractItemModel::rowsMoved,this,[this] { refreshCheckedRows(); })
            << connect(source,&QAbstractItemModel::columnsInserted,this,refresh)
            << connect(source,&QAbstractItemModel::columnsRemoved,this,refresh)
            << connect(source,&QAbstractItemModel::columnsMoved,this,refresh)
            << connect(source,&QAbstractItemModel::headerDataChanged,this,[this](Qt::Orientation orientation) {
                if (orientation==Qt::Horizontal) scheduleColumnRefresh();
            })
            << connect(source,&QAbstractItemModel::modelReset,this,[this] {
                m_topPins.clear(); m_bottomPins.clear(); refreshCheckedRows(); scheduleColumnRefresh(); emit pinnedRowsChanged();
            })
            << connect(source,&QObject::destroyed,this,[this] {
                m_topPins.clear(); m_bottomPins.clear(); refreshCheckedRows(); scheduleColumnRefresh(); emit pinnedRowsChanged();
            });
    }
    rebuildColumnUi(); updatePageInfo(); refreshCheckedRows(); emit pinnedRowsChanged();
}

void AdvancedTable::scheduleColumnRefresh()
{
    if (m_columnRefreshPending) return;
    m_columnRefreshPending=true;
    QTimer::singleShot(0,this,[this] { m_columnRefreshPending=false; rebuildColumnUi(); updatePageInfo(); });
}

QAbstractItemModel* AdvancedTable::sourceModel() const
{
    return m_source;
}

void AdvancedTable::rebuildColumnUi()
{
    m_columnsMenu->clear();
    const int oldColumn=m_sortFilter->filterKeyColumn();
    const auto headers=currentHeaderLabels();
    for (int c=0;c<headers.size();++c) {
        auto* action=m_columnsMenu->addAction(headers[c]);
        action->setCheckable(true); action->setChecked(!m_tableView->isColumnHidden(c));
        connect(action,&QAction::toggled,this,[this,c](bool checked) { m_tableView->setColumnHidden(c,!checked); });
    }
    const QSignalBlocker block(m_columnCombo);
    m_columnCombo->clear(); m_columnCombo->addItem(tr("全部列")); m_columnCombo->addItems(headers);
    const int column=oldColumn>=headers.size() ? -1 : oldColumn;
    m_columnCombo->setCurrentIndex(column+1);
    m_sortFilter->setFilterKeyColumn(column);
}

void AdvancedTable::updatePageInfo()
{
    const bool paginated=m_paged->paginationEnabled();
    m_exportBtn->setEnabled(m_source && m_source->columnCount()>0);
    m_totalLabel->setText(paginated
        ? tr("%1 条 · 固定 %2 · %3/%4 页")
              .arg(m_paged->totalRows()).arg(m_paged->pinnedRowCount())
              .arg(m_paged->pageCount()?m_paged->currentPage()+1:0).arg(m_paged->pageCount())
        : tr("共 %1 条 · 固定 %2 条").arg(m_paged->totalRows()).arg(m_paged->pinnedRowCount()));
    m_totalLabel->setToolTip(tr("当前显示 %1 条，含 %2 条固定记录；每页普通记录数 %3。")
        .arg(m_paged->rowCount()).arg(m_paged->pinnedRowCount()).arg(m_paged->pageSize()));
    const QSignalBlocker block(m_pageJumpBottom);
    m_pageJumpBottom->setMaximum(qMax(1,m_paged->pageCount()));
    m_pageJumpBottom->setValue(m_paged->currentPage()+1);
    m_prevBtn->setEnabled(m_paged->currentPage()>0);
    m_nextBtn->setEnabled(m_paged->currentPage()+1<m_paged->pageCount());
    rebuildPaginationUi();
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
        btn->setCheckable(true); btn->setChecked(active);
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
    auto options=m_filterRe.patternOptions();
    options.setFlag(QRegularExpression::CaseInsensitiveOption,m_filterCs==Qt::CaseInsensitive);
    const QString text=m_filterEdit->text();
    const QRegularExpression candidate(m_filterMode==FilterMode::RegularExpression?text:QRegularExpression::escape(text),options);
    if (!candidate.isValid()) {
        m_filterEdit->setToolTip(candidate.errorString()); emit filterRejected(candidate.errorString()); return;
    }
    m_filterRe=candidate; m_filterEdit->setToolTip(QString());
    m_sortFilter->setFilterRegularExpression(candidate);
    emit filterChanged(text,m_sortFilter->filterKeyColumn());
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
    if (size<=0) return;
    if (m_paged) m_paged->setPageSize(size);
    const QString text = QString::number(size);
    const QSignalBlocker block(m_pageSizeBottom);
    if (m_pageSizeBottom->findText(text)<0) m_pageSizeBottom->addItem(text);
    m_pageSizeBottom->setCurrentText(text);
}

int AdvancedTable::pageSize() const
{
    return m_paged ? m_paged->pageSize() : 0;
}

void AdvancedTable::setPageSizeOptions(const QStringList& sizes)
{
    const QSignalBlocker block(m_pageSizeBottom);
    m_pageSizeBottom->clear();
    QSet<int> seen;
    for (const auto& text:sizes) {
        bool ok=false; const int size=text.toInt(&ok);
        if (ok && size>0 && !seen.contains(size)) { seen.insert(size); m_pageSizeBottom->addItem(QString::number(size)); }
    }
    setPageSize(m_paged->pageSize());
}

void AdvancedTable::setFilterText(const QString& text)
{
    if (m_filterTimer) m_filterTimer->stop();
    if (m_filterEdit) { const QSignalBlocker block(m_filterEdit); m_filterEdit->setText(text); }
    applyFilter();
}

void AdvancedTable::setFilterColumn(int column)
{
    if (column < -1 || (column >= 0 && (!m_source || column >= m_source->columnCount()))) return;
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
    // Keep Qt 5's QRegExp dialect (including wildcard/fixed-string modes) at this legacy entry point.
    if(!exp.isValid()) { emit filterRejected(exp.errorString()); return; }
    m_filterMode=exp.patternSyntax()==QRegExp::FixedString?FilterMode::Literal:FilterMode::RegularExpression;
    m_filterCs=exp.caseSensitivity(); m_filterTimer->stop();
    { const QSignalBlocker block(m_filterEdit); m_filterEdit->setText(exp.pattern()); }
    m_filterEdit->setToolTip(QString());
    m_sortFilter->setFilterRegExp(exp);
    QString pattern=exp.pattern();
    if(exp.patternSyntax()==QRegExp::FixedString)pattern=QRegularExpression::escape(pattern);
    else if(exp.patternSyntax()==QRegExp::Wildcard || exp.patternSyntax()==QRegExp::WildcardUnix)
        pattern=QRegularExpression::wildcardToRegularExpression(pattern);
    m_filterRe=QRegularExpression(pattern,m_filterCs==Qt::CaseInsensitive?QRegularExpression::CaseInsensitiveOption:QRegularExpression::NoPatternOption);
    emit filterChanged(exp.pattern(),m_sortFilter->filterKeyColumn());
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
    if (!re.isValid()) { emit filterRejected(re.errorString()); return; }
    m_filterMode = FilterMode::RegularExpression; m_filterRe = re;
    m_filterCs=re.patternOptions().testFlag(QRegularExpression::CaseInsensitiveOption)?Qt::CaseInsensitive:Qt::CaseSensitive;
    m_filterTimer->stop();
    { const QSignalBlocker block(m_filterEdit); m_filterEdit->setText(re.pattern()); }
    m_filterEdit->setToolTip(QString());
    m_sortFilter->setFilterRegularExpression(re);
    emit filterChanged(re.pattern(),m_sortFilter->filterKeyColumn());
}

QRegularExpression AdvancedTable::filterRegularExpression() const
{
    return m_filterRe;
}

void AdvancedTable::setPaginationConfig(const PaginationConfig& cfg)
{
    m_paginationCfg.windowSize = qBound(3,cfg.windowSize,31);
    m_paginationCfg.sideCount = qBound(1,cfg.sideCount,10);
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

    QSaveFile f(options.filePath);
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

    auto finish = [&]() {
        ts.flush();
        if (ts.status()!=QTextStream::Ok || !f.commit()) {
            if (error) *error=f.errorString(); return false;
        }
        return true;
    };
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
                              ? m_tableView->selectionModel()->selectedIndexes()
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
        return finish();
    }

    if (options.scope == ExportScope::CurrentPage) {
        const int rows = m_paged->rowCount();
        for (int r = 0; r < rows; ++r) {
            writeRow(r, [&](int rr, int cc){
                return m_paged->data(m_paged->index(rr, cc), Qt::DisplayRole);
            });
        }
        return finish();
    }

    const int rows = m_sortFilter->rowCount();
    for (int r = 0; r < rows; ++r) {
        writeRow(r, [&](int rr, int cc){
            return m_sortFilter->data(m_sortFilter->index(rr, cc), Qt::DisplayRole);
        });
    }
    return finish();
}

void AdvancedTable::setRowCheckBoxesVisible(bool visible)
{
    m_rowChecksVisible=visible;
    static_cast<RowCheckTableView*>(m_tableView)->setChecksVisible(visible);
}
QModelIndexList AdvancedTable::checkedSourceRows() const
{
    QModelIndexList result;
    for(const auto& index:m_checkedRows)if(index.isValid() && index.model()==m_source)
        result.append(index.sibling(index.row(),0));
    std::sort(result.begin(),result.end(),[](const QModelIndex& a,const QModelIndex& b) { return a.row()<b.row(); });
    return result;
}
bool AdvancedTable::isRowChecked(const QModelIndex& source) const
{
    if(!source.isValid() || source.model()!=m_source || source.parent().isValid())return false;
    // Compare record identity, not its current page/row position. QVector avoids hashing
    // persistent indexes whose row/hash changes after source insertions or sorting.
    for(const auto& index:m_checkedRows)if(index.isValid() && index.row()==source.row())return true;
    return false;
}
bool AdvancedTable::isRowCheckable(const QModelIndex& source) const
{
    if(!source.isValid() || source.model()!=m_source || source.parent().isValid())return false;
    const auto flags=source.sibling(source.row(),0).flags();
    return flags.testFlag(Qt::ItemIsEnabled) && flags.testFlag(Qt::ItemIsSelectable);
}
bool AdvancedTable::setRowChecked(const QModelIndex& source,bool checked)
{
    if(!source.isValid() || source.model()!=m_source || source.parent().isValid() || (checked && !isRowCheckable(source)))return false;
    if(isRowChecked(source)==checked)return true;
    if(checked)m_checkedRows.append(source.sibling(source.row(),0));
    else m_checkedRows.erase(std::remove_if(m_checkedRows.begin(),m_checkedRows.end(),[&](const QPersistentModelIndex& index) {
        return index.isValid() && index.row()==source.row();
    }),m_checkedRows.end());
    refreshCheckedRows(); emit checkedRowsChanged(); return true;
}
Qt::CheckState AdvancedTable::currentPageCheckState() const
{
    QSet<int> checked;
    for(const auto& index:m_checkedRows)if(index.isValid())checked.insert(index.row());
    int count=0,total=0;
    for(int row=0;row<m_paged->rowCount();++row) {
        const auto source=mapToSource(m_paged->index(row,0));
        if(!isRowCheckable(source))continue;
        ++total; if(checked.contains(source.row()))++count;
    }
    return count==0?Qt::Unchecked:count==total?Qt::Checked:Qt::PartiallyChecked;
}
void AdvancedTable::setCurrentPageChecked(bool checked)
{
    QSet<int> existing,targets;
    for(const auto& index:m_checkedRows)if(index.isValid())existing.insert(index.row());
    bool changed=false;
    for(int row=0;row<m_paged->rowCount();++row) {
        const auto source=mapToSource(m_paged->index(row,0));
        if(!isRowCheckable(source))continue;
        if(checked && !existing.contains(source.row())) {
            m_checkedRows.append(source); existing.insert(source.row()); changed=true;
        } else if(!checked && existing.contains(source.row())) { targets.insert(source.row()); changed=true; }
    }
    if(!checked)m_checkedRows.erase(std::remove_if(m_checkedRows.begin(),m_checkedRows.end(),[&](const QPersistentModelIndex& index) {
        return index.isValid() && targets.contains(index.row());
    }),m_checkedRows.end());
    refreshCheckedRows(); if(changed)emit checkedRowsChanged();
}
void AdvancedTable::clearCheckedRows()
{
    const bool changed=!m_checkedRows.isEmpty(); m_checkedRows.clear(); refreshCheckedRows();
    if(changed)emit checkedRowsChanged();
}
void AdvancedTable::refreshCheckedRows()
{
    const int before=m_checkedRows.size();
    m_checkedRows.erase(std::remove_if(m_checkedRows.begin(),m_checkedRows.end(),[this](const QPersistentModelIndex& index) {
        return !index.isValid() || index.model()!=m_source;
    }),m_checkedRows.end());
    auto* view=static_cast<RowCheckTableView*>(m_tableView);
    bool any=false;
    for(int row=0;row<m_paged->rowCount();++row)if(isRowCheckable(mapToSource(m_paged->index(row,0)))) { any=true; break; }
    { const QSignalBlocker block(view->allCheck);
      view->allCheck->setCheckState(currentPageCheckState()); view->allCheck->setEnabled(any); }
    view->refreshChecks();
    if(before!=m_checkedRows.size())emit checkedRowsChanged();
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
    if (m_pageSizeBottomLabel) m_pageSizeBottomLabel->setVisible(visible);
    if (m_pageSizeBottom) m_pageSizeBottom->setVisible(visible);
}

void AdvancedTable::setJumpControlVisible(bool visible)
{
    if (m_pageJumpBottomLabel) m_pageJumpBottomLabel->setVisible(visible);
    if (m_pageJumpBottom) m_pageJumpBottom->setVisible(visible);
}

void AdvancedTable::setPaginationBarVisible(bool visible)
{
    if (m_bottomBar) m_bottomBar->setVisible(visible);
}


void AdvancedTable::setPresentationMode(PresentationMode mode)
{
    m_mode=mode;
    const bool paginated=mode==PresentationMode::Paged;
    m_paged->setPaginationEnabled(paginated);
    m_topBar->setVisible(mode!=PresentationMode::Basic);
    m_bottomBar->setVisible(mode!=PresentationMode::Basic);
    for (auto* widget : QList<QWidget*>{m_firstBtn,m_prevBtn,m_nextBtn,m_lastBtn,m_pageScroll}) widget->setVisible(paginated);
    setPageSizeSelectorVisible(paginated); setJumpControlVisible(paginated);
    updatePageInfo();
}
void AdvancedTable::setSortingEnabled(bool enabled)
{
    m_tableView->setSortingEnabled(enabled);
    if(!enabled)m_sortFilter->sort(-1); // Restore source order; fixed rows retain their explicit positions.
}
void AdvancedTable::setSortRole(int role) { m_sortFilter->setSortRole(role); }
QModelIndex AdvancedTable::mapToSource(const QModelIndex& index) const
{
    return index.isValid() && index.model()==m_paged ? m_sortFilter->mapToSource(m_paged->mapToSource(index)) : QModelIndex();
}
QModelIndex AdvancedTable::mapFromSource(const QModelIndex& index) const
{
    return index.isValid() && index.model()==m_source ? m_paged->mapFromSource(m_sortFilter->mapFromSource(index)) : QModelIndex();
}
QModelIndexList AdvancedTable::selectedSourceIndexes() const
{
    QModelIndexList result;
    if (m_tableView->selectionModel()) for (const auto& index:m_tableView->selectionModel()->selectedIndexes()) {
        const auto source=mapToSource(index); if (source.isValid()) result.append(source);
    }
    return result;
}
QModelIndexList AdvancedTable::selectedSourceRows() const
{
    QModelIndexList result; QSet<int> seen;
    auto indexes=m_tableView->selectionModel()->selectedIndexes();
    std::sort(indexes.begin(),indexes.end(),[](const QModelIndex& a,const QModelIndex& b) { return a.row()<b.row(); });
    for (const auto& index:indexes) if (!seen.contains(index.row())) {
        seen.insert(index.row()); const auto source=mapToSource(index.sibling(index.row(),0));
        if (source.isValid()) result.append(source);
    }
    return result;
}
QModelIndexList AdvancedTable::pinnedSourceRows(PinPosition position) const
{
    QModelIndexList result;
    for (const auto& pin:position==PinPosition::Top?m_topPins:m_bottomPins) if (pin.isValid()) result.append(pin);
    return result;
}
bool AdvancedTable::commitPins(const QModelIndexList& top,const QModelIndexList& bottom)
{
    if (!m_paged->setPinnedIndexes(top,bottom)) return false;
    m_topPins.clear(); m_bottomPins.clear();
    for (const auto& index:top) m_topPins.append(index);
    for (const auto& index:bottom) m_bottomPins.append(index);
    emit pinnedRowsChanged(); return true;
}
bool AdvancedTable::setPinnedRows(const QList<int>& topRows,const QList<int>& bottomRows)
{
    QModelIndexList top,bottom;
    auto convert=[this](const QList<int>& rows,QModelIndexList& out) {
        for (int row:rows) {
            if (!m_source || row<0 || row>=m_source->rowCount() || m_source->columnCount()==0) return false;
            out.append(m_source->index(row,0));
        }
        return true;
    };
    return convert(topRows,top) && convert(bottomRows,bottom) && commitPins(top,bottom);
}
bool AdvancedTable::pinRows(const QModelIndexList& rows,PinPosition position)
{
    auto top=pinnedSourceRows(PinPosition::Top), bottom=pinnedSourceRows(PinPosition::Bottom);
    QSet<int> seen;
    for (const auto& row:rows) {
        if (!row.isValid() || row.model()!=m_source || row.parent().isValid()) return false;
        const auto pin=row.sibling(row.row(),0); if (seen.contains(pin.row())) continue;
        seen.insert(pin.row()); top.removeAll(pin); bottom.removeAll(pin);
        (position==PinPosition::Top?top:bottom).append(pin);
    }
    return commitPins(top,bottom);
}
void AdvancedTable::unpinRows(const QModelIndexList& rows)
{
    auto top=pinnedSourceRows(PinPosition::Top), bottom=pinnedSourceRows(PinPosition::Bottom);
    for (const auto& row:rows) { const auto pin=row.sibling(row.row(),0); top.removeAll(pin); bottom.removeAll(pin); }
    commitPins(top,bottom);
}
void AdvancedTable::clearPinnedRows() { commitPins({},{}); }
void AdvancedTable::showContextMenu(const QPoint& pos)
{
    const auto clicked=m_tableView->indexAt(pos);
    if (clicked.isValid() && !m_tableView->selectionModel()->isSelected(clicked))
        m_tableView->selectionModel()->setCurrentIndex(clicked,QItemSelectionModel::ClearAndSelect |
            (m_tableView->selectionBehavior()==QAbstractItemView::SelectRows?QItemSelectionModel::Rows:QItemSelectionModel::NoUpdate));
    const QPersistentModelIndex source=mapToSource(clicked);
    const QPoint global=m_tableView->viewport()->mapToGlobal(pos);
    emit contextMenuRequested(source,global);
    auto* menu=new QMenu(m_tableView);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    const auto rows=selectedSourceRows();
    QVector<QPersistentModelIndex> persistent;
    for (const auto& row:rows) persistent.append(row);
    auto liveRows=[persistent] { QModelIndexList result; for (const auto& row:persistent) if (row.isValid()) result.append(row); return result; };
    if (m_defaultContextMenu) {
        auto* copy=menu->addAction(tr("复制选中行为 CSV")); copy->setObjectName("copyRows"); copy->setEnabled(!rows.isEmpty());
        connect(copy,&QAction::triggered,this,[this,liveRows] {
            QString text; QTextStream stream(&text);
            for (const auto& row:liveRows()) {
                if (row.model()!=m_source) continue;
                for (int c=0;c<m_source->columnCount();++c) {
                    if(c) stream<<m_exportDelimiter;
                    stream<<escapeCsvField(row.sibling(row.row(),c).data().toString(),m_exportDelimiter);
                }
                stream<<"\n";
            }
            QApplication::clipboard()->setText(text);
        });
        auto* cell=menu->addAction(tr("复制单元格")); cell->setEnabled(source.isValid());
        connect(cell,&QAction::triggered,this,[source] { if(source.isValid()) QApplication::clipboard()->setText(source.data().toString()); });
        auto* all=menu->addAction(tr("选择当前页"));
        connect(all,&QAction::triggered,m_tableView,&QTableView::selectAll);
        menu->addSeparator();
        for (const auto order:{Qt::AscendingOrder,Qt::DescendingOrder}) {
            auto* action=menu->addAction(order==Qt::AscendingOrder?tr("按此列升序"):tr("按此列降序"));
            action->setEnabled(source.isValid() && m_tableView->isSortingEnabled());
            connect(action,&QAction::triggered,this,[this,source,order] { if(source.isValid()&&source.model()==m_source) m_tableView->sortByColumn(source.column(),order); });
        }
        auto* pinMenu=menu->addMenu(tr("固定记录"));
        for (const auto position:{PinPosition::Top,PinPosition::Bottom}) {
            auto* action=pinMenu->addAction(position==PinPosition::Top?tr("固定到顶部"):tr("固定到底部")); action->setEnabled(!rows.isEmpty());
            connect(action,&QAction::triggered,this,[this,liveRows,position] { pinRows(liveRows(),position); });
        }
        auto* unpin=pinMenu->addAction(tr("取消选中记录的固定")); unpin->setEnabled(!rows.isEmpty());
        connect(unpin,&QAction::triggered,this,[this,liveRows] { unpinRows(liveRows()); });
        auto* clear=pinMenu->addAction(tr("取消全部固定"));
        clear->setEnabled(!m_topPins.isEmpty()||!m_bottomPins.isEmpty());
        connect(clear,&QAction::triggered,this,&AdvancedTable::clearPinnedRows);
        menu->addSeparator();
        auto* fit=menu->addAction(tr("列宽适应内容")); connect(fit,&QAction::triggered,m_tableView,&QTableView::resizeColumnsToContents);
        auto* hide=menu->addAction(tr("隐藏此列")); hide->setEnabled(source.isValid());
        connect(hide,&QAction::triggered,this,[this,source] { if(source.isValid()&&source.model()==m_source) { m_tableView->hideColumn(source.column()); rebuildColumnUi(); } });
        auto* columns=menu->addMenu(tr("显示列"));
        const auto headers=currentHeaderLabels();
        for(int c=0;c<headers.size();++c) {
            auto* action=columns->addAction(headers[c]); action->setCheckable(true); action->setChecked(!m_tableView->isColumnHidden(c));
            connect(action,&QAction::toggled,this,[this,c](bool checked) { m_tableView->setColumnHidden(c,!checked); rebuildColumnUi(); });
        }
        auto* filter=menu->addAction(tr("清空筛选")); filter->setEnabled(!m_filterEdit->text().isEmpty());
        connect(filter,&QAction::triggered,this,[this] { setFilterText(QString()); });
    }
    emit prepareContextMenu(menu,rows);
    if (menu->isEmpty()) { menu->deleteLater(); return; }
    menu->popup(global);
}

} // namespace basewidgets
