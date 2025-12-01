#include <QtAdvancedUI/Inputs/MultiSelectComboBox.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

#include <QStandardItemModel>
#include <QListView>
#include <QLineEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>

using namespace acss;

namespace QtAdvancedUI {

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
    if (lineEdit()) {
        lineEdit()->setReadOnly(true);
        lineEdit()->installEventFilter(this); // 支持双击与键盘输入进入搜索
    }
    setInsertPolicy(QComboBox::NoInsert);
    setStyleManager(StyleManagerAdapter::instance());
    initModel();
    // Toggle item check state when pressed；保持弹层不关闭
    QObject::connect(view(), &QAbstractItemView::pressed, [this](const QModelIndex& idx){
        toggleItem(idx.row());
        // 弹层开启期间不刷新文本，避免搜索框被覆盖
        if (!m_popupOpen) updateDisplayText();
        emit selectionChanged(selectedOptions());
    });
    // 防止选择行为改变当前索引导致关闭：禁用选择、禁用编辑
    if (view()) {
        view()->setSelectionMode(QAbstractItemView::NoSelection);
        view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
        if (view()->viewport()) view()->viewport()->installEventFilter(this);
    }
}

MultiSelectComboBox::MultiSelectComboBox(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : ComboBox(manager, parent)
{
    setEditable(true);
    if (lineEdit()) {
        lineEdit()->setReadOnly(true);
        lineEdit()->installEventFilter(this);
    }
    setInsertPolicy(QComboBox::NoInsert);
    initModel();
    QObject::connect(view(), &QAbstractItemView::pressed, [this](const QModelIndex& idx){
        toggleItem(idx.row());
        if (!m_popupOpen) updateDisplayText();
        emit selectionChanged(selectedOptions());
    });
    if (view()) {
        view()->setSelectionMode(QAbstractItemView::NoSelection);
        view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
        if (view()->viewport()) view()->viewport()->installEventFilter(this);
    }
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
    int startRow = 0;
    if (m_showSelectAll) {
        m->removeRows(1, m->rowCount() - 1);
        startRow = 1;
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
    if (m_showSelectAll) {
        // update Select All state
        bool allChecked = true;
        for (int r = 1; r < m->rowCount(); ++r) allChecked &= (m->item(r)->checkState() == Qt::Checked);
        if (m->rowCount() > 0) m->item(0)->setCheckState(allChecked ? Qt::Checked : Qt::Unchecked);
    }
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
    m_showSelectAll = show;
    initModel();
}

void MultiSelectComboBox::toggleItem(int row)
{
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!m || row < 0 || row >= m->rowCount()) return;
    QStandardItem* it = m->item(row);
    if (!it->isCheckable()) return;
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
            emit selectionLimitReached(m_maxSelection);
        } else {
            setAllChecked(target);
        }
        // 根据实际是否全部（非禁用项）被选中来设置“全选”的勾选状态
        bool allChecked = true;
        for (int r = 1; r < m->rowCount(); ++r) {
            QStandardItem* child = m->item(r);
            if (m_disabledOptions.contains(child->text())) continue;
            allChecked &= (child->checkState() == Qt::Checked);
        }
        it->setCheckState(allChecked ? Qt::Checked : Qt::Unchecked);
        // 取消全选时，若开启搜索则显示所有行
        if (m_searchEnabled && !target) filterOptions("");
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
    if (m_showSelectAll && m->rowCount() > 1) {
        bool allChecked = true;
        for (int r = 1; r < m->rowCount(); ++r) allChecked &= (m->item(r)->checkState() == Qt::Checked);
        m->item(0)->setCheckState(allChecked ? Qt::Checked : Qt::Unchecked);
    }
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
        if (m_disabledOptions.contains(it->text())) {
            it->setFlags((it->flags() & ~Qt::ItemIsEnabled) | Qt::ItemIsSelectable);
        }
    }
}

