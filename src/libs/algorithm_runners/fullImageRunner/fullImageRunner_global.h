#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(FULLIMAGERUNNER_LIB)
#  define FULLIMAGERUNNER_EXPORT Q_DECL_EXPORT
# else
#  define FULLIMAGERUNNER_EXPORT Q_DECL_IMPORT
# endif
#else
# define FULLIMAGERUNNER_EXPORT
#endif
