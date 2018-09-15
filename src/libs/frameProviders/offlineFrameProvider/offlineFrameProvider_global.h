#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(OFFLINEFRAMEPROVIDER_LIB)
#  define OFFLINE_FRAME_PROV_EXPORT Q_DECL_EXPORT
# else
#  define OFFLINE_FRAME_PROV_EXPORT Q_DECL_IMPORT
# endif
#else
# define OFFLINE_FRAME_PROV_EXPORT
#endif
