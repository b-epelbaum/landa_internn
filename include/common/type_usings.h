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
		class IAlgorithmHandler;
	}
}

namespace LandaJune {
	namespace Parameters {
		class ProcessParameter;
	}
}

namespace cv {
	class Mat;
}

namespace LandaJune
{
	using FrameProviderPtr = std::shared_ptr<FrameProviders::IFrameProvider>;
	using AlgorithmHandlerPtr = std::shared_ptr<Algorithms::IAlgorithmHandler>;
	using ProcessParameterPtr = std::shared_ptr<Parameters::ProcessParameter>;

	using IPropertyTuple = std::tuple<QString, QVariant, bool>;
	using IPropertyList = QVector<IPropertyTuple>;
}