#ifndef acss_globalsH
#define acss_globalsH

#include <QtCore/QtGlobal>
#include <QDebug>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

#define ACSS_EXPORT ACSS_UI_EXPORT

// Define ACSS_DEBUG_PRINT to enable a lot of debug output
#ifdef ACSS_DEBUG_PRINT
#define ACSS_PRINT(s) qDebug() << s
#else
#define ACSS_PRINT(s)
#endif

// Set ACSS_DEBUG_LEVEL to enable additional debug output and to enable layout
// dumps to qDebug and std::cout after layout changes
#define ACSS_DEBUG_LEVEL 0

#endif // acss_globalsH
