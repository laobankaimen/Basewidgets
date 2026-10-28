#pragma once
#include <basewidgets/Core/StyledWidget.h>
#include <QIcon>
#include <QUrl>
#include <QVector>
#include <QPair>
#include <QJsonObject>
class QVBoxLayout;
namespace basewidgets {
/** 关于页链接只发出信号，由应用决定打开网页、文档或业务页面。 */
struct ACSS_UI_EXPORT AboutLink { QString id,text; QUrl url; };
struct ACSS_UI_EXPORT AboutInfo {
    QString productName,version,tagline,build,organization,description,copyright,license;
    QIcon logo;
    QVector<QPair<QString,QString>> details; ///< 自定义键值元信息；空键或空值不展示。
    QVector<AboutLink> links; ///< id 必须唯一、文字非空、URL 有效且非空。
};
/** 通用可嵌入关于页：纯文本、可选 Logo、版本/构建、元信息、许可和链接。
 * productName/version/logo 为空时沿用应用默认；其余空字段隐藏。
 * setInfo 原子验证链接并重建展示，info() 保留传入配置；GUI 线程调用。
 */
class ACSS_UI_EXPORT AboutWidget : public StyledWidget {
    Q_OBJECT
public:
    explicit AboutWidget(QWidget* parent=nullptr);
    bool setInfo(const AboutInfo& info);
    /** 完整 JSON 快照：字符串字段与 AboutInfo 同名，logo 为本地/qrc 路径；
     * details=[{label,value}]，links=[{id,text,url}]，copyButtonVisible 为可选 bool。
     * 缺项采用默认；类型/Logo/链接错误时返回 false，保持当前配置，不读网络 URL。 */
    bool setConfiguration(const QJsonObject& configuration);
    AboutInfo info() const { return m_info; }
    QString informationText() const; ///< 可复制的产品/版本/元信息，不包含许可全文或 URL。
    void setCopyButtonVisible(bool visible); ///< 默认显示复制版本信息按钮。
    bool isCopyButtonVisible() const { return m_copyVisible; }
    QSize sizeHint() const override { return QSize(480,480); }
signals:
    void linkActivated(const QString& id,const QUrl& url); ///< 不自动启动浏览器或网络请求。
    void informationCopied(const QString& text);
private:
    void rebuild();
    QString productName() const;
    QString version() const;
    AboutInfo m_info;
    QVBoxLayout* m_outer=nullptr;
    QWidget* m_page=nullptr;
    bool m_copyVisible=true;
};
}
