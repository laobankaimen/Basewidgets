/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_ADVANCEDTABLE_H
#define BASEWIDGETS_ADVANCEDTABLE_H

#include <basewidgets/Core/StyledWidget.h>
#include <basewidgets/QtAdvancedUIExport.h>
#include <basewidgets/Views/PagedProxyModel.h>
#include <QScrollArea>
#include <QAbstractItemView>
#include <QRegularExpression>
#include <QPointer>
#include <QPersistentModelIndex>

class QAbstractItemModel;
class QSortFilterProxyModel;
class QHeaderView;
class QMenu;
class QTimer;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QRegExp;
#endif

namespace basewidgets {

class LineEdit;
class ComboBox;
class SpinBox;
class PushButton;
class ToolButton;
class Label;
class TableView;

/** 复合表格：源模型→筛选/排序代理→分页代理→TableView。内部控件归本控件持有。 */
class ACSS_UI_EXPORT AdvancedTable : public StyledWidget {
    Q_OBJECT
public:
    /** Basic 仅表格；Standard 带筛选/导出但不分页；Paged 增加分页。 */
    enum class PresentationMode { Basic, Standard, Paged };
    enum class PinPosition { Top, Bottom };

    enum class FilterMode {
        Literal,
        RegularExpression,
    };

    enum class ExportScope {
        Filtered,
        CurrentPage,
        Selection,
    };

    struct PaginationConfig {
        int windowSize = 7;
        int sideCount = 2;
    };

    struct CsvExportOptions {
        QString filePath;
        ExportScope scope = ExportScope::Filtered;
        bool includeHeader = true;
        bool includeUtf8Bom = false;
        QString delimiter = ",";
        QList<int> columns;
    };

