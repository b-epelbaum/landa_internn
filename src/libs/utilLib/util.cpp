#include "util.h"
#include <Windows.h>
using namespace LandaJune::Helpers;


unsigned long Utility::threadId()
{
	return GetCurrentThreadId();
}

