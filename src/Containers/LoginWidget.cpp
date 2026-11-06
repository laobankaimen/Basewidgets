#include <basewidgets/Containers/LoginWidget.h>
#include <basewidgets/Inputs/LineEdit.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QVBoxLayout>
#include <QLabel>
namespace basewidgets {
LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent)
{
    setObjectName("LoginWidget");
    auto* layout=new QVBoxLayout(this); layout->setContentsMargins(24,24,24,24); layout->setSpacing(10);
    auto* title=new QLabel(tr("登录工作区"),this); QFont heading=title->font(); heading.setBold(true); heading.setPointSize(heading.pointSize()+3); title->setFont(heading); layout->addWidget(title);
    auto* hint=new QLabel(tr("使用工作区账号登录。"),this); hint->setWordWrap(true); layout->addWidget(hint);
    auto* accountLabel=new QLabel(tr("账号(&A)"),this); m_account=new LineEdit(this); m_account->setObjectName("LoginAccount"); m_account->setAccessibleName(tr("账号")); accountLabel->setBuddy(m_account); layout->addWidget(accountLabel); layout->addWidget(m_account);
    auto* passwordLabel=new QLabel(tr("密码(&P)"),this); m_password=new LineEdit(this); m_password->setObjectName("LoginPassword"); m_password->setAccessibleName(tr("密码")); m_password->setEchoMode(QLineEdit::Password); passwordLabel->setBuddy(m_password); layout->addWidget(passwordLabel); layout->addWidget(m_password);
    m_reveal=new CheckBox(tr("显示密码"),this); m_reveal->setObjectName("LoginReveal"); layout->addWidget(m_reveal);
    m_error=new QLabel(this); m_error->setObjectName("LoginError"); m_error->setTextFormat(Qt::PlainText); m_error->setWordWrap(true); m_error->hide(); layout->addWidget(m_error);
    auto recolor=[this] { auto p=m_error->palette(); p.setColor(QPalette::WindowText,ThemeTokens::instance()->danger()); m_error->setPalette(p); };
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,recolor); recolor();
    auto* submit=new PushButton(tr("登录"),this); submit->setObjectName("LoginSubmit"); submit->setAppearance(ButtonAppearance::Primary); m_submit=submit; layout->addWidget(submit); layout->addStretch();
    connect(m_reveal,&QCheckBox::toggled,this,[this](bool show) { m_password->setEchoMode(show?QLineEdit::Normal:QLineEdit::Password); });
    connect(m_account,&QLineEdit::returnPressed,this,[this] { m_password->setFocus(); });
    connect(m_password,&QLineEdit::returnPressed,this,&LoginWidget::submit);
    connect(m_submit,&QPushButton::clicked,this,&LoginWidget::submit);
    setFocusProxy(m_account);
}
void LoginWidget::setAccount(const QString& account) { m_account->setText(account); }
QString LoginWidget::account() const { return m_account->text().trimmed(); }
void LoginWidget::setBusy(bool busy)
{
    m_busy=busy; m_account->setEnabled(!busy); m_password->setEnabled(!busy); m_reveal->setEnabled(!busy); m_submit->setEnabled(!busy); m_submit->setText(busy?tr("正在登录…"):tr("登录"));
    if(busy)m_reveal->setChecked(false);
}
void LoginWidget::setError(const QString& message) { showError(message,m_password); }
void LoginWidget::showError(const QString& message,QLineEdit* field)
{
    setBusy(false); m_error->setText(message); m_error->setVisible(!message.isEmpty());
    m_account->setAccessibleDescription(QString());
    m_password->setAccessibleDescription(QString());
    if(!message.isEmpty()) { field->setAccessibleDescription(message); field->setFocus(); field->selectAll(); }
}
void LoginWidget::clearPassword() { m_reveal->setChecked(false); m_password->clear(); }
void LoginWidget::submit()
{
    if(m_busy)return;
    if(account().isEmpty()) { showError(tr("请输入账号。"),m_account); return; }
    if(m_password->text().isEmpty()) { setError(tr("请输入密码。")); return; }
    setError(QString()); setBusy(true);
    emit authenticationRequested(account(),m_password->text());
}
}
