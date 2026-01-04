/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef QTADVANCED_UI_EXPORT_H
#define QTADVANCED_UI_EXPORT_H

#include <QtGlobal>

#if defined(QTADVANCED_UI_BUILD)
#  define ACSS_UI_EXPORT Q_DECL_EXPORT
#else
#  define ACSS_UI_EXPORT Q_DECL_IMPORT
#endif

#endif // QTADVANCED_UI_EXPORT_H

