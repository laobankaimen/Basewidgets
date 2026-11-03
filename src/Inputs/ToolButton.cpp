#include <QEvent>
#include <QTimer>
#include <QCursor>
#include <QApplication>
#include <QMenu>
/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/ToolButton.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

using namespace acss;

namespace basewidgets {

ToolButton::ToolButton(QWidget* parent)
    : QToolButton(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

ToolButton::ToolButton(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QToolButton(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ToolButton::~ToolButton() = default;

bool ToolButton::event(QEvent* event)
{
    const auto type=event->type();
    const bool result=QToolButton::event(event);
    if(type==QEvent::FocusIn || type==QEvent::FocusOut || type==QEvent::WindowActivate || type==QEvent::UngrabMouse) {
        QTimer::singleShot(0,this,[this] {
            const bool hover=isVisible() && rect().contains(mapFromGlobal(QCursor::pos()));
            setAttribute(Qt::WA_UnderMouse,hover);
            if(menu() && !menu()->isVisible() && !(QApplication::mouseButtons() & Qt::LeftButton))setDown(false);
            update();
        });
    }
    return result;
}

void ToolButton::setAppearance(ButtonAppearance appearance)
{
    // Dynamic properties also let NativeStyle render plain Qt child buttons consistently.
    setProperty("bwAppearance", static_cast<int>(appearance));
    update();
}

ButtonAppearance ToolButton::appearance() const
{
    return static_cast<ButtonAppearance>(property("bwAppearance").toInt());
}

void ToolButton::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ToolButton::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void ToolButton::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void ToolButton::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace basewidgets

