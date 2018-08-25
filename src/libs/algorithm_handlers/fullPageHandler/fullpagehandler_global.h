#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(FULLPAGEHANDLER_LIB)
#  define FULLPAGEHANDLER_EXPORT Q_DECL_EXPORT
# else
#  define FULLPAGEHANDLER_EXPORT Q_DECL_IMPORT
# endif
#else
# define FULLPAGEHANDLER_EXPORT
#endif
