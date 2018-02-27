#ifndef EBMBUS_GLOBAL_H
#define EBMBUS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EBMBUS_LIBRARY)
#  define EBMBUSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EBMBUSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBEBMBUS_GLOBAL_H
