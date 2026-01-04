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
#include <QRegExp>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QTextStream>
#include <QClipboard>
#include <QApplication>
#include <QTimer>

namespace basewidgets {

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
    m_pageSizeCombo->addItems({"20", "50", "100"});
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
    auto* bottomRow = new QHBoxLayout();
    m_prevBtn = new PushButton("上一页", this);
    m_nextBtn = new PushButton("下一页", this);
    m_exportBtn = new PushButton("导出CSV", this);
    m_prevBtn->setObjectName("PrevPageBtn");
    m_nextBtn->setObjectName("NextPageBtn");
    m_exportBtn->setObjectName("ExportCsvBtn");
    m_nextBtn->setProperty("primary", true);
    m_pageSizeBottom = new ComboBox(this);
    m_pageSizeBottom->addItems({"10", "20", "50"});
    m_pageSizeBottom->setCurrentText("20");
    QObject::connect(m_pageSizeBottom, static_cast<void(ComboBox::*)(const QString&)>(&ComboBox::currentTextChanged), [this](const QString& s){
        m_paged->setPageSize(s.toInt());
        if (m_pageSizeCombo) m_pageSizeCombo->setCurrentText(s);
    });

    m_totalLabel = new Label(this);
    m_totalLabel->setText("共0条数据");

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
    m_pageScroll->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_pageScroll->setMaximumHeight(48);
    // m_pageScroll->setStyleSheet moved to QSS (#PageScroll)
    if (m_pageScroll->viewport()) m_pageScroll->viewport()->setAutoFillBackground(false);

    m_pageJumpBottom = new SpinBox(this);
    m_pageJumpBottom->setMinimum(1);
    QObject::connect(m_pageJumpBottom, static_cast<void(SpinBox::*)(int)>(&SpinBox::valueChanged), [this](int val){
        m_paged->setCurrentPage(qMax(0, val - 1));
    });

    bottomRow->setContentsMargins(6, 6, 6, 6);
    bottomRow->setSpacing(8);
    bottomRow->addWidget(new Label("每页显示记录数", this));
    bottomRow->addWidget(m_pageSizeBottom);
    bottomRow->addSpacing(8);
    bottomRow->addWidget(m_totalLabel);
    bottomRow->addStretch();
    bottomRow->addWidget(m_prevBtn);
    bottomRow->addWidget(m_nextBtn);
    bottomRow->addSpacing(8);
    bottomRow->addWidget(m_pageScroll, 1);
    bottomRow->addSpacing(8);
    bottomRow->addWidget(new Label("跳转", this));
    bottomRow->addWidget(m_pageJumpBottom);
    bottomRow->addSpacing(8);
    bottomRow->addWidget(m_exportBtn);

