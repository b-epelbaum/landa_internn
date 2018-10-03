#pragma once
#include <QCoreApplication>
#include <QDirIterator>
#include <QPluginLoader>

#include "interfaces/IQBAse.h"
#include "common/june_exceptions.h"
#include "common/type_usings.h"


namespace LandaJune {
	namespace Core {
		class FrameRef;
	}
}

namespace LandaJune {
	namespace Parameters {
		class BaseParameters;
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

			IFrameProvider() = default;
			IFrameProvider(const IFrameProvider &) = delete;
			IFrameProvider(IFrameProvider &&) = delete;
			const IFrameProvider & operator = (const IFrameProvider &) = delete;
			IFrameProvider & operator = (IFrameProvider &&) = delete;
		
			virtual QString getName() const = 0;
			virtual QString getDescription() const = 0;
			virtual int getRecommendedFramePoolSize() = 0;
			
			virtual BaseParametersPtr getProviderParameters() const = 0;

			virtual CORE_ERROR prepareData(Core::FrameRef* frameRef) = 0;
			virtual CORE_ERROR accessData(Core::FrameRef* frameRef) = 0;
			virtual void releaseData(Core::FrameRef* frameRef) = 0;

			virtual bool canContinue(CORE_ERROR lastError) = 0;
			virtual bool warnAboutDroppedFrames() = 0;

			virtual CORE_ERROR init(BaseParametersPtr parameters, Core::ICore * coreObject, FrameProviderCallback callback ) = 0;
			virtual CORE_ERROR cleanup() = 0;

			virtual int32_t getFrameLifeSpan() const = 0;
			virtual int32_t getFrameDropDelayTimeout() const = 0;
			virtual int64_t getCurrentFrameIndex() const = 0;

			virtual IPropertyList getProviderProperties() const = 0;
			virtual bool setProviderProperties(const IPropertyList& vals ) = 0;
			
			virtual QVariant getProviderProperty(const QString& strValName) const = 0;
			virtual bool setProviderProperty(const QString& strValName, const QVariant& val) = 0;

			virtual bool isBusy() = 0;
			
			static std::list<FrameProviderPtr> enumerateImageProviders();
		
		protected:

			virtual ~IFrameProvider() = default;
			

			static IFrameProvider* loadNextProvider(const QString& strPath);
		};
	}
}

#define IFrameProvider_iid "LandaCorp.com.JuneQCS.IFrameProvider"
Q_DECLARE_INTERFACE(LandaJune::FrameProviders::IFrameProvider, IFrameProvider_iid)
Q_DECLARE_METATYPE(std::shared_ptr<LandaJune::FrameProviders::IFrameProvider>)

using namespace LandaJune::FrameProviders;

inline IFrameProvider* IFrameProvider::loadNextProvider(const QString& strPath)
{
	QPluginLoader loader(strPath);
	const auto plugin = loader.instance();
	if (plugin != nullptr)
	{
		return qobject_cast<IFrameProvider*>(plugin);
	}
	THROW_EX_ERR_STR (CORE_ERROR::ERR_RUNNER_DLL_CANNOT_BE_LOADED, loader.errorString().toStdString());
}


inline std::list<LandaJune::FrameProviderPtr> IFrameProvider::enumerateImageProviders()
{
	std::list<FrameProviderPtr> retVal;
	const auto rootDir = qApp->applicationDirPath();
	QDirIterator it(rootDir, QStringList() << "fp_*.dll", QDir::Files);
	while (it.hasNext())
	{
		const auto rawPtr = loadNextProvider(it.next());
		if (rawPtr)
		{
			const auto& name = rawPtr->getName();
			FrameProviderPtr ptr;
			ptr.reset(rawPtr, [](IFrameProvider*) {});
			retVal.emplace_back(ptr);
		}
	}
	return retVal;
}