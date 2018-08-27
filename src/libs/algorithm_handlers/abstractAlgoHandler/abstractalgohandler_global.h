#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(ABSTRACTALGOHANDLER_LIB)
#  define ABSTRACTALGOHANDLER_EXPORT Q_DECL_EXPORT
# else
#  define ABSTRACTALGOHANDLER_EXPORT Q_DECL_IMPORT
# endif
#else
# define ABSTRACTALGOHANDLER_EXPORT
#endif
