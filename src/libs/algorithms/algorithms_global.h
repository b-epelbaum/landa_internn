#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(ALGORITHMS_LIB)
#  define ALGORITHMS_EXPORT Q_DECL_EXPORT
# else
#  define ALGORITHMS_EXPORT Q_DECL_IMPORT
# endif
#else
# define ALGORITHMS_EXPORT
#endif
