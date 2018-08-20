#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(OFFREADER_LIB)
#  define OFFREADER_EXPORT Q_DECL_EXPORT
# else
#  define OFFREADER_EXPORT Q_DECL_IMPORT
# endif
#else
# define OFFREADER_EXPORT
#endif