    m_topBar = new QWidget(this);
    m_topBar->setLayout(topRow);
    // 移除顶部重复的分页功能（每页与跳页），保留下部
    if (m_pageSizeLabel) m_pageSizeLabel->setVisible(false);
    if (m_pageSizeCombo) m_pageSizeCombo->setVisible(false);
    if (m_jumpLabel) m_jumpLabel->setVisible(false);
    if (m_jumpSpin) m_jumpSpin->setVisible(false);
    outer->addWidget(m_topBar);
    outer->addWidget(m_tableView);
    m_bottomBar = new QWidget(this);
    m_bottomBar->setProperty("pagination", true);
    m_bottomBar->setLayout(bottomRow);
    m_bottomBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    outer->addWidget(m_bottomBar);
    setLayout(outer);
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
        m_sortFilter->setFilterCaseSensitivity(m_filterCs);
        m_sortFilter->setFilterRegExp(QRegExp(m_filterEdit->text(), m_filterCs, QRegExp::FixedString));
        emit filterChanged(m_filterEdit->text(), m_sortFilter->filterKeyColumn());
    });

    QObject::connect(m_pageSizeCombo, static_cast<void(ComboBox::*)(const QString&)>(&ComboBox::currentTextChanged), [this](const QString& s){
        m_paged->setPageSize(s.toInt());
    });
    QObject::connect(m_columnCombo, static_cast<void(ComboBox::*)(int)>(&ComboBox::currentIndexChanged), [this](int idx){
        m_sortFilter->setFilterKeyColumn(idx - 1);
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
        if (m_totalLabel) {
            m_totalLabel->setText(QString("共%5条数据 | 当前页 %1 / %2 | 当前行 %3~%4")
                                  .arg(m_paged->currentPage() + 1)
                                  .arg(m_paged->pageCount())
                                  .arg(m_paged->currentPage() * m_paged->pageSize() + 1)
                                  .arg(qMin((m_paged->currentPage() + 1) * m_paged->pageSize(), m_paged->totalRows()))
                                  .arg(m_paged->totalRows()));
        }
        m_jumpSpin->setMaximum(qMax(1, m_paged->pageCount()));
        m_jumpSpin->setValue(m_paged->currentPage() + 1);
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
            const auto rows = m_tableView->selectionModel()->selectedRows();
            for (const QModelIndex& proxyRowIdx : rows) {
                int r = proxyRowIdx.row();
                for (int c = 0; c < m_source->columnCount(); ++c) {
                    QModelIndex src = m_paged->mapToSource(m_paged->index(r, c));
                    ts << m_source->data(src).toString();
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
}

void AdvancedTable::setSourceModel(QAbstractItemModel* source)
{
    m_source = source;
    m_sortFilter->setSourceModel(source);
    rebuildColumnUi();
    emit m_paged->pageChanged(m_paged->currentPage(), m_paged->pageCount());
}

QAbstractItemModel* AdvancedTable::sourceModel() const
{
    return m_source;
}

void AdvancedTable::rebuildColumnUi()
{
    if (!m_source) return;
    // 列可见性菜单
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
    for (auto* b : m_pageButtons) { b->deleteLater(); }
    m_pageButtons.clear();

    int pc = m_paged ? m_paged->pageCount() : 1;
    int cur = m_paged ? m_paged->currentPage() + 1 : 1;

    auto addBtn = [this, layout](int n, bool active){
        auto* btn = new PushButton(QString::number(n), this);
        btn->setProperty("page", true);
        btn->setProperty("active", active);
        QObject::connect(btn, &PushButton::clicked, [this, n](){ m_paged->setCurrentPage(n - 1); });
        layout->addWidget(btn);
        m_pageButtons.append(btn);
    };

    auto addEllipsis = [layout, this](){
        auto* ellipsis = new Label("...", this);
        layout->addWidget(ellipsis);
    };

    while (layout->count() > 0) {
        if (auto* item = layout->takeAt(0)) { if (item->widget()) item->widget()->deleteLater(); }
    }

    // 导航按钮（仅保留首页/尾页）
    m_firstBtn = new PushButton("首页", this);
    m_lastBtn = new PushButton("尾页", this);
    for (auto* nav : {m_firstBtn, m_lastBtn}) {
        nav->setProperty("page", true);
        layout->addWidget(nav);
    }
    m_firstBtn->setEnabled(cur > 1);
    m_lastBtn->setEnabled(cur < pc);
    QObject::connect(m_firstBtn, &PushButton::clicked, [this](){ m_paged->setCurrentPage(0); });
    QObject::connect(m_lastBtn, &PushButton::clicked, [this, pc](){ m_paged->setCurrentPage(pc - 1); });

    // 页码窗口（中心跟随当前页）
    const int window = 7; // 显示窗口大小
    const int side = 2;   // 当前页左右保留页数
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
    m_paged->setPageSize(size);
}

int AdvancedTable::pageSize() const
{
    return m_paged->pageSize();
}

void AdvancedTable::setFilterText(const QString& text)
{
    m_filterEdit->setText(text);
    m_sortFilter->setFilterCaseSensitivity(m_filterCs);
    m_sortFilter->setFilterRegExp(QRegExp(m_filterEdit->text(), m_filterCs, QRegExp::FixedString));
    emit filterChanged(m_filterEdit->text(), m_sortFilter->filterKeyColumn());
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
    if (filePath.isEmpty()) return false;
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream ts(&f);
    const QStringList headers = currentHeaderLabels();
    if (includeHeader && !headers.isEmpty()) {
        for (int c = 0; c < headers.size(); ++c) {
            ts << headers[c];
            if (c + 1 < headers.size()) ts << m_exportDelimiter;
        }
        ts << "\n";
    }
    if (m_sortFilter) {
        for (int r = 0; r < m_sortFilter->rowCount(); ++r) {
            for (int c = 0; c < m_sortFilter->columnCount(); ++c) {
                ts << m_sortFilter->data(m_sortFilter->index(r, c)).toString();
                if (c + 1 < m_sortFilter->columnCount()) ts << m_exportDelimiter;
            }
            ts << "\n";
        }
    }
    f.close();
    return true;
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
    m_sortFilter->setFilterCaseSensitivity(m_filterCs);
    m_sortFilter->setFilterRegExp(QRegExp(m_filterEdit->text(), m_filterCs, QRegExp::FixedString));
    emit filterChanged(m_filterEdit->text(), m_sortFilter->filterKeyColumn());
}

Qt::CaseSensitivity AdvancedTable::filterCaseSensitivity() const
{
    return m_filterCs;
}

void AdvancedTable::setFilterRegExp(const QRegExp& exp)
{
    m_sortFilter->setFilterRegExp(exp);
    emit filterChanged(m_filterEdit ? m_filterEdit->text() : QString(), m_sortFilter->filterKeyColumn());
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
}

void AdvancedTable::setJumpControlVisible(bool visible)
{
    if (m_jumpLabel) m_jumpLabel->setVisible(visible);
    if (m_jumpSpin) m_jumpSpin->setVisible(visible);
}

void AdvancedTable::setPaginationBarVisible(bool visible)
{
    if (m_bottomBar) m_bottomBar->setVisible(visible);
}

} // namespace basewidgets

