#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(SISOPROVIDER_LIB)
#  define SISOPROVIDER_EXPORT Q_DECL_EXPORT
# else
#  define SISOPROVIDER_EXPORT Q_DECL_IMPORT
# endif
#else
# define SISOPROVIDER_EXPORT
#endif
