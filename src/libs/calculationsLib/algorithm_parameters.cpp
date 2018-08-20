#include "algorithm_parameters.h"
#include "frameRef.h"

using namespace LandaJune::Core;
using namespace LandaJune::Parameters;

ABSTRACT_INPUT::ABSTRACT_INPUT(const FrameRef * frame)
	: _frame(frame)
	, _Pixel2MM_X(frame->getProcessParams()->Pixel2MM_X())
	, _Pixel2MM_Y(frame->getProcessParams()->Pixel2MM_Y())
{}