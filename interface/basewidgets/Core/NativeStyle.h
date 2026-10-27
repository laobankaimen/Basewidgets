#ifndef BASEWIDGETS_NATIVESTYLE_H
#define BASEWIDGETS_NATIVESTYLE_H

#include <QProxyStyle>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 应用级 QPainter 样式，保留 Qt 控件的输入、布局和可访问性逻辑。
 * 通常由 StyleManagerAdapter 安装，不要为每个控件创建实例。
 * 普通 Qt 按钮可设置 bwAppearance=int(ButtonAppearance)；库按钮优先用 setAppearance。
 * bwPrimary/primary/active 为兼容状态；bwWindowControl 为标题栏内部协议。
 * bwFrameHoverEnabled 显式启用容器边框 hover；默认关闭，输入框与键盘焦点单独处理。
 * 尺寸使用逻辑像素，不要与 QSS padding/border 叠加使用。
 */
class ACSS_UI_EXPORT NativeStyle : public QProxyStyle
{
    Q_OBJECT
public:
    NativeStyle();
    using QProxyStyle::polish;
    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr,
                    const QWidget* widget = nullptr) const override;
    /** 跨平台外观策略：标准对话框按钮默认纯文字，不注入平台主题图标。 */
    int styleHint(StyleHint hint, const QStyleOption* option = nullptr,
                  const QWidget* widget = nullptr, QStyleHintReturn* returnData = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                           const QSize& size, const QWidget* widget = nullptr) const override;
    QRect subControlRect(ComplexControl control, const QStyleOptionComplex* option,
                         SubControl subControl, const QWidget* widget = nullptr) const override;
    SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                                    const QPoint& point, const QWidget* widget = nullptr) const override;
    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption* option = nullptr,
                       const QWidget* widget = nullptr) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget = nullptr) const override;
    void drawControl(ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget = nullptr) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                            QPainter* painter, const QWidget* widget = nullptr) const override;
protected:
    bool eventFilter(QObject* object, QEvent* event) override;
};

} // namespace basewidgets
#endif
