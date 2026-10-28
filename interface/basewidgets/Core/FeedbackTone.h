#pragma once

namespace basewidgets {
/** 提示与筛选标签的语义色，实际颜色由 ThemeTokens / 当前主题提供。
 * 状态还应使用文字表达，不应仅靠颜色区分业务结果。
 */
enum class FeedbackTone { Neutral, Info, Success, Warning, Danger };
}
