#pragma once


#ifndef BUILD_STATIC
# if defined(STATLIB_LIB)
#  define STATLIB_EXPORT  __declspec(dllexport)
# else
#  define STATLIB_EXPORT  __declspec(dllimport)
# endif
#else
# define STATLIB_EXPORT
#endif
