#pragma once

#include <memory>
#include <QPair>
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
	using ProcessParameterPtr = std::shared_ptr<Parameters::ProcessParameter>;

	using IPropertyPair = QPair<QString, QVariant>;
	using IPropertyList = QVector<IPropertyPair>;
}