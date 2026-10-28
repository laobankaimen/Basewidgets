#include <basewidgets/Containers/AboutWidget.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Containers/CollapsibleSection.h>
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPainter>
#include <QSet>
#include <QPointer>
#include <QJsonArray>
#include <QFileInfo>
namespace basewidgets {
namespace {
class ProductLogo : public QWidget {
public:
    explicit ProductLogo(const QIcon& icon,QWidget* parent) : QWidget(parent),m_icon(icon) { setFixedSize(64,64); }
protected:
    void paintEvent(QPaintEvent*) override { QPainter p(this); m_icon.paint(&p,rect().adjusted(4,4,-4,-4)); }
private:
    QIcon m_icon;
};
QLabel* label(const QString& text,QWidget* parent,bool secondary=false)
{
    auto* result=new QLabel(text,parent); result->setTextFormat(Qt::PlainText); result->setWordWrap(true);
    result->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard); result->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
    if(secondary) {
        auto recolor=[result] { auto p=result->palette(); p.setColor(QPalette::WindowText,ThemeTokens::instance()->textSecondary()); result->setPalette(p); };
        recolor(); QObject::connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,result,recolor);
    }
    return result;
}
}
AboutWidget::AboutWidget(QWidget* parent) : StyledWidget(parent)
{
    m_outer=new QVBoxLayout(this); m_outer->setContentsMargins(0,0,0,0); rebuild();
}
QString AboutWidget::productName() const
{
    if(!m_info.productName.isEmpty())return m_info.productName;
    const QString name=QGuiApplication::applicationDisplayName(); return name.isEmpty()?QCoreApplication::applicationName():name;
}
QString AboutWidget::version() const { return m_info.version.isEmpty()?QCoreApplication::applicationVersion():m_info.version; }
bool AboutWidget::setInfo(const AboutInfo& info)
{
    QSet<QString> ids;
    for(const auto& link:info.links) {
        if(link.id.trimmed().isEmpty()||ids.contains(link.id)||link.text.trimmed().isEmpty()||!link.url.isValid()||link.url.isEmpty())return false;
        ids.insert(link.id);
    }
    m_info=info; rebuild(); return true;
}
bool AboutWidget::setConfiguration(const QJsonObject& config)
{
    AboutInfo info;
    auto string=[&](const QJsonObject& object,const QString& key,QString& output) {
        if(!object.contains(key))return true;
        if(!object.value(key).isString())return false;
        output=object.value(key).toString(); return true;
    };
    if(!string(config,"productName",info.productName)||!string(config,"version",info.version)||!string(config,"tagline",info.tagline)
       ||!string(config,"build",info.build)||!string(config,"organization",info.organization)||!string(config,"description",info.description)
       ||!string(config,"copyright",info.copyright)||!string(config,"license",info.license))return false;
    QString logo; if(!string(config,"logo",logo))return false;
    if(!logo.isEmpty()) { if(!QFileInfo::exists(logo))return false; info.logo=QIcon(logo); if(info.logo.isNull())return false; }
    if(config.contains("details")) {
        if(!config.value("details").isArray())return false;
        for(const auto value:config.value("details").toArray()) {
            if(!value.isObject())return false; const auto item=value.toObject(); QString name,text;
            if(!string(item,"label",name)||!string(item,"value",text))return false;
            info.details.append({name,text});
        }
    }
    if(config.contains("links")) {
        if(!config.value("links").isArray())return false;
        for(const auto value:config.value("links").toArray()) {
            if(!value.isObject())return false; const auto item=value.toObject(); AboutLink link; QString url;
            if(!string(item,"id",link.id)||!string(item,"text",link.text)||!string(item,"url",url))return false;
            link.url=QUrl(url); info.links.append(link);
        }
    }
    if(config.contains("copyButtonVisible")&&!config.value("copyButtonVisible").isBool())return false;
    // Validate before changing either the page or the copy-button preference.
    QSet<QString> ids; for(const auto& link:info.links) {
        if(link.id.trimmed().isEmpty()||ids.contains(link.id)||link.text.trimmed().isEmpty()||!link.url.isValid()||link.url.isEmpty())return false;
        ids.insert(link.id);
    }
    m_copyVisible=config.value("copyButtonVisible").toBool(true); m_info=info; rebuild(); return true;
}
QString AboutWidget::informationText() const
{
    QStringList lines{productName()};
    auto add=[&](const QString& key,const QString& value) { if(!value.isEmpty())lines.append(key+": "+value); };
    add(tr("版本"),version()); add(tr("构建"),m_info.build); add(tr("组织"),m_info.organization);
    for(const auto& pair:m_info.details)if(!pair.first.isEmpty())add(pair.first,pair.second);
    return lines.join('\n');
}
void AboutWidget::setCopyButtonVisible(bool visible) { if(m_copyVisible==visible)return; m_copyVisible=visible; rebuild(); }
void AboutWidget::rebuild()
{
    // Remove the old page immediately from layout, defer QObject destruction so a
    // link/copy handler may safely reconfigure the page that emitted its signal.
    if(m_page) { m_outer->removeWidget(m_page); m_page->hide(); m_page->deleteLater(); }
    auto* scroll=new ScrollArea(this); scroll->setFrameShape(QFrame::NoFrame); scroll->setObjectName("AboutScroll"); m_page=scroll; m_outer->addWidget(scroll);
    auto* body=new StyledWidget; auto* layout=new QVBoxLayout(body); layout->setContentsMargins(18,14,18,14); layout->setSpacing(10);
    auto* identity=new QHBoxLayout; identity->setSpacing(14);
    const QIcon icon=m_info.logo.isNull()?QApplication::windowIcon():m_info.logo;
    if(!icon.isNull())identity->addWidget(new ProductLogo(icon,body),0,Qt::AlignTop);
    auto* identityText=new QVBoxLayout;
    auto* title=label(productName(),body); title->setObjectName("AboutProductName"); QFont heading=title->font(); heading.setPointSizeF(qMax(16.,heading.pointSizeF()+5)); heading.setBold(true); title->setFont(heading); identityText->addWidget(title);
    if(!m_info.tagline.isEmpty())identityText->addWidget(label(m_info.tagline,body,true));
    identity->addLayout(identityText,1); layout->addLayout(identity);
    auto* metadata=new QFormLayout; metadata->setRowWrapPolicy(QFormLayout::WrapLongRows); metadata->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow); metadata->setContentsMargins(0,0,0,0); metadata->setVerticalSpacing(4); metadata->setHorizontalSpacing(14);
    auto row=[&](const QString& name,const QString& value) {
        if(name.isEmpty()||value.isEmpty())return;
        auto* key=label(name,body,true); key->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        metadata->addRow(key,label(value,body));
    };
    row(tr("版本"),version()); row(tr("构建"),m_info.build); row(tr("组织"),m_info.organization);
    for(const auto& pair:m_info.details)row(pair.first,pair.second); layout->addLayout(metadata);
    if(!m_info.description.isEmpty())layout->addWidget(label(m_info.description,body));
    if(!m_info.links.isEmpty()) {
        auto* links=new ResponsiveGridLayout(nullptr,120,2); links->setContentsMargins(0,0,0,0); links->setSpacing(6);
        for(const auto& link:m_info.links) {
            auto* button=new PushButton(link.text,body); button->setObjectName("AboutLink_"+link.id); button->setToolTip(link.url.toDisplayString()); button->setAppearance(ButtonAppearance::Subtle);
            links->addWidget(button); connect(button,&QPushButton::clicked,this,[this,link] { emit linkActivated(link.id,link.url); });
        }
        layout->addLayout(links);
    }
    if(!m_info.license.isEmpty()) {
        auto* license=new CollapsibleSection(tr("许可与第三方声明"),body); license->setObjectName("AboutLicense");
        license->contentLayout()->addWidget(label(m_info.license,license->contentWidget())); license->setExpanded(false); layout->addWidget(license);
    }
    if(!m_info.copyright.isEmpty())layout->addWidget(label(m_info.copyright,body,true));
    if(m_copyVisible) {
        auto* copy=new PushButton(tr("复制版本信息"),body); copy->setObjectName("AboutCopyInformation"); copy->setAppearance(ButtonAppearance::Subtle); layout->addWidget(copy,0,Qt::AlignLeading);
        connect(copy,&QPushButton::clicked,this,[this] { const QString text=informationText(); const QPointer<AboutWidget> self(this); QApplication::clipboard()->setText(text); if(self)emit informationCopied(text); });
    }
    layout->addStretch(); scroll->setWidget(body);
}
}
