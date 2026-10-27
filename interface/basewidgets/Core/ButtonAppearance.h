#ifndef BASEWIDGETS_BUTTONAPPEARANCE_H
#define BASEWIDGETS_BUTTONAPPEARANCE_H

#include <QMetaType>

namespace basewidgets {
/** 按钮的视觉层级；只改变外观，不改变 clicked/toggled 或默认按钮行为。 */
enum class ButtonAppearance {
    Neutral, ///< 普通操作：中性底色、无描边，适合“取消”“打开”等次要操作。
    Primary, ///< 当前区域最重要的操作：强调色实底，建议每组只使用一个。
    Subtle,  ///< 弱化操作：常态透明，悬停/按下时显示底色，适合工具栏。
    Outline, ///< 需要额外边界时显式启用描边；不要用于所有普通按钮。
    Danger   ///< 破坏性操作：危险色实底；是否确认由调用方决定。
};
}
Q_DECLARE_METATYPE(basewidgets::ButtonAppearance)
#endif
