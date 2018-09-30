#pragma once

#ifndef BUILD_STATIC
# if defined(THREADS_LIB)
#  define THREADS_EXPORT  __declspec(dllexport)
# else
#  define THREADS_EXPORT  __declspec(dllimport)
# endif
#else
# define THREADS_EXPORT
#endif
