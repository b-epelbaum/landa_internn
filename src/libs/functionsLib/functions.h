#pragma once
#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <functional>
#include <any>

#include "common/type_usings.h"


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
		void frameGenerate(FrameProviderPtr, Core::ICore * coreObject, FrameProviderCallback );
		void frameConsume(AlgorithmRunnerPtr, Core::ICore * coreObject, FrameConsumerCallback );

		void frameRunAlgorithms(Core::FrameRef *frame, const AlgorithmRunnerUniquePtr& algorithmHandler);
		void frameSaveData(SaveDataType& args);
	}
}


