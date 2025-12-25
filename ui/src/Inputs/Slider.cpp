#include <QtAdvancedUI/Inputs/Slider.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

Slider::Slider(Qt::Orientation orientation, QWidget* parent)
    : QSlider(orientation, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

Slider::Slider(acss::QtAdvancedStylesheet* manager, Qt::Orientation orientation, QWidget* parent)
    : QSlider(orientation, parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

Slider::~Slider() = default;

void Slider::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void Slider::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void Slider::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void Slider::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI
