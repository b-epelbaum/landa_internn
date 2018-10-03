#pragma once
#include "coreLib_global.h"
#include "interfaces/ICore.h"
#include "common/june_exceptions.h"

#include <mutex>
#include <any>

#include <QObject>
#include <QMetaObject>


namespace LandaJune {
	enum class FrameProviderDataCallbackType;
	enum class FrameConsumerDataCallbackType;
}

namespace LandaJune {
	namespace Core {
		class FrameRef;
		struct SharedFrameData;
	}
}

namespace LandaJune
{
	namespace Core
	{
		class CORELIB_EXPORT BaseCore :  public QObject, public ICore
		{
			Q_OBJECT
			Q_PLUGIN_METADATA(IID ICore_iid)
			Q_INTERFACES(LandaJune::Core::ICore)

			friend class ICore;

			using autolock = std::lock_guard<std::mutex>;

		
		public:

			BaseCore() = default;
			BaseCore(const BaseCore &) = delete;
			BaseCore(BaseCore &&) = delete;

			void init(bool reportEvents ) override;
			void cleanup() override;

			const std::list<FrameProviderPtr>& getFrameProviderList() const override;
			const std::list<AlgorithmRunnerPtr>& getAlgorithmRunnerList() const override;

			BaseParametersPtr getProcessParameters() override;

			void selectFrameProvider(FrameProviderPtr provider) override;
			FrameProviderPtr getSelectedFrameProvider() const override;

			void selectAlgorithmRunner(AlgorithmRunnerPtr algoRunner) override;
			AlgorithmRunnerPtr getSelectedAlgorithmRunner() const override;

			QObject * getClassObject () override { return this; }

			void runOne() override;
			void runAll() override;
			void stop() override;

			std::string getRootFolderForOneRun() const override;
						
			bool isBusy() override;

			void deleteThis() const
			{
				delete this;
			}

		signals :

			void coreStopped();
			void coreException(const LandaJune::BaseException ec);
			void frameData (std::shared_ptr<LandaJune::Core::SharedFrameData> frameData );
			void frameProcessed ( int frameIndex );
			void offlineFileSourceCount( int iCount );

		private slots:

			void onCriticalException (BaseException ex);
			void onFrameData (std::shared_ptr<LandaJune::Core::SharedFrameData> frameData);
			void onFrameProcessed ( int frameIndex );
			void onFileCountData ( int sourceFileCount );

		protected :
			
			virtual ~BaseCore() = default;
			const BaseCore & operator = (const BaseCore &) = delete;
			BaseCore & operator = (BaseCore &&) = delete;

			virtual QString getDefaultConfigurationFileName() const ;
			virtual void saveConfiguration();

			virtual void run(BaseParametersPtr params);

			
			void initProcessParameters();
			void initFramePool() const;
			void initProviders();
			void initAlgorithmRunners();
			void initFileWriter( bool bInit ) const;
			void processExceptionData(BaseException& ex);
			
			bool _bInited = false;
			std::list<FrameProviderPtr> _providerList;
			std::list<AlgorithmRunnerPtr> _algorithmRunnerList;

			FrameProviderPtr	_currentFrameProvider;
			AlgorithmRunnerPtr	_currentAlgorithmRunner;
			
			BaseParametersPtr	_processParameters;
			bool _bCanAcceptExceptions = true;
			bool _bCanAcceptEvents = true;
			bool _reportEvents = false;
			std::mutex _mutex;

			static void providerDataCallback ( ICore * coreObject, FrameProviderDataCallbackType callbackType, std::any callbackData );
			static void consumerDataCallback ( ICore * coreObject, FrameConsumerDataCallbackType callbackType, std::any callbackData );

			static void coreExceptionHandler ( ICore * coreObject, BaseException& ex ) noexcept;

		};
	}
}

Q_DECLARE_METATYPE(LandaJune::BaseException)
Q_DECLARE_METATYPE(std::shared_ptr<LandaJune::Core::SharedFrameData>)

