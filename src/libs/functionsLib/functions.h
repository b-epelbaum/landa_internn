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
		void frameRunAlgorithms(Core::FrameRef *frame, const std::unique_ptr<Algorithms::IAlgorithmHandler>& algorithmHandler);
		void frameProcessOutput();
		void frameSaveImage(cv::Mat image, std::string pathName);
		void frameSaveImage(cv::Mat * pimage, std::string pathName);
	}
}


