#pragma once
#include "coreLib_global.h"
#include "interfaces/ICore.h"
#include "common/june_exceptions.h"

#include <mutex>
#include <any>

#include <QObject>
#include <QMetaObject>

#define DECLARE_STATIC_CORE_HANDLER(x) static void on_##x ( BaseCore *coreObject, std::any& callbackData );

namespace LandaJune {
	enum class CoreCallbackType;
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

			using EventParserFunction = std::function<void( BaseCore *, std::any& )>;
			using EVENT_MAP = std::map<CoreCallbackType, EventParserFunction>;
		
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

			// provider signals
			void providerScannedFilesCount		( int sourceFileCount );
			void providerFrameGeneratedOk		( int frameIndex );	
			void providerFrameSkipped			( int frameIndex );		
			void providerFinished				();		
			void providerFrameImageData			(std::shared_ptr<LandaJune::Core::SharedFrameData> frameData);

			// runner signals
			void runnerFrameHandledOk			( int frameIndex );
			void runnerFrameSkipped				( int frameIndex );
			void runnerDetectionSuccess			( int frameIndex );
			void runnerDetectionFailure			( int frameIndex );

			// saver signals
			void fileSaverFailure				( CORE_ERROR err );

		private slots:

			// provider slots
			void _onProviderScannedFilesCount		( int sourceFileCount );
			void _onProviderFrameGeneratedOk		( int frameIndex );	
			void _onProviderFrameSkipped			( int frameIndex );		
			void _onProviderFinished				();		
			void _onProviderFrameImageData			(std::shared_ptr<LandaJune::Core::SharedFrameData> frameData);	
			void _onProviderException				(std::exception_ptr pex);

			// runner slots
			void _onRunnerFrameHandledOk			( int frameIndex );
			void _onRunnerFrameSkipped				( int frameIndex );
			void _onRunnerDetectionSuccess			( int frameIndex );
			void _onRunnerDetectionFailure			( int frameIndex );
			void _onRunnerException					(std::exception_ptr pex);

			// file saver slots
			void _onFileSaverFailure					( int err );
	


		protected :
			
			virtual ~BaseCore() = default;
			const BaseCore & operator = (const BaseCore &) = delete;
			BaseCore & operator = (BaseCore &&) = delete;

			virtual QString getDefaultConfigurationFileName() const ;
			virtual void saveConfiguration();

			virtual void run(BaseParametersPtr params);

			void cleanDestinationFolder (const QString& destFolder );
						
			void initProcessParameters();
			void initFramePool() const;
			void initProviders();
			void initAlgorithmRunners();
			void initFileWriter( bool bInit );

			void processProviderExceptionData(std::exception_ptr pex);
			void processRunnerExceptionData(std::exception_ptr pex);
			
			bool _bInited			= false;
			bool _reportEvents		= false;
			bool _waitingForFirstSkippedHandledFrame = false;

			std::list<FrameProviderPtr>			_providerList;
			std::list<AlgorithmRunnerPtr>		_algorithmRunnerList;

			FrameProviderPtr	_currentFrameProvider;
			AlgorithmRunnerPtr	_currentAlgorithmRunner;
			BaseParametersPtr	_processParameters;
	
			
			std::mutex _mutex;
			
			static EVENT_MAP _eventParserMap;
			static void coreEventCallback ( ICore * coreObject, CoreCallbackType callbackType, std::any callbackData );

			// static event parser functions

			DECLARE_STATIC_CORE_HANDLER (ProviderScannedFilesCount)
			DECLARE_STATIC_CORE_HANDLER (ProviderFrameGeneratedOk)
			DECLARE_STATIC_CORE_HANDLER (ProviderFrameSkipped)
			DECLARE_STATIC_CORE_HANDLER (ProviderFinished)
			DECLARE_STATIC_CORE_HANDLER (ProviderFrameImageData)
			DECLARE_STATIC_CORE_HANDLER (ProviderException)

			DECLARE_STATIC_CORE_HANDLER (RunnerFrameHandledOk)
			DECLARE_STATIC_CORE_HANDLER (RunnerFrameSkipped)
			DECLARE_STATIC_CORE_HANDLER (RunnerDetectionSuccess)
			DECLARE_STATIC_CORE_HANDLER (RunnerDetectionFailure)
			DECLARE_STATIC_CORE_HANDLER (RunnerException)

			DECLARE_STATIC_CORE_HANDLER (SaverError)

			static void on_UnknownEvent( BaseCore *coreObject, CoreCallbackType callbackType, std::any& callbackData );
		};
	}
}

Q_DECLARE_METATYPE(LandaJune::BaseException)
Q_DECLARE_METATYPE(std::exception_ptr)
Q_DECLARE_METATYPE(std::shared_ptr<LandaJune::Core::SharedFrameData>)

