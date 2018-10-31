#pragma once
#include "../interfaces/IFrameProvider.h"

#include <QObject>
#include <QMetaObject>
#include "applog.h"
#include "baseparam.h"

namespace LandaJune
{
	namespace FrameProviders
	{
		#define CHECK_INIT if (!_coreObject ) \
		return CORE_ERROR::ERR_PROVIDER_UNINITIALIZED; \
		if (!_currentOfflineProvider ) \
		{ \
			THROW_EX_ERR(CORE_ERROR::ERR_PROVIDER_INVALID_SELECTED_PROVIDER); \
		}


		class BaseFrameProvider :  public QObject, public IFrameProvider
		{
			Q_OBJECT

			friend class IFrameProvider;
			friend class offlineFrameProvider;
		
		public:
			BaseFrameProvider() = default;
			BaseFrameProvider(const BaseFrameProvider &) = delete;
			BaseFrameProvider(BaseFrameProvider &&) = delete;
			virtual ~BaseFrameProvider() = default;
			const BaseFrameProvider & operator = (const BaseFrameProvider &) = delete;
			BaseFrameProvider & operator = (BaseFrameProvider &&) = delete;

			bool isInited() const override { return _coreObject && _dataCallback && _providerParameters; }

			QString getName() const override { return _name;  }
			QString getDescription() const override { return _description; }

			BaseParametersPtr getProviderParameters() const override { return _providerParameters;  }
			IPropertyList getProviderProperties() const override;

			int32_t getFrameDropDelayTimeout() const override {
				return _DropFrameWaitTimeout;
			}

			int64_t getCurrentFrameIndex() const override { return _lastAcquiredImage; };

			QVariant getProviderProperty(const QString& strValName) const override;
			bool setProviderProperty(const QString& strValName, const QVariant& val) override;

			bool isBusy() override {
				return _busy;
			}

			DECLARE_NORMAL_PARAM_PROPERTY(DropFrameWaitTimeout, int, 50)

		public slots:

			void onUpdateParameters();

		protected :
			
		
			virtual int64_t getLastImageIndex() const {
				return _lastAcquiredImage;
			}

			virtual void validateParameters(BaseParametersPtr parameters) = 0;

			CoreEventCallback _dataCallback;
			Core::ICore * _coreObject = nullptr;
			BaseParametersPtr _providerParameters;
			
			bool _busy = false;

			int64_t _lastAcquiredImage = 0;
			QString _name;
			QString _description;
		};
	}
}

