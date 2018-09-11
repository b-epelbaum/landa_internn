#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(BASEALGORITHMRUNNER_LIB)
#  define BASE_ALGORITHM_RUNNER_EXPORT Q_DECL_EXPORT
# else
#  define BASE_ALGORITHM_RUNNER_EXPORT Q_DECL_IMPORT
# endif
#else
# define BASE_ALGORITHM_RUNNER_EXPORT
#endif
