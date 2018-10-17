#pragma once
#include <QDirIterator>
#include <QPluginLoader>
#include <QCoreApplication>

#include "common/type_usings.h"
#include "interfaces/IQBAse.h"

#define DLL_NAME "corelib.dll"

class QIODevice;

namespace LandaJune {
	namespace FrameProviders {
		class IFrameProvider;
	}
}

namespace LandaJune {
	namespace Parameters {
		class BaseParameters;
	}
}


namespace LandaJune
{
	namespace Core
	{
		class ICore : public IQBase
		{
		
		public:
			using CorePtr = std::shared_ptr<ICore>;

			ICore(const ICore &) = delete;
			ICore(ICore &&) = delete;
			const ICore & operator = (const ICore &) = delete;
			ICore & operator = (ICore &&) = delete;
		
			virtual void init( bool reportEvents ) = 0;
			virtual void cleanup() = 0;

			virtual const std::list<FrameProviderPtr>& getFrameProviderList() const = 0;
			virtual const std::list<AlgorithmRunnerPtr>& getAlgorithmRunnerList() const = 0;
					
			virtual BaseParametersPtr getProcessParameters() = 0;

			virtual void selectFrameProvider(FrameProviderPtr provider) = 0;
			virtual FrameProviderPtr getSelectedFrameProvider() const = 0;

			virtual void selectAlgorithmRunner(AlgorithmRunnerPtr algoRunner) = 0;
			virtual AlgorithmRunnerPtr getSelectedAlgorithmRunner() const = 0;

			virtual void runAll() = 0;
			virtual void runOne() = 0;
			virtual void stop() = 0;

			virtual std::string getRootFolderForOneRun() const = 0;

			virtual bool isBusy() = 0;
			virtual QObject * getClassObject () = 0;
			
			static CorePtr get();

		protected:

			ICore() = default;
			virtual ~ICore() = default;

			inline static CorePtr _pThis = nullptr;
		};
	}
}

#define ICore_iid "LandaCorp.com.JuneQCS.ICore"
Q_DECLARE_INTERFACE(LandaJune::Core::ICore, ICore_iid)
Q_DECLARE_METATYPE(std::shared_ptr<LandaJune::Core::ICore>)

inline LandaJune::Core::ICore::CorePtr LandaJune::Core::ICore::get()
{
	if (!_pThis)
	{
		const auto coreDllPath = qApp->applicationDirPath() + QDir::separator() + DLL_NAME;
		QPluginLoader loader(coreDllPath);
		const auto plugin = loader.instance();
		if (plugin != nullptr)
		{
			ICore* rawPtr = qobject_cast<ICore*>(plugin);
			if (rawPtr)
			{
				_pThis.reset(rawPtr, [](ICore*) {});
			}
		}
		else
		{
			auto errStr = loader.errorString();
		}
	}
	return _pThis;
}