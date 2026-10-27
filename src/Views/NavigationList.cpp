/*
 * Author: zhoutao@zkhx.com
 * Date: 2026-03-16
 */
#include <basewidgets/Views/NavigationList.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

#include <QListWidgetItem>
#include <QStyledItemDelegate>


using namespace acss;

namespace basewidgets {
    namespace {
    class NavigationDelegate : public QStyledItemDelegate {
    public:
        using QStyledItemDelegate::QStyledItemDelegate;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
            QSize size = QStyledItemDelegate::sizeHint(option,index);
            size.setHeight(qMax(32,option.fontMetrics.height()+12));
            return size;
        }
    };
    }

    NavigationList::NavigationList(QWidget *parent)
        : QListWidget(parent) {
        setAttribute(Qt::WA_StyledBackground, true);
        setAutoFillBackground(true);
        setStyleManager(StyleManagerAdapter::instance());

        setObjectName("Navigation");
        setMinimumWidth(120);
        setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding);
        setItemDelegate(new NavigationDelegate(this));
        connect(this,&QListWidget::itemClicked,this,[this](QListWidgetItem* item) {
            emit buttonClicked(m_items.key(item,-1),item->text(),row(item));
        });
        connect(model(),&QAbstractItemModel::modelAboutToBeReset,this,[this] { m_items.clear(); });
        connect(model(),&QAbstractItemModel::rowsAboutToBeRemoved,this,[this](const QModelIndex&,int first,int last) {
            for (int row=first;row<=last;++row) {
                const auto* removed = item(row);
                for (auto it=m_items.begin();it!=m_items.end();) {
                    if (it.value()==removed) it=m_items.erase(it);
                    else ++it;
                }
            }
        });
    }

    NavigationList::~NavigationList()
    {
        // QListWidget clears its model in its base destructor, after m_items is gone.
        // Disconnect the bookkeeping callbacks while the derived members still exist.
        disconnect(model(),nullptr,this,nullptr);
    }

    void NavigationList::setStyleManager(acss::QtAdvancedStylesheet *manager) {
        if (m_manager == manager) return;
        m_manager = manager;
        attachToManager();
    }

    void NavigationList::addButton(const QString &title, const int &id, const QIcon &icon) {
        if (title.isEmpty()) {
            qDebug()<<"Title is empty";
            return;
        }
        // 检查用户指定的有效ID是否已存在
        if (id != -1 && m_items.contains(id)) {
            qDebug()<<"ID already exists:"<<id;
            return;
        }

        auto *item = new QListWidgetItem(icon, title);

        if (!icon.isNull()) item->setIcon(icon);
        addItem(item); //新增按钮
        int finalId = id;
        if (id == -1) finalId = findAvailableId(); //自动分配ID
        m_items.insert(finalId, item);
    }

    void NavigationList::insertButton(const QString &title,int row, const int &id, const QIcon &icon) {
        if (title.isEmpty()) {
            qDebug()<<"Title is empty";
            return;
        }
        // 检查用户指定的有效ID是否已存在
        if (id != -1 && m_items.contains(id)) {
            qDebug()<<"Id already exists:"<<id;
            return;
        }

        if(row < 0 || row > count()) row = count();

        auto *item = new QListWidgetItem(icon, title);
        if (!icon.isNull()) item->setIcon(icon);
        insertItem(row, item); //插入按钮
        int finalId = id;
        if (id == -1) finalId = findAvailableId(); //自动分配ID
        m_items.insert(finalId, item);
    }

    void NavigationList::setCurrentButton(const int &id) {
        setCurrentItem(m_items.value(id));
    }

    int NavigationList::getCurrentButton() const {
        return m_items.key(currentItem(), -1);
    }

    void NavigationList::removeButton(const int &id) {
        auto *item = m_items.value(id,nullptr);
        if(!item) return;
        removeItemWidget(item);
        delete item;
        m_items.remove(id);
    }

    void NavigationList::attachToManager() {
        if (!m_manager) return;
        // 组件级不再连接样式变更信号；应用级统一管理
    }

    int NavigationList::findAvailableId() {
        int id = 0;
        while (m_items.contains(id)) ++id;
        return id;
    }

    void NavigationList::onStylesheetChanged() {
        // 统一在应用级进行样式更新；此处不再单独处理
    }

    void NavigationList::applyStylesheet() {
        // 清空组件级样式应用，改为应用级统一管理
    }
} // namespace basewidgets
