#ifndef QTADVANCEDUI_ADVANCEDTABLE_H
#define QTADVANCEDUI_ADVANCEDTABLE_H

#include <QtAdvancedUI/Core/StyledWidget.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QtAdvancedUI/Views/PagedProxyModel.h>

class QAbstractItemModel;
class QSortFilterProxyModel;
class QHeaderView;
class QMenu;
class QTimer;
class QRegExp;

namespace QtAdvancedUI {

class LineEdit;
class ComboBox;
class SpinBox;
class PushButton;
class ToolButton;
class Label;
class TableView;

class ACSS_UI_EXPORT AdvancedTable : public StyledWidget {
    Q_OBJECT
public:
    explicit AdvancedTable(QWidget* parent = nullptr);
    explicit AdvancedTable(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~AdvancedTable() override;

    void setSourceModel(QAbstractItemModel* source);
    QAbstractItemModel* sourceModel() const;

    TableView* view() const { return m_tableView; }
    PagedProxyModel* pager() const { return m_paged; }
    QSortFilterProxyModel* sortFilterProxy() const { return m_sortFilter; }
    QHeaderView* header() const;

    void setPageSize(int size);
    int pageSize() const;
    void setFilterText(const QString& text);
    void setFilterColumn(int column); // -1 表示全部列
    void setFilterCaseSensitivity(Qt::CaseSensitivity cs);
    Qt::CaseSensitivity filterCaseSensitivity() const;
    void setFilterRegExp(const QRegExp& exp);

    // 导出当前过滤结果到 CSV 文件（不含表头或含表头由参数决定）
    bool exportCsvToFile(const QString& filePath, bool includeHeader = true) const;
    void setExportDelimiter(const QString& delim);
    QString exportDelimiter() const;

    // 组件可见性控制
    void setTopBarVisible(bool visible);
    void setExportButtonVisible(bool visible);
    void setColumnVisibilityMenuVisible(bool visible);

signals:
    void filterChanged(const QString& text, int column);
    void exportCompleted(const QString& filePath, int rows);
    void exportFailed(const QString& filePath, const QString& error);
    void prepareContextMenu(QMenu* menu, const QModelIndexList& selectedRows);

protected:
    void applyStylesheet();

private:
    void initUi();
    void bindSignals();
    void rebuildColumnUi();
    QStringList currentHeaderLabels() const;

private:
    // 顶部工具条
    QWidget* m_topBar = nullptr;
    LineEdit* m_filterEdit = nullptr;
    ComboBox* m_columnCombo = nullptr;
    ComboBox* m_pageSizeCombo = nullptr;
    SpinBox* m_jumpSpin = nullptr;
    ToolButton* m_columnsBtn = nullptr;
    QMenu* m_columnsMenu = nullptr;

    // 中部视图
    TableView* m_tableView = nullptr;

    // 底部分页与导出
    Label* m_pageInfo = nullptr;
    PushButton* m_prevBtn = nullptr;
    PushButton* m_nextBtn = nullptr;
    PushButton* m_exportBtn = nullptr;

    // 模型链
    QAbstractItemModel* m_source = nullptr;
    QSortFilterProxyModel* m_sortFilter = nullptr;
    PagedProxyModel* m_paged = nullptr;

    // 防抖
    QTimer* m_filterTimer = nullptr;

    // 选项
    Qt::CaseSensitivity m_filterCs = Qt::CaseInsensitive;
    QString m_exportDelimiter = ",";
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_ADVANCEDTABLE_H