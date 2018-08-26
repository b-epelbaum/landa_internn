#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(REGISTRATIONHANDLER_LIB)
#  define REGISTRATIONHANDLER_EXPORT Q_DECL_EXPORT
# else
#  define REGISTRATIONHANDLER_EXPORT Q_DECL_IMPORT
# endif
#else
# define REGISTRATIONHANDLER_EXPORT
#endif
