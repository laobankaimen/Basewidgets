/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef QTADVANCEDUI_EXPORT_H
#define QTADVANCEDUI_EXPORT_H

#include <QtGlobal>

#if defined(QTADVANCEDUI_BUILD)
#  define ACSS_UI_EXPORT Q_DECL_EXPORT
#else
#  define ACSS_UI_EXPORT Q_DECL_IMPORT
#endif

#endif // QTADVANCEDUI_EXPORT_H

