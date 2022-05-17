#ifndef LIBTHESLATE_GLOBAL_H
#define LIBTHESLATE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBTHESLATE_LIBRARY)
    #define LIBTHESLATE_EXPORT Q_DECL_EXPORT
#else
    #define LIBTHESLATE_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBTHESLATE_GLOBAL_H
