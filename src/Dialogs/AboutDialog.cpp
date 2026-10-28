#include <basewidgets/Dialogs/AboutDialog.h>
#include <basewidgets/Inputs/PushButton.h>
#include <QApplication>
#include <QVBoxLayout>
namespace basewidgets {
AboutDialog::AboutDialog(QWidget* parent) : FramelessDialog(false,parent)
{
    setTitle(tr("关于")); auto* body=new QWidget; auto* layout=new QVBoxLayout(body); layout->setContentsMargins(0,0,0,12);
    m_about=new AboutWidget(body); layout->addWidget(m_about,1);
    auto* close=new PushButton(tr("关闭"),body); close->setObjectName("AboutClose"); layout->addWidget(close,0,Qt::AlignHCenter);
    connect(close,&QPushButton::clicked,this,&QDialog::accept); setContentWidget(body); setResizeable(false); resize(540,580);
}
void AboutDialog::reject()
{
    // FramelessDialog(false) only emits onDialogClosing by default. About has
    // no pending edits: Escape, the title-bar X and native close should finish
    // normally, emitting rejected/finished and honoring WA_DeleteOnClose.
    QDialog::reject();
}
bool AboutDialog::setConfiguration(const QJsonObject& config)
{
    if(!m_about->setConfiguration(config))return false;
    const auto info=m_about->info(); const QString name=info.productName.isEmpty()?QGuiApplication::applicationDisplayName():info.productName;
    setTitle(name.isEmpty()?tr("关于"):tr("关于 %1").arg(name)); return true;
}
bool AboutDialog::setInfo(const AboutInfo& info)
{
    if(!m_about->setInfo(info))return false;
    const QString name=info.productName.isEmpty()?QGuiApplication::applicationDisplayName():info.productName;
    setTitle(name.isEmpty()?tr("关于"):tr("关于 %1").arg(name)); return true;
}
}
