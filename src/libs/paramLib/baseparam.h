#pragma once

#include "paramlib_global.h"
#include <QVector>
#include <QObject>
#include "interfaces/type_usings.h"


#define DECLARE_PARAM_PROPERTY(x,type,initval,editable) Q_PROPERTY(type x MEMBER _##x READ x WRITE set##x USER editable) private: type _##x = initval; public: type x() const { return _##x; } void set##x(const type val) { _##x = val; if (editable) emit propertyChanged(#x); }

namespace LandaJune
{
	namespace Parameters
	{
		class BaseParameter : public QObject
		{
			Q_OBJECT 
		
		public:
			
			BaseParameter() = default;
			virtual ~BaseParameter() = default;

			virtual QString	GetMetaClassFullName() const { return "Landa::Core::IParameter"; }
			virtual QString	GetMetaClassDebugName() const { return "IParameter"; }

			virtual IPropertyList getPropertyList() const;
			virtual bool setPropertyList(const IPropertyList& vals);
			virtual QVariant getParamProperty(const QString& strValName) const;
			virtual bool setParamProperty(const QString& strValName, const QVariant& val);

			virtual QString serialize() { return ""; }

		signals:

			void propertyChanged(QString strPropName);
		};
	}
}

Q_DECLARE_METATYPE(LandaJune::IPropertyTuple)