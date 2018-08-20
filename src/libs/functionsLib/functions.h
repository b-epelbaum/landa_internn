#pragma once
#include <string>

#include "functionsLib_global.h"
#include "interfaces/type_usings.h"

namespace LandaJune
{
	namespace Functions
	{
		void frameGenerate(FrameProviderPtr frameProvider);
		void frameConsume();
		void frameCreateRegions(const Core::FrameRef *frame);
		void frameRunAlgorithms(Core::FrameRef *frame);
		void frameProcessOutput();
		void frameSaveImage(const cv::Mat& image, const std::string& pathName);
	}
}


