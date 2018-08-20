#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(FGSIMULATOR_LIB)
#  define FGSIMULATOR_EXPORT Q_DECL_EXPORT
# else
#  define FGSIMULATOR_EXPORT Q_DECL_IMPORT
# endif
#else
# define FGSIMULATOR_EXPORT
#endif
