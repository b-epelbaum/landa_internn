#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(JUNECORE_LIB)
#  define JUNECORE_EXPORT Q_DECL_EXPORT
# else
#  define JUNECORE_EXPORT Q_DECL_IMPORT
# endif
#else
# define JUNECORE_EXPORT
#endif
