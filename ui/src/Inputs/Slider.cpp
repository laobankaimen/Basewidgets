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
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &Slider::onStylesheetChanged);
    applyStylesheet();
}

void Slider::onStylesheetChanged()
{
    applyStylesheet();
}

void Slider::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI