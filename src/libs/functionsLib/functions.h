#pragma once
#include <string>

namespace LandaJune {
	namespace FrameProviders {
		class IFrameProvider;
	}
}

namespace LandaJune {
	namespace Algorithms {
		class IAlgorithmHandler;
	}
}


namespace LandaJune {
	namespace Core {
		class FrameRef;
	}
}

namespace cv {
	class Mat;
}


namespace LandaJune
{
	namespace Functions
	{
		void frameGenerate(std::shared_ptr<FrameProviders::IFrameProvider> frameProvider);
		void frameConsume(std::shared_ptr<Algorithms::IAlgorithmHandler> algorithmHandler);
		void frameCreateRegions(const Core::FrameRef *frame);
		void frameRunAlgorithms(Core::FrameRef *frame, std::unique_ptr<Algorithms::IAlgorithmHandler> algorithmHandler);
		void frameProcessOutput();
		void frameSaveImage(const cv::Mat& image, const std::string& pathName);
	}
}