    explicit AdvancedTable(QWidget* parent = nullptr);
    explicit AdvancedTable(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~AdvancedTable() override;

    /** 不接管源模型所有权；模型应由应用/页面持有。替换模型会刷新列配置。 */
    void setSourceModel(QAbstractItemModel* source);
    QAbstractItemModel* sourceModel() const;

    TableView* view() const { return m_tableView; }
    /** 借用分页代理；currentPage 从 0 开始，用户可见跳转从 1 开始。 */
    PagedProxyModel* pager() const { return m_paged; }
    QSortFilterProxyModel* sortFilterProxy() const { return m_sortFilter; }
    QHeaderView* header() const;

    /** 模式只是 UI/分页预设，不改变源模型、排序、筛选或固定记录。默认 Paged。 */
    void setPresentationMode(PresentationMode mode);
    PresentationMode presentationMode() const { return m_mode; }
    void setSortingEnabled(bool enabled);
    void setSortRole(int role);
    /** 传入本类 view() 的索引，返回调用者 setSourceModel() 提供的模型索引。 */
    QModelIndex mapToSource(const QModelIndex& viewIndex) const;
    /** 记录被过滤/不在当前页时返回无效索引。 */
    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    QModelIndexList selectedSourceIndexes() const;
    /** 支持单元格选择：只要该行有单元格选中就返回其源模型第 0 列索引。 */
    QModelIndexList selectedSourceRows() const;
    /** 原子替换固定行；行号属于源模型，合计最多 64 行，重复/越界返回 false。
     * 顺序按参数保留，升降序都不改变；遵守筛选，每页重复显示；pageSize 不含固定行。
     * 固定的是记录顺序，不是滚动视口。源模型 reset/替换后须重新设置。 */
    bool setPinnedRows(const QList<int>& topRows, const QList<int>& bottomRows = {});
    /** 追加到对应固定区；已固定的记录会移动到新位置。 */
    bool pinRows(const QModelIndexList& sourceRows, PinPosition position = PinPosition::Top);
    void unpinRows(const QModelIndexList& sourceRows);
    void clearPinnedRows();
    QModelIndexList pinnedSourceRows(PinPosition position) const;
    /** 关闭内置菜单后仍会发出 prepareContextMenu，便于完全自定义。 */
    void setDefaultContextMenuEnabled(bool enabled) { m_defaultContextMenu = enabled; }
    void setPageSize(int size);
    int pageSize() const;
    void setPageSizeOptions(const QStringList& sizes);
    void setFilterText(const QString& text);
    void setFilterColumn(int column); // -1 表示全部列
    void setFilterCaseSensitivity(Qt::CaseSensitivity cs);
    Qt::CaseSensitivity filterCaseSensitivity() const;
    void setFilterMode(FilterMode mode);
    FilterMode filterMode() const;
    void setFilterRegularExpression(const QRegularExpression& re);
    QRegularExpression filterRegularExpression() const;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void setFilterRegExp(const QRegExp& exp);
#endif

    void setPaginationConfig(const PaginationConfig& cfg);
    PaginationConfig paginationConfig() const;

    // 导出当前过滤结果到 CSV 文件（不含表头或含表头由参数决定）
    bool exportCsvToFile(const QString& filePath, bool includeHeader = true) const;
    /** 导出失败返回 false，可选 error 接收原因；字段按 CSV 规则转义。 */
    bool exportCsv(const CsvExportOptions& options, QString* error = nullptr) const;
    void setExportDelimiter(const QString& delim);
    QString exportDelimiter() const;

    /** 行头勾选默认开启；隐藏不清空勾选。勾选与 Qt 单元格/行高亮选择独立，
     * 不写入源模型 CheckStateRole、不增加业务模型列。表头全选只作用于当前页。
     * 排序/翻页/筛选保留勾选；删除记录会移除其勾选，模型 reset/替换会清空。 */
    void setRowCheckBoxesVisible(bool visible);
    bool rowCheckBoxesVisible() const { return m_rowChecksVisible; }
    /** 返回跨页（包括被筛选隐藏）的已勾选记录，第 0 列，按当前源模型行号排序。 */
    QModelIndexList checkedSourceRows() const;
    bool isRowChecked(const QModelIndex& sourceIndex) const;
    /** 索引必须属于 sourceModel() 的根层记录；禁用/不可选择记录不能新勾选。 */
    bool setRowChecked(const QModelIndex& sourceIndex, bool checked);
    void setCurrentPageChecked(bool checked);
    void clearCheckedRows();
    Qt::CheckState currentPageCheckState() const;

    void setSelectionMode(QAbstractItemView::SelectionMode mode);
    void setSelectionBehavior(QAbstractItemView::SelectionBehavior behavior);
    void setEditTriggers(QAbstractItemView::EditTriggers triggers);

    // 组件可见性控制
    void setTopBarVisible(bool visible);
    void setExportButtonVisible(bool visible);
    void setColumnVisibilityMenuVisible(bool visible);
    void setFilterControlVisible(bool visible);
    void setColumnSelectorVisible(bool visible);
    void setPageSizeSelectorVisible(bool visible);
    void setJumpControlVisible(bool visible);
    void setPaginationBarVisible(bool visible);

signals:
    void filterChanged(const QString& text, int column);
    void pageChanged(int currentPage, int totalPages);
    void exportCompleted(const QString& filePath, int rows);
    void exportFailed(const QString& filePath, const QString& error);
    /** menu 由控件持有，关闭后删除；selectedRows 属于 sourceModel()，非分页代理。
     * 在同步槽内添加动作；动作执行时若异步保留索引，请使用 QPersistentModelIndex。 */
    void prepareContextMenu(QMenu* menu, const QModelIndexList& selectedRows);
    void contextMenuRequested(const QModelIndex& sourceIndex, const QPoint& globalPosition);
    void sourceActivated(const QModelIndex& sourceIndex);
    void pinnedRowsChanged();
    /** 勾选集合改变时发送；批量全选/清空只发送一次。排序/翻页不改变集合。 */
    void checkedRowsChanged();
    /** 无效正则保留上次有效结果，输入框会显示错误提示。 */
    void filterRejected(const QString& error);

protected:
    void applyStylesheet();
    void resizeEvent(QResizeEvent*) override;

private:
    void initUi();
    void bindSignals();
    void rebuildColumnUi();
    void rebuildPaginationUi();
    void applyFilter();
    void showContextMenu(const QPoint& pos);
    void updatePageInfo();
    bool commitPins(const QModelIndexList& top, const QModelIndexList& bottom);
    void scheduleColumnRefresh();
    void refreshCheckedRows();
    bool isRowCheckable(const QModelIndex& sourceIndex) const;
    QStringList currentHeaderLabels() const;

private:
    // 顶部工具条
    QWidget* m_topBar = nullptr;
    QWidget* m_bottomBar = nullptr;
    QWidget* m_pageNumbers = nullptr;
    PushButton* m_firstBtn = nullptr;
    PushButton* m_lastBtn = nullptr;
    QScrollArea* m_pageScroll = nullptr;
    ComboBox* m_pageSizeBottom = nullptr;
    Label* m_pageSizeBottomLabel = nullptr;
    Label* m_totalLabel = nullptr;
    SpinBox* m_pageJumpBottom = nullptr;
    Label* m_pageJumpBottomLabel = nullptr;
    QList<PushButton*> m_pageButtons;
    QList<PushButton*> m_pageButtonPool;
    QList<Label*> m_ellipsisPool;
    Label* m_filterLabel = nullptr;
    LineEdit* m_filterEdit = nullptr;
    Label* m_columnLabel = nullptr;
    ComboBox* m_columnCombo = nullptr;
    ToolButton* m_columnsBtn = nullptr;
    QMenu* m_columnsMenu = nullptr;

    // 中部视图
    TableView* m_tableView = nullptr;

    // 底部分页与导出
    PushButton* m_prevBtn = nullptr;
    PushButton* m_nextBtn = nullptr;
    PushButton* m_exportBtn = nullptr;

    // 模型链
    QPointer<QAbstractItemModel> m_source;
    QSortFilterProxyModel* m_sortFilter = nullptr;
    PagedProxyModel* m_paged = nullptr;

    // 防抖
    QTimer* m_filterTimer = nullptr;
    QVector<QMetaObject::Connection> m_sourceConnections;
    QVector<QPersistentModelIndex> m_topPins, m_bottomPins, m_savedSelection;
    QPersistentModelIndex m_savedCurrent;
    PresentationMode m_mode = PresentationMode::Paged;
    bool m_defaultContextMenu = true;
    bool m_columnRefreshPending = false;
    bool m_rowChecksVisible = true;
    QVector<QPersistentModelIndex> m_checkedRows;

    // 选项
    Qt::CaseSensitivity m_filterCs = Qt::CaseInsensitive;
    FilterMode m_filterMode = FilterMode::Literal;
    QRegularExpression m_filterRe;
    PaginationConfig m_paginationCfg;
    QString m_exportDelimiter = ",";
};

} // namespace basewidgets

#endif // BASEWIDGETS_ADVANCEDTABLE_H

