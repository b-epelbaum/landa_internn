#pragma once

#ifndef BUILD_STATIC
# if defined(APPLOG_LIB)
#  define APPLOG_EXPORT Q_DECL_EXPORT
# else
#  define APPLOG_EXPORT Q_DECL_IMPORT
# endif
#else
# define APPLOG_EXPORT
#endif
