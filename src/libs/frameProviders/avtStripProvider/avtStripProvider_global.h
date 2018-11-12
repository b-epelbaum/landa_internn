#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(AVTSTRIPPROVIDER_LIB)
#  define AVT_STRIP_PROV_EXPORT Q_DECL_EXPORT
# else
#  define AVT_STRIP_PROV_EXPORT Q_DECL_IMPORT
# endif
#else
# define AVT_STRIP_PROV_EXPORT
#endif
