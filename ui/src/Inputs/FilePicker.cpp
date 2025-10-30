#include <QtAdvancedUI/Inputs/FilePicker.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QtAdvancedUI/Dialogs/DialogUtils.h>
#include <QHBoxLayout>

using namespace acss;

namespace QtAdvancedUI {

FilePicker::FilePicker(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());

    m_edit = new LineEdit(this);
    m_button = new ToolButton(this);
    m_button->setText("浏览...");

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_edit, 1);
    lay->addWidget(m_button);
    setLayout(lay);

    connect(m_button, &ToolButton::clicked, this, &FilePicker::browse);
    connect(m_edit, &LineEdit::textChanged, this, &FilePicker::pathChanged);
}

FilePicker::FilePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());

    m_edit = new LineEdit(this);
    m_button = new ToolButton(this);
    m_button->setText("浏览...");

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_edit, 1);
    lay->addWidget(m_button);
    setLayout(lay);

    connect(m_button, &ToolButton::clicked, this, &FilePicker::browse);
    connect(m_edit, &LineEdit::textChanged, this, &FilePicker::pathChanged);
}

FilePicker::~FilePicker() = default;

void FilePicker::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void FilePicker::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &FilePicker::onStylesheetChanged);
    applyStylesheet();
}

void FilePicker::onStylesheetChanged()
{
    applyStylesheet();
}

void FilePicker::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

void FilePicker::setMode(Mode m) { m_mode = m; }
void FilePicker::setTitle(const QString& title) { m_title = title; }
void FilePicker::setFilter(const QString& filter) { m_filter = filter; }
void FilePicker::setInitialDir(const QString& dir) { m_initialDir = dir; }
void FilePicker::setPath(const QString& path) { m_edit->setText(path); }
QString FilePicker::path() const { return m_edit->text(); }

void FilePicker::browse()
{
    QString result;
    switch (m_mode) {
    case OpenFile:
        result = DialogUtils::openFile(this, m_title, m_initialDir, m_filter);
        break;
    case SaveFile:
        result = DialogUtils::saveFile(this, m_title, m_initialDir, m_filter);
        break;
    case OpenDirectory:
        result = DialogUtils::openDirectory(this, m_title, m_initialDir);
        break;
    }
    if (!result.isEmpty()) {
        m_edit->setText(result);
        emit pathChanged(result);
    }
}

} // namespace QtAdvancedUI