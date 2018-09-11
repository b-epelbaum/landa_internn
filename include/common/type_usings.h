#pragma once

#include <memory>
#include <QVariant>
#include <QString>

namespace LandaJune {
	namespace Core {
		class FrameRef;
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
	}
}

namespace cv {
	class Mat;
}

namespace LandaJune
{
	using FrameProviderPtr = std::shared_ptr<FrameProviders::IFrameProvider>;
	using AlgorithmRunnerPtr = std::shared_ptr<Algorithms::IAlgorithmRunner>;
	using ProcessParametersPtr = std::shared_ptr<Parameters::ProcessParameters>;

	using IPropertyTuple = std::tuple<QString, QVariant, bool>;
	using IPropertyList = QVector<IPropertyTuple>;
}