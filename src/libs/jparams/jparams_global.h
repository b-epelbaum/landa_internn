#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(JPARAMS_LIB)
#  define JPARAMS_EXPORT Q_DECL_EXPORT
# else
#  define JPARAMS_EXPORT Q_DECL_IMPORT
# endif
#else
# define JPARAMS_EXPORT
#endif
