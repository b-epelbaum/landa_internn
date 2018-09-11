#pragma once
#include "corelib_global.h"
#include "interfaces/ICore.h"

#include <QObject>
#include <QMetaObject>

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
		
		public:

			BaseCore() = default;
			BaseCore(const BaseCore &) = delete;
			BaseCore(BaseCore &&) = delete;

			void loadDefaultConfiguration() override;
			void loadConfiguration(QIODevice& strJSONFile) override;
			void loadConfiguration(QString strJSON) override;

			void init() override;
			void cleanup() override;

			const std::list<FrameProviderPtr>& getFrameProviderList() const override;
			const std::list<AlgorithmRunnerPtr>& getAlgorithmRunnerList() const override;

			std::shared_ptr<Parameters::BaseParameters> getProcessParameters() override;

			void selectFrameProvider(FrameProviderPtr provider) override;
			FrameProviderPtr getSelectedFrameProvider() const override;

			void selectAlgorithmRunner(AlgorithmRunnerPtr algoRUnner) override;
			AlgorithmRunnerPtr getSelectedAlgorithmRunner() const override;

			void start() const override;
			void stop() const override;

			bool isBusy() override;

			void deleteThis() const
			{
				delete this;
			}

		protected :
			
			virtual ~BaseCore() = default;
			const BaseCore & operator = (const BaseCore &) = delete;
			BaseCore & operator = (BaseCore &&) = delete;

			virtual QString getDefaultConfigurationFileName() const ;
			virtual void saveConfiguration();
			
			void initGlobalParameters();
			void initFramePool() const;
			void initProviders();
			void initAlgorithmRunners();
			bool _bInited = false;

			std::list<FrameProviderPtr> _providerList;
			std::list<AlgorithmRunnerPtr> _algorithmRunnerList;

			FrameProviderPtr	_currentFrameProvider;
			AlgorithmRunnerPtr	_currentAlgorithmRunner;

			std::shared_ptr<Parameters::BaseParameters>	_processParameters;
		};
	}
}

