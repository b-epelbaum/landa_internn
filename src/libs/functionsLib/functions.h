#pragma once
#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <functional>


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
		void frameGenerate(std::shared_ptr<FrameProviders::IFrameProvider> frameProvider, Core::ICore * coreObject, std::function<void( Core::ICore *, std::shared_ptr<Core::SharedFrameData>)> viewFunc);
		void frameConsume(std::shared_ptr<Algorithms::IAlgorithmRunner> algorithmRunner);
		void frameRunAlgorithms(Core::FrameRef *frame, const std::unique_ptr<Algorithms::IAlgorithmRunner>& algorithmHandler);
		void frameSaveData(std::tuple<std::shared_ptr<std::vector<unsigned char>>, std::string> & args);
	}
}