void MultiSelectComboBox::filterOptions(const QString& text)
{
    auto* listView = qobject_cast<QListView*>(view());
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!listView || !m) return;
    const int startRow = (m_showSelectAll ? 1 : 0);
    int matched = 0;
    for (int r = 0; r < m->rowCount(); ++r) {
        if (m_showSelectAll && r == 0) { continue; }
        const QString t = m->item(r)->text();
        const bool visible = t.contains(text, Qt::CaseInsensitive);
        listView->setRowHidden(r, !visible);
        if (visible) ++matched;
    }
    // 仅在未输入搜索内容时显示“全选”；搜索期间隐藏
    if (m_showSelectAll && m->rowCount() > 0) {
        listView->setRowHidden(0, !text.isEmpty());
        QStandardItem* it0 = m->item(0);
        if (it0) {
            it0->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        }
    }
}

void MultiSelectComboBox::setSearchEnabled(bool enabled)
{
    m_searchEnabled = enabled;
    // 由 showPopup/hidePopup 控制只读状态，避免关闭弹层时可编辑造成误解
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

void MultiSelectComboBox::showPopup()
{
    ComboBox::showPopup();
    m_popupOpen = true;
    if (m_searchEnabled && lineEdit()) {
        lineEdit()->setReadOnly(false); // 弹层期间允许输入以进行搜索
        lineEdit()->clear();
        lineEdit()->setPlaceholderText(tr("搜索..."));
        lineEdit()->setFocus(Qt::OtherFocusReason);
        QTimer::singleShot(0, this, [this]{ if (lineEdit()) lineEdit()->setFocus(Qt::OtherFocusReason); });
        QObject::connect(lineEdit(), &QLineEdit::textChanged, this, &MultiSelectComboBox::filterOptions);
    }
    if (view() && model()) {
        int startRow = (m_showSelectAll ? 1 : 0);
        if (model()->rowCount() > startRow) {
            QModelIndex idx = model()->index(startRow, 0);
            view()->setCurrentIndex(idx);
        }
    }
}

void MultiSelectComboBox::hidePopup()
{
    if (m_searchEnabled && lineEdit()) {
        QObject::disconnect(lineEdit(), &QLineEdit::textChanged, this, &MultiSelectComboBox::filterOptions);
        // 取消过滤并恢复文本
        filterOptions("");
        lineEdit()->setReadOnly(true);
        lineEdit()->setPlaceholderText("");
        updateDisplayText();
    }
    m_popupOpen = false;
    ComboBox::hidePopup();
}

bool MultiSelectComboBox::eventFilter(QObject* obj, QEvent* e)
{
    // 双击输入框进入搜索模式；键盘输入即进入搜索
    if (lineEdit() && obj == lineEdit()) {
        if (e->type() == QEvent::MouseButtonDblClick) {
            if (m_searchEnabled) {
                if (!m_popupOpen) showPopup();
                return true; // 消费双击，避免默认行为
            }
        } else if (e->type() == QEvent::KeyPress) {
            if (m_searchEnabled) {
                auto* ke = static_cast<QKeyEvent*>(e);
                // Esc 直接退出搜索并关闭弹层
                if (ke->key() == Qt::Key_Escape) {
                    if (m_popupOpen) hidePopup();
                    return true;
                }
                // 打开弹层并允许编辑，交由 QLineEdit 默认处理插入文本
                if (!m_popupOpen) showPopup();
                if (lineEdit()->isReadOnly()) lineEdit()->setReadOnly(false);
                return false; // 让 QLineEdit 自身处理字符输入与 textChanged 触发
            }
        }
    }
    // 阻止点击列表项导致 QComboBox 选择并关闭弹层
    if (view() && obj == view()->viewport()) {
        switch (e->type()) {
        case QEvent::MouseButtonRelease:
            return true; // 阻止默认在释放时触发的关闭逻辑
        case QEvent::MouseButtonDblClick:
            return true; // 消费事件，不让默认处理关闭弹层
        case QEvent::KeyPress: {
            auto* ke = static_cast<QKeyEvent*>(e);
            if (ke->key() == Qt::Key_Space) {
                QModelIndex idx = view()->currentIndex();
                if (idx.isValid()) toggleItem(idx.row());
                return true;
            } else if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
                if (m_popupOpen) hidePopup();
                return true;
            } else if (ke->key() == Qt::Key_Escape) {
                if (m_popupOpen) hidePopup();
                return true;
            }
            break;
        }
        default:
            break;
        }
    }
    return ComboBox::eventFilter(obj, e);
}

} // namespace QtAdvancedUI
