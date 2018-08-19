#pragma once

#include <QIODevice>
#include "frameRef.h"
#include "interfaces/IQBAse.h"
#include "common/june_errors.h"
#include "baseparam.h"

namespace LandaJune
{
	namespace FrameProviders
	{
		class BaseFrameProvider;
	}
}

namespace LandaJune {
	namespace Parameters {
		class ProcessParameter;
	}
}

namespace LandaJune
{
	namespace FrameProviders
	{
		class IFrameProvider : public IQBase
		{
			friend class BaseFrameProvider;

		public:
			using FrameProviderPtr = QSharedPointer<IFrameProvider>;

		public:

			IFrameProvider() = default;
			IFrameProvider(const IFrameProvider &) = delete;
			IFrameProvider(IFrameProvider &&) = delete;
			virtual ~IFrameProvider() = default;
			const IFrameProvider & operator = (const IFrameProvider &) = delete;
			IFrameProvider & operator = (IFrameProvider &&) = delete;

			virtual QString getName() const = 0;
			virtual QString getDescription() const = 0;
			virtual int getRecommendedFramePoolSize() = 0;
			
			virtual FRAME_PROVIDER_ERROR dataPreProcess(FrameRef* frameRef) = 0;
			virtual FRAME_PROVIDER_ERROR dataAccess(FrameRef* frameRef) = 0;
			virtual FRAME_PROVIDER_ERROR dataPostProcess(FrameRef* frameRef) = 0;

			virtual bool canContinue(FRAME_PROVIDER_ERROR lastError) = 0;

			virtual void loadDefaultConfiguration() = 0;
			virtual void loadConfiguration(QIODevice& strJSONFile) = 0;
			virtual void loadConfiguration(QString strJSON) = 0;
			
			virtual FRAME_PROVIDER_ERROR init() = 0;
			virtual FRAME_PROVIDER_ERROR clean() = 0;

			virtual int32_t getFrameDropDelayTimeout() const = 0;
			virtual int64_t getCurrentFrameIndex() const = 0;
			virtual Parameters::IPropertyList getProviderProperties() const = 0;
			virtual bool setProviderProperties(const Parameters::IPropertyList& vals ) = 0;
			virtual QVariant getProviderProperty(const QString& strValName) const = 0;
			virtual bool setProviderProperty(const QString& strValName, const QVariant& val) = 0;

			virtual bool isBusy() = 0;
			
			static QList<FrameProviderPtr> enumerateImageProviders();
		};
	}
}

#define IFrameProvider_iid "LandaCorp.com.JuneQCS.IFrameProvider"
Q_DECLARE_INTERFACE(LandaJune::FrameProviders::IFrameProvider, IFrameProvider_iid)
Q_DECLARE_METATYPE(QSharedPointer<LandaJune::FrameProviders::IFrameProvider>)