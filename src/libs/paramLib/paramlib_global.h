#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(PARAMLIB_LIB)
#  define PARAMLIB_EXPORT Q_DECL_EXPORT
# else
#  define PARAMLIB_EXPORT Q_DECL_IMPORT
# endif
#else
# define PARAMLIB_EXPORT
#endif
