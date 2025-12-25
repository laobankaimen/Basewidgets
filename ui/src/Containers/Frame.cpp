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
}

void Frame::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void Frame::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI
