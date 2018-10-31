#pragma once
#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <functional>
#include <any>

#include "common/type_usings.h"
#include "common/june_errors.h"


namespace LandaJune {
	namespace FrameProviders {
		class IFrameProvider;
	}
}

namespace LandaJune {
	namespace Algorithms {
		class IAlgorithmRunner;
	}
}


namespace LandaJune {
	namespace Core {
		class FrameRef;
		class ICore;
		struct SharedFrameData;
	}
}

namespace cv {
	class Mat;
}


namespace LandaJune
{
	namespace Functions
	{
		CORE_ERROR frameGenerate(BaseParametersPtr, FrameProviderPtr, Core::ICore *, CoreEventCallback );
		void frameGeneratorCleanup	(FrameProviderPtr, Core::ICore *, CoreEventCallback );
		
		CORE_ERROR frameConsume	(BaseParametersPtr, AlgorithmRunnerPtr, Core::ICore *, CoreEventCallback );
		void frameRunnerCleanup	(AlgorithmRunnerPtr, Core::ICore *, CoreEventCallback );

		CORE_ERROR frameSaveData(SaveDataType& args);
	}
}


