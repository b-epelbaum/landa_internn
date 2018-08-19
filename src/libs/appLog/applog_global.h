#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(APPLOG_LIB)
#  define APPLOG_EXPORT Q_DECL_EXPORT
# else
#  define APPLOG_EXPORT Q_DECL_IMPORT
# endif
#else
# define APPLOG_EXPORT
#endif
