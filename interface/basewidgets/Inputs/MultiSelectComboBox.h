/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_MULTISELECTCOMBOBOX_H
#define BASEWIDGETS_MULTISELECTCOMBOBOX_H

#include <basewidgets/Inputs/ComboBox.h>
#include <basewidgets/QtAdvancedUIExport.h>
#include <QPersistentModelIndex>

namespace basewidgets {

/** 多选下拉列表；内部模型由控件持有，selectionChanged 返回选中的文本列表。
 * 鼠标/列表 Space 即时切换且保留弹层；Enter/Escape/外点关闭，保留已作选择。
 */
class ACSS_UI_EXPORT MultiSelectComboBox : public ComboBox {
    Q_OBJECT
public:
    explicit MultiSelectComboBox(QWidget* parent = nullptr);
    ~MultiSelectComboBox() override;

    void setOptions(const QStringList& options);
    QStringList options() const;

    QStringList selectedOptions() const;
    /** 程序设置匹配文本的选择；可包含禁用项，不受交互选择上限限制。
     * 每次调用发 selectionChanged，延续原有程序设置语义。
     */
    void setSelectedOptions(const QStringList& selected);

    void setJoinSelectedText(bool join, const QString& delim = ", ");
    bool joinSelectedText() const { return m_joinText; }
    QString delimiter() const { return m_delim; }

    /** 仅切换“全选”行的显示，保留现有选项、选择及元数据。 */
    void setShowSelectAll(bool show);
    bool showSelectAll() const { return m_showSelectAll; }

    // 扩展：搜索过滤、禁用项、图标、选择上限
    void setSearchEnabled(bool enabled);
    bool searchEnabled() const { return m_searchEnabled; }

    void setDisabledOptions(const QStringList& options);
    void setOptionIcons(const QHash<QString, QIcon>& icons);
    void setShowOptionIcons(bool show);
    bool showOptionIcons() const { return m_showOptionIcons; }
    /** 交互选择上限；0 表示不限制，设置上限不会截断已有选择。 */
    void setMaxSelection(int max);
    int maxSelection() const { return m_maxSelection; }

signals:
    void selectionChanged(const QStringList& selected);
    void selectionLimitReached(int limit);

protected:
    void showPopup() override;
    void hidePopup() override;
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    void initModel();
    void updateDisplayText();
    void toggleItem(int row);
    void activateItem(int row);
    void updateSelectAllState();
    void setSearchEditorActive(bool active);
    void setAllChecked(bool checked);
    void applyIconsAndDisabled();
    void filterOptions(const QString& text);

private:
    bool m_joinText = true;
    QString m_delim = ", ";
    bool m_showSelectAll = true;
    bool m_searchEnabled = false;
    QStringList m_disabledOptions;
    QHash<QString, QIcon> m_optionIcons;
    bool m_showOptionIcons = true;
    int m_maxSelection = 0;
    bool m_popupOpen = false;
    QPersistentModelIndex m_pressedIndex;
};

} // namespace basewidgets

#endif // BASEWIDGETS_MULTISELECTCOMBOBOX_H

