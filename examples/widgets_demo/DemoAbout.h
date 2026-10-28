#pragma once
#include <basewidgets/Containers/AboutWidget.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
/** 演示配置，业务可从自己的配置文件/构建元信息组装同一结构，不需改库源码。 */
inline basewidgets::AboutInfo demoAboutInfo()
{
    basewidgets::AboutInfo info;
    info.productName="水声数据管理中心"; info.version="1.0.0-demo"; info.tagline="采集 · 分析 · 存储管理";
    info.build="Widgets 示例构建"; info.organization="示例组织（可配置）";
    info.description="面向水声录音、频谱分析与数据管理的桌面工作区。此处展示产品说明，可替换为业务软件的介绍与支持信息。";
    info.copyright="© 示例组织 · 版权信息由应用配置";
    info.license="示例许可说明：发布产品时，请填写实际授权条款与所用第三方组件的许可信息。\n本组件按纯文本展示配置，不读取或推断软件授权。";
    info.details={{"界面模式","Qt Widgets"},{"数据来源","合成演示数据"}};
    info.links={{"guide","使用指南",QUrl("app:guide")},{"licenses","第三方许可",QUrl("app:licenses")}};
    info.logo=basewidgets::StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/logo/logo.svg"); return info;
}
