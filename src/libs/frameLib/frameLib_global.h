#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(FRAMELIB_LIB)
#  define FRAMELIB_EXPORT Q_DECL_EXPORT
# else
#  define FRAMELIB_EXPORT Q_DECL_IMPORT
# endif
#else
# define FRAMELIB_EXPORT
#endif
