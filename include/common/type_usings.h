#pragma once

#include <memory>
#include <QVariant>
#include <QString>
#include <any>

namespace LandaJune {
	namespace Core {
		class FrameRef;
		class ICore;
	}
}

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
	namespace Parameters {
		class ProcessParameters;
		class BaseParameters;
	}
}

namespace cv {
	class Mat;
}

namespace LandaJune {
	enum class FrameProviderDataCallbackType;
	enum class FrameConsumerDataCallbackType;
}

namespace LandaJune
{
	using FrameProviderPtr = std::shared_ptr<FrameProviders::IFrameProvider>;
	using AlgorithmRunnerPtr = std::shared_ptr<Algorithms::IAlgorithmRunner>;
	using AlgorithmRunnerUniquePtr = std::shared_ptr<Algorithms::IAlgorithmRunner>;
	using ProcessParametersPtr = std::shared_ptr<Parameters::ProcessParameters>;
	using BaseParametersPtr = std::shared_ptr<Parameters::BaseParameters>;

	using IPropertyTuple = std::tuple<QString, QVariant, bool>;
	using IPropertyList = QVector<IPropertyTuple>;

	using FrameProviderCallback = std::function<void( Core::ICore *, FrameProviderDataCallbackType, std::any)>;
	using FrameConsumerCallback = std::function<void( Core::ICore *, FrameConsumerDataCallbackType, std::any)>;

	using SaveDataType = std::tuple<std::shared_ptr<std::vector<unsigned char>>, std::string>;
}

