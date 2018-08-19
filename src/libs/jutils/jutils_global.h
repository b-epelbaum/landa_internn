#pragma once


#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(JUTILS_LIB)
#  define JUTILS_EXPORT Q_DECL_EXPORT
# else
#  define JUTILS_EXPORT Q_DECL_IMPORT
# endif
#else
# define JUTILS_EXPORT
#endif
