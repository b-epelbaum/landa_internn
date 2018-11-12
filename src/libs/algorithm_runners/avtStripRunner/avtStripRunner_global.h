#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(AVTSTRIPRUNNER_LIB)
#  define AVTSTRIPRUNNER_EXPORT Q_DECL_EXPORT
# else
#  define AVTSTRIPRUNNER_EXPORT Q_DECL_IMPORT
# endif
#else
# define AVTSTRIPRUNNER_EXPORT
#endif
