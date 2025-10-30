#include <QtAdvancedUI/Containers/Frame.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

Frame::Frame(QWidget* parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    setStyleManager(StyleManagerAdapter::instance());
}

Frame::Frame(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

Frame::~Frame() = default;

void Frame::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void Frame::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &Frame::onStylesheetChanged);
    applyStylesheet();
}

void Frame::onStylesheetChanged()
{
    applyStylesheet();
}

void Frame::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI