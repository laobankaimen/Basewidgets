#ifndef QTADVANCEDUI_MULTISELECTCOMBOBOX_H
#define QTADVANCEDUI_MULTISELECTCOMBOBOX_H

#include <QtAdvancedUI/Inputs/ComboBox.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT MultiSelectComboBox : public ComboBox {
    Q_OBJECT
public:
    explicit MultiSelectComboBox(QWidget* parent = nullptr);
    explicit MultiSelectComboBox(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~MultiSelectComboBox() override;

    void setOptions(const QStringList& options);
    QStringList options() const;

    QStringList selectedOptions() const;
    void setSelectedOptions(const QStringList& selected);

    void setJoinSelectedText(bool join, const QString& delim = ", ");
    bool joinSelectedText() const { return m_joinText; }
    QString delimiter() const { return m_delim; }

    void setShowSelectAll(bool show);
    bool showSelectAll() const { return m_showSelectAll; }

    // 扩展：搜索过滤、禁用项、图标、选择上限
    void setSearchEnabled(bool enabled);
    bool searchEnabled() const { return m_searchEnabled; }

    void setDisabledOptions(const QStringList& options);
    void setOptionIcons(const QHash<QString, QIcon>& icons);
    void setShowOptionIcons(bool show);
    bool showOptionIcons() const { return m_showOptionIcons; }
    void setMaxSelection(int max); // 0 表示不限制
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
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_MULTISELECTCOMBOBOX_H