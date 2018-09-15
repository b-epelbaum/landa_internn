#pragma once
#include "../interfaces/IFrameProvider.h"

#include <QObject>
#include <QMetaObject>
#include "applog.h"

#include "baseparam.h"

//#define DECLARE_PROVIDER_PROPERTY(x,type,initval) Q_PROPERTY(type x READ x WRITE set##x) private: type _##x = initval; public: type x() const { return _##x; } void set##x(type val) { _##x = val; }

namespace LandaJune
{
	namespace FrameProviders
	{
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

			QString getName() const override { return _name;  }
			QString getDescription() const override { return _description; }

			bool warnAboutDroppedFrames() override { return true; }

			void setProviderParameters(std::shared_ptr<Parameters::BaseParameters> parameters) override;
			std::shared_ptr<Parameters::BaseParameters>  getProviderParameters() const override { return _providerParameters;  }

			int32_t getFrameDropDelayTimeout() const override {
				return _DropFrameWaitTimeout;
			}

			int64_t getCurrentFrameIndex() const override { return _lastAcquiredImage; };

			IPropertyList getProviderProperties() const override;
			bool setProviderProperties(const IPropertyList& vals) override;

			QVariant getProviderProperty(const QString& strValName) const override;
			bool setProviderProperty(const QString& strValName, const QVariant& val) override;

			bool isBusy() override {
				return _busy;
			}

			DECLARE_PARAM_PROPERTY(DropFrameWaitTimeout, int, 50, true)

		public slots:

			void onUpdateParameters();

		protected :
			
			virtual int64_t getLastImageIndex() const {
				return _lastAcquiredImage;
			}

			virtual void validateParameters(std::shared_ptr<Parameters::BaseParameters> parameters) = 0;
			
			bool _busy = false;
			int64_t _lastAcquiredImage = 0;
			std::shared_ptr<Parameters::BaseParameters> _providerParameters;

			QString _name;
			QString _description;
		};
	}
}

