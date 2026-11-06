/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/MultiSelectComboBox.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

#include <QStandardItemModel>
#include <QListView>
#include <QLineEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QMouseEvent>
#include <QPointer>
#include <QSignalBlocker>
#include <QCoreApplication>

using namespace acss;

namespace basewidgets {

static QStandardItemModel* ensureModel(QComboBox* cb)
{
    auto* m = qobject_cast<QStandardItemModel*>(cb->model());
    if (!m) {
        m = new QStandardItemModel(cb);
        cb->setModel(m);
    }
    return m;
}

MultiSelectComboBox::MultiSelectComboBox(QWidget* parent)
    : ComboBox(parent)
{
    setEditable(true);
    setSearchEnabled(true);
    if (lineEdit()) {
        lineEdit()->setReadOnly(true);
        lineEdit()->installEventFilter(this); // 支持双击与键盘输入进入搜索
    }
    setInsertPolicy(QComboBox::NoInsert);
    setStyleManager(StyleManagerAdapter::instance());
    initModel();
    view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view()->installEventFilter(this);
    view()->viewport()->installEventFilter(this);
}

MultiSelectComboBox::~MultiSelectComboBox() = default;

void MultiSelectComboBox::initModel()
{
    auto* m = ensureModel(this);
    m->clear();
    if (m_showSelectAll) {
        QStandardItem* all = new QStandardItem(tr("全选"));
        all->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        all->setCheckable(true);
        all->setCheckState(Qt::Unchecked);
        m->appendRow(all);
    }
    updateDisplayText();
}

void MultiSelectComboBox::setOptions(const QStringList& options)
{
    auto* m = ensureModel(this);
    // preserve select-all at row 0 if enabled
    if (m_showSelectAll) {
        m->removeRows(1, m->rowCount() - 1);
    } else {
        m->clear();
    }
    for (const QString& opt : options) {
        QStandardItem* it = new QStandardItem(opt);
        it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        it->setCheckable(true);
        it->setCheckState(Qt::Unchecked);
        m->appendRow(it);
    }
    applyIconsAndDisabled();
    updateDisplayText();
}

QStringList MultiSelectComboBox::options() const
{
    QStringList out;
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!m) return out;
    int startRow = (m_showSelectAll ? 1 : 0);
    for (int r = startRow; r < m->rowCount(); ++r) out << m->item(r)->text();
    return out;
}

QStringList MultiSelectComboBox::selectedOptions() const
{
    QStringList out;
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!m) return out;
    int startRow = (m_showSelectAll ? 1 : 0);
    for (int r = startRow; r < m->rowCount(); ++r) {
        QStandardItem* it = m->item(r);
        if (it->checkState() == Qt::Checked) out << it->text();
    }
    return out;
}

void MultiSelectComboBox::setSelectedOptions(const QStringList& selected)
{
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!m) return;
    int startRow = (m_showSelectAll ? 1 : 0);
    for (int r = startRow; r < m->rowCount(); ++r) {
        QStandardItem* it = m->item(r);
        it->setCheckState(selected.contains(it->text()) ? Qt::Checked : Qt::Unchecked);
    }
    updateSelectAllState();
    updateDisplayText();
    emit selectionChanged(selectedOptions());
}

void MultiSelectComboBox::setJoinSelectedText(bool join, const QString& delim)
{
    m_joinText = join;
    m_delim = delim.isEmpty() ? ", " : delim;
    updateDisplayText();
}

void MultiSelectComboBox::setShowSelectAll(bool show)
{
    if (m_showSelectAll == show) return;
    auto* m = ensureModel(this);
    const QString query = lineEdit() ? lineEdit()->text() : QString();
    m_showSelectAll = show;
    // Only alter the special row; retained items keep their data, icons and checks.
    {
        const QSignalBlocker blocker(lineEdit());
        if (show) {
            auto* all = new QStandardItem(tr("全选"));
            all->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
            all->setCheckable(true);
            m->insertRow(0, all);
            updateSelectAllState();
        } else if (m->rowCount() > 0) {
            m->removeRow(0);
        }
        if (m_popupOpen && m_searchEnabled && lineEdit()) lineEdit()->setText(query);
        else updateDisplayText();
    }
    filterOptions(m_popupOpen && m_searchEnabled ? query : QString());
}

void MultiSelectComboBox::updateSelectAllState()
{
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!m || !m_showSelectAll || m->rowCount() == 0) return;
    bool anyEnabled = false;
    bool allChecked = true;
    for (int r = 1; r < m->rowCount(); ++r) {
        auto* item = m->item(r);
        if (!item || !(item->flags() & Qt::ItemIsEnabled)) continue;
        anyEnabled = true;
        allChecked &= item->checkState() == Qt::Checked;
    }
    m->item(0)->setCheckState(anyEnabled && allChecked ? Qt::Checked : Qt::Unchecked);
}

