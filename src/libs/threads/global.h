#pragma once

#ifndef BUILD_STATIC
# if defined(THREADS_LIB)
#  define THREADS_EXPORT  __declspec(dllexport)
//#  define THREADS_EXTERN
# else
#  define THREADS_EXPORT  __declspec(dllimport)
//#  define THREADS_EXTERN extern
# endif
#else
# define THREADS_EXPORT
#endif
