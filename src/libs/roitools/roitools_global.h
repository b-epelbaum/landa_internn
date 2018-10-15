#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(ROITOOLS_LIB)
#  define ROITOOLS_EXPORT Q_DECL_EXPORT
# else
#  define ROITOOLS_EXPORT Q_DECL_IMPORT
# endif
#else
# define ROITOOLS_EXPORT
#endif