void MultiSelectComboBox::activateItem(int row)
{
    const QStringList before = selectedOptions();
    const QPointer<MultiSelectComboBox> guard(this);
    toggleItem(row);
    if (!guard) return; // A selectionLimitReached receiver may destroy the widget.
    const QStringList after = selectedOptions();
    if (!m_popupOpen || !m_searchEnabled) updateDisplayText();
    if (after != before) emit selectionChanged(after);
}

void MultiSelectComboBox::toggleItem(int row)
{
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!m || row < 0 || row >= m->rowCount()) return;
    QStandardItem* it = m->item(row);
    if (!it || !it->isCheckable() || !(it->flags() & Qt::ItemIsEnabled)) return;
    // 禁用项不允许切换
    if (m_disabledOptions.contains(it->text())) return;
    // Select All row
    if (m_showSelectAll && row == 0) {
        bool target = it->checkState() != Qt::Checked;
        if (target && m_maxSelection > 0) {
            // 限制最多选择 m_maxSelection 项，仅勾选前 N 个（排除禁用项）
            int selected = 0;
            for (int r = 1; r < m->rowCount(); ++r) {
                QStandardItem* child = m->item(r);
                if (m_disabledOptions.contains(child->text())) {
                    continue; // 跳过禁用项
                }
                if (selected < m_maxSelection) {
                    child->setCheckState(Qt::Checked);
                    ++selected;
                } else {
                    child->setCheckState(Qt::Unchecked);
                }
            }
        } else {
            setAllChecked(target);
        }
        updateSelectAllState();
        // 取消全选时，若开启搜索则显示所有行
        if (m_searchEnabled && !target) filterOptions("");
        if (target && m_maxSelection > 0) emit selectionLimitReached(m_maxSelection);
        return;
    }
    // 选择上限控制
    if (it->checkState() == Qt::Unchecked) {
        if (m_maxSelection > 0 && selectedOptions().size() >= m_maxSelection) {
            emit selectionLimitReached(m_maxSelection);
            return; // 不允许超过上限
        }
        it->setCheckState(Qt::Checked);
    } else {
        it->setCheckState(Qt::Unchecked);
    }
    updateSelectAllState();
}

void MultiSelectComboBox::setAllChecked(bool checked)
{
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!m) return;
    int startRow = (m_showSelectAll ? 1 : 0);
    for (int r = startRow; r < m->rowCount(); ++r) {
        QStandardItem* it = m->item(r);
        if (m_disabledOptions.contains(it->text())) continue;
        it->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
}

void MultiSelectComboBox::updateDisplayText()
{
    if (!lineEdit()) return;
    if (!m_joinText) {
        lineEdit()->setText(tr("已选择 %1 项").arg(selectedOptions().size()));
        return;
    }
    const QStringList sel = selectedOptions();
    lineEdit()->setText(sel.isEmpty() ? tr("未选择") : sel.join(m_delim));
}

void MultiSelectComboBox::applyIconsAndDisabled()
{
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!m) return;
    int startRow = (m_showSelectAll ? 1 : 0);
    for (int r = startRow; r < m->rowCount(); ++r) {
        QStandardItem* it = m->item(r);
        if (m_showOptionIcons && m_optionIcons.contains(it->text())) {
            it->setIcon(m_optionIcons.value(it->text()));
        } else {
            it->setIcon(QIcon()); // 纯文字展示时清空图标
        }
        it->setEnabled(!m_disabledOptions.contains(it->text()));
    }
    updateSelectAllState();
    filterOptions(m_popupOpen && m_searchEnabled && lineEdit() ? lineEdit()->text() : QString());
}

void MultiSelectComboBox::filterOptions(const QString& text)
{
    auto* listView = qobject_cast<QListView*>(view());
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!listView || !m) return;
    for (int r = 0; r < m->rowCount(); ++r) {
        if (m_showSelectAll && r == 0) { continue; }
        const QString t = m->item(r)->text();
        const bool visible = t.contains(text, Qt::CaseInsensitive);
        listView->setRowHidden(r, !visible);
    }
    // 仅在未输入搜索内容时显示“全选”；搜索期间隐藏
    if (m_showSelectAll && m->rowCount() > 0) {
        listView->setRowHidden(0, !text.isEmpty());
        QStandardItem* it0 = m->item(0);
        if (it0) {
            it0->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        }
    }
    // A hidden current row must never remain the target of a subsequent Space.
    // The special "select all" row is available by navigation, not the default target.
    QModelIndex current;
    for (int r = m_showSelectAll ? 1 : 0; r < m->rowCount(); ++r) {
        const QModelIndex index = m->index(r, 0);
        if (!listView->isRowHidden(r) && (index.flags() & Qt::ItemIsEnabled)) {
            current = index;
            break;
        }
    }
    listView->setCurrentIndex(current);
}

void MultiSelectComboBox::setSearchEnabled(bool enabled)
{
    if (m_searchEnabled == enabled) return;
    m_searchEnabled = enabled;
    if (m_popupOpen) setSearchEditorActive(enabled);
}

