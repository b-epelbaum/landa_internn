#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(UTILLIB_LIB)
#  define UTILLIB_EXPORT Q_DECL_EXPORT
# else
#  define UTILLIB_EXPORT Q_DECL_IMPORT
# endif
#else
# define UTILLIB_EXPORT
#endif
