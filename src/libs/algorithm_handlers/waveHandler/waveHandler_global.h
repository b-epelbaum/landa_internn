#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(WAVEHANDLER_LIB)
#  define WAVEHANDLER_EXPORT Q_DECL_EXPORT
# else
#  define WAVEHANDLER_EXPORT Q_DECL_IMPORT
# endif
#else
# define WAVEHANDLER_EXPORT
#endif