void MultiSelectComboBox::setDisabledOptions(const QStringList& options)
{
    m_disabledOptions = options;
    applyIconsAndDisabled();
}

void MultiSelectComboBox::setOptionIcons(const QHash<QString, QIcon>& icons)
{
    m_optionIcons = icons;
    applyIconsAndDisabled();
}

void MultiSelectComboBox::setShowOptionIcons(bool show)
{
    m_showOptionIcons = show;
    applyIconsAndDisabled();
}

void MultiSelectComboBox::setMaxSelection(int max)
{
    m_maxSelection = qMax(0, max);
}

void MultiSelectComboBox::setSearchEditorActive(bool active)
{
    if (!lineEdit()) return;
    QObject::disconnect(lineEdit(), &QLineEdit::textChanged, this, &MultiSelectComboBox::filterOptions);
    const QSignalBlocker blocker(lineEdit());
    lineEdit()->setReadOnly(!active);
    lineEdit()->setPlaceholderText(active ? tr("搜索...") : QString());
    filterOptions(QString());
    if (active) {
        lineEdit()->clear();
        QObject::connect(lineEdit(), &QLineEdit::textChanged, this, &MultiSelectComboBox::filterOptions, Qt::UniqueConnection);
    } else {
        updateDisplayText();
    }
}

void MultiSelectComboBox::showPopup()
{
    if (m_popupOpen) return;
    ComboBox::showPopup();
    m_popupOpen = view() && view()->isVisible();
    if (!m_popupOpen) return;
    m_pressedIndex = QPersistentModelIndex();
    // Install last so our multi-selection behavior precedes QComboBox's close-on-release filter.
    view()->installEventFilter(this);
    view()->viewport()->installEventFilter(this);
    setSearchEditorActive(m_searchEnabled);
    if (m_searchEnabled && lineEdit()) {
        lineEdit()->setFocus(Qt::OtherFocusReason);
        QTimer::singleShot(0, this, [this] {
            if (m_popupOpen && m_searchEnabled && lineEdit()) lineEdit()->setFocus(Qt::OtherFocusReason);
        });
    }
}

void MultiSelectComboBox::hidePopup()
{
    m_popupOpen = false;
    m_pressedIndex = QPersistentModelIndex();
    // Cleanup must also run if search was disabled while the popup was open.
    setSearchEditorActive(false);
    ComboBox::hidePopup();
}

bool MultiSelectComboBox::eventFilter(QObject* obj, QEvent* e)
{
    if (lineEdit() && obj == lineEdit()) {
        if (e->type() == QEvent::MouseButtonDblClick && m_searchEnabled) {
            if (!m_popupOpen) showPopup();
            return true;
        }
        if (e->type() == QEvent::KeyPress) {
            auto* key = static_cast<QKeyEvent*>(e);
            if (m_popupOpen && (key->key() == Qt::Key_Escape || key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter)) {
                hidePopup();
                return true;
            }
            if (m_popupOpen && (key->key() == Qt::Key_Down || key->key() == Qt::Key_Up)) {
                view()->setFocus(Qt::OtherFocusReason);
                QKeyEvent forwarded(key->type(), key->key(), key->modifiers(), key->text(), key->isAutoRepeat(), key->count());
                QCoreApplication::sendEvent(view(), &forwarded);
                return true;
            }
            const bool printable = !key->text().isEmpty() && key->text().at(0).isPrint();
            if (m_searchEnabled && !m_popupOpen && printable && !(key->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
                showPopup();
                return false; // Let QLineEdit insert the first search character.
            }
        }
    }
    if (view() && (obj == view() || obj == view()->viewport())) {
        if (e->type() == QEvent::KeyPress) {
            auto* key = static_cast<QKeyEvent*>(e);
            if (key->key() == Qt::Key_Space) {
                if (!key->isAutoRepeat()) {
                    const QModelIndex index = view()->currentIndex();
                    if (index.isValid()) activateItem(index.row());
                }
                return true;
            }
            if (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter || key->key() == Qt::Key_Escape) {
                hidePopup();
                return true;
            }
        }
        if (obj == view()->viewport()) {
            if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick) {
                auto* mouse = static_cast<QMouseEvent*>(e);
                if (mouse->button() == Qt::LeftButton) {
                    m_pressedIndex = view()->indexAt(mouse->pos());
                    if (m_pressedIndex.isValid() && (m_pressedIndex.flags() & Qt::ItemIsEnabled)) view()->setCurrentIndex(m_pressedIndex);
                    return true;
                }
            } else if (e->type() == QEvent::MouseButtonRelease) {
                auto* mouse = static_cast<QMouseEvent*>(e);
                if (mouse->button() == Qt::LeftButton) {
                    const QModelIndex index = view()->indexAt(mouse->pos());
                    const bool activate = index.isValid() && index == m_pressedIndex;
                    m_pressedIndex = QPersistentModelIndex();
                    if (activate) activateItem(index.row());
                    return true;
                }
            }
        }
    }
    return ComboBox::eventFilter(obj, e);
}

} // namespace basewidgets

