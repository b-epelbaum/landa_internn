#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(FULLFRAMECYCLER_LIB)
#  define FULL_FRAME_CYCLER_EXPORT Q_DECL_EXPORT
# else
#  define FULL_FRAME_CYCLER_EXPORT Q_DECL_IMPORT
# endif
#else
# define FULL_FRAME_CYCLER_EXPORT
#endif
