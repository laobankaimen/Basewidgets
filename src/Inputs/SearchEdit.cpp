#include <basewidgets/Inputs/SearchEdit.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QAction>
#include <QTimer>
#include <QKeyEvent>
namespace basewidgets {
SearchEdit::SearchEdit(QWidget* parent) : LineEdit(parent)
{
    setPlaceholderText(tr("搜索…")); setAccessibleName(tr("搜索")); setClearButtonEnabled(true);
    m_searchAction=addAction(QIcon(),QLineEdit::LeadingPosition); m_searchAction->setText(tr("搜索")); m_searchAction->setToolTip(tr("立即搜索"));
    m_timer=new QTimer(this); m_timer->setSingleShot(true); m_timer->setInterval(300);
    connect(this,&QLineEdit::textChanged,this,[this] { m_timer->stop(); if(isEnabled()&&hasAcceptableInput())m_timer->start(); });
    connect(m_timer,&QTimer::timeout,this,&SearchEdit::requestSearch);
    connect(this,&QLineEdit::returnPressed,this,&SearchEdit::requestSearch);
    connect(m_searchAction,&QAction::triggered,this,&SearchEdit::requestSearch);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,&SearchEdit::updateIcon); updateIcon();
}
void SearchEdit::updateIcon() { m_searchAction->setIcon(StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/titlebar/search.svg")); }
bool SearchEdit::setDebounceInterval(int milliseconds)
{
    if(milliseconds<0||milliseconds>60000)return false;
    m_timer->setInterval(milliseconds); return true;
}
int SearchEdit::debounceInterval() const { return m_timer->interval(); }
void SearchEdit::cancelPendingSearch() { m_timer->stop(); }
void SearchEdit::requestSearch()
{
    m_timer->stop(); if(!isEnabled()||!hasAcceptableInput())return;
    const QString query=text(); emit searchRequested(query);
}
void SearchEdit::keyPressEvent(QKeyEvent* event)
{
    if(event->key()==Qt::Key_Escape&&!isReadOnly()&&!text().isEmpty()) { clear(); event->accept(); return; }
    LineEdit::keyPressEvent(event);
}
void SearchEdit::changeEvent(QEvent* event)
{
    LineEdit::changeEvent(event);
    if(event->type()==QEvent::EnabledChange && m_timer && !isEnabled())m_timer->stop();
}
}
