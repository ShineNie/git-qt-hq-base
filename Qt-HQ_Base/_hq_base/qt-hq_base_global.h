/************************************************************************
 ** Class name:     Macro defining
 ** Author:         NieXin
 ** Created date:   2022-12-26
 ** Used for:       The purpose of the header files of 'hq_base'
 **
 ** Used for 'cpp':
 **
 **         Add the code 'DEFINES += QTHQ_BASE_SOURCE' in the .pro file.
 **     Then the macro 'QTHQ_BASESHARED_EXPORT' means nothing. So the header
 **     files of 'hq_base' will used for normal 'cpp' source codes.
 **
 ** Used for 'dll' developing:
 **
 **         Add the code 'DEFINES += QTHQ_BASE_LIBRARY' in the .pro file.
 **     Then the macro 'QTHQ_BASESHARED_EXPORT' equals 'Q_DECL_EXPORT', it
 **     means dll exporting. So the header files of 'hq_base' will used for
 **     the 'dll' project, such as 'Qt-HQ_Base-dll'.
 **
 ** Used for 'dll' calling:
 **
 **         Do not add any other codes of macro defining in the .pro file.
 **     Then the macro 'QTHQ_BASESHARED_EXPORT' equals 'Q_DECL_IMPORT', it
 **     means dll importing. So the header files of 'hq_base' will used for
 **     the 'dll' calling project, such as 'Qt-HQ_Base-exe'.
 **
 ** In the header files:
 **
 **     This header file including is required.
 **     Class defining using macro 'QTHQ_BASESHARED_EXPORT' is required.
 **     Such as below:
 **
 **         #include "qt-hq_base_global.h"
 **
 **         class QTHQ_BASESHARED_EXPORT MyClass {...};
 **
 ************************************************************************/
#ifndef QTHQ_BASE_GLOBAL_H
#define QTHQ_BASE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QTHQ_BASE_SOURCE)
#  define QTHQ_BASESHARED_EXPORT //Used for normal 'cpp'.
#else

#if defined(QTHQ_BASE_LIBRARY)
#  define QTHQ_BASESHARED_EXPORT Q_DECL_EXPORT //Used for 'dll' developing.
#else
#  define QTHQ_BASESHARED_EXPORT Q_DECL_IMPORT //Used for 'dll' calling.
#endif

#endif

#endif // QTHQ_BASE_GLOBAL_H
