#pragma once
#include "coreLib_global.h"
#include "interfaces/ICore.h"
#include <mutex>
#include <QObject>
#include <QMetaObject>
#include "baseProviderLib/BaseFrameProvider.h"

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

			void init() override;
			void cleanup() override;

			const std::list<FrameProviderPtr>& getFrameProviderList() const override;
			const std::list<AlgorithmRunnerPtr>& getAlgorithmRunnerList() const override;

			std::shared_ptr<Parameters::BaseParameters> getProcessParameters() override;

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

		private slots:

			void onException (BaseException ex);

		protected :
			
			virtual ~BaseCore() = default;
			const BaseCore & operator = (const BaseCore &) = delete;
			BaseCore & operator = (BaseCore &&) = delete;

			virtual QString getDefaultConfigurationFileName() const ;
			virtual void saveConfiguration();

			virtual void run(std::shared_ptr<Parameters::BaseParameters> params);

			
			void initProcessParameters();
			void initFramePool() const;
			void initProviders();
			void initAlgorithmRunners();
			void initFileWriter( bool bInit ) const;
			bool _bInited = false;

			std::list<FrameProviderPtr> _providerList;
			std::list<AlgorithmRunnerPtr> _algorithmRunnerList;

			FrameProviderPtr	_currentFrameProvider;
			AlgorithmRunnerPtr	_currentAlgorithmRunner;
			
			std::shared_ptr<Parameters::BaseParameters>	_processParameters;
			bool _bCanAcceptExceptions = true;
			std::mutex _mutex;

			static void providerExceptionHandler ( void * pThis, BaseException& ex ) noexcept;
			static void consumerExceptionHandler ( void * pThis, BaseException& ex ) noexcept;
		};
	}
}

Q_DECLARE_METATYPE(LandaJune::BaseException)

