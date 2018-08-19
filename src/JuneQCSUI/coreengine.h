#pragma once

#include <QObject>
#include <QSharedPointer>
#include "ProcessParameter.h"
#include "IFrameProvider.h"

namespace LandaJune
{
	namespace Core
	{
		class CoreEngine : public QObject
		{
			Q_OBJECT

			friend class QSharedPointer<CoreEngine>;

		public:
			static CoreEngine* get();

			void init();
			void cleanup();

	
			const QList<FrameProviders::IFrameProvider::FrameProviderPtr>& getFrameProviderList() const;
			std::shared_ptr <Parameters::ProcessParameter> getBatchParameters() const;
			
			void selectFrameProvider(FrameProviders::IFrameProvider::FrameProviderPtr provider);
			FrameProviders::IFrameProvider::FrameProviderPtr getSelectedFrameProvider() const;
		
			void start() const;
			void stop() const;

		private :

			void initGlobalParameters();
			void initFramePool() const;
			void initProviders();

			explicit CoreEngine(QObject *parent = nullptr);
			~CoreEngine();

			inline static CoreEngine* _this = nullptr;

			bool _bInited = false;

			QList<FrameProviders::IFrameProvider::FrameProviderPtr> _providerList;
			FrameProviders::IFrameProvider::FrameProviderPtr	_currentFrameProvider;
			std::shared_ptr <Parameters::ProcessParameter>	_commonBatchParameters;
		};
	}
}
