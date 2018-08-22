#pragma once

#include "paramlib_global.h"
#include <QVector>
#include <QRect>
#include "interfaces/type_usings.h"


#define DECLARE_PARAM_PROPERTY(x,type,initval,editable) Q_PROPERTY(type x MEMBER _##x READ x WRITE set##x USER editable) private: type _##x = initval; public: type x() const { return _##x; } void set##x(const type val) { _##x = val; if (editable) emit propertyChanged(#x); }

namespace LandaJune
{
	namespace Parameters
	{
		struct PARAM_GROUP_HEADER
		{
			PARAM_GROUP_HEADER() = default;
			PARAM_GROUP_HEADER(const PARAM_GROUP_HEADER& other) = default;
			~PARAM_GROUP_HEADER() = default;

			QString _groupName;
		};

		struct COLOR_TRIPLET_SINGLE
		{
			COLOR_TRIPLET_SINGLE() = default;
			COLOR_TRIPLET_SINGLE(const COLOR_TRIPLET_SINGLE& other) = default;
			~COLOR_TRIPLET_SINGLE() = default;
			qint32	_iH = 0;
			qint32	_iS = 0;
			qint32	_iV = 0;

		};

		struct COLOR_TRIPLET
		{
			COLOR_TRIPLET() = default;
			COLOR_TRIPLET(const COLOR_TRIPLET& other) = default;
			~COLOR_TRIPLET() = default;
			COLOR_TRIPLET_SINGLE _min;
			COLOR_TRIPLET_SINGLE _max;
			std::string _colorName;
		};

		class BaseParameter : public QObject
		{
			Q_OBJECT 
		
		public:
			
			BaseParameter();
			virtual ~BaseParameter() = default;

			virtual QString	GetMetaClassFullName() const { return "AbstractParameter"; }
			virtual QString	GetMetaClassDebugName() const { return "AbstractParameter"; }

			virtual IPropertyList getPropertyList() const;
			virtual bool setPropertyList(const IPropertyList& vals);
			virtual QVariant getParamProperty(const QString& strValName) const;
			virtual bool setParamProperty(const QString& strValName, const QVariant& val);

			virtual QString serialize() { return ""; }

		signals:

			void propertyChanged(QString strPropName);
			void bulkChanged();
		};
	}
}
Q_DECLARE_METATYPE(QVector<QRect>)
Q_DECLARE_METATYPE(LandaJune::Parameters::PARAM_GROUP_HEADER)
Q_DECLARE_METATYPE(LandaJune::Parameters::COLOR_TRIPLET_SINGLE)
Q_DECLARE_METATYPE(LandaJune::Parameters::COLOR_TRIPLET)
Q_DECLARE_METATYPE(QVector<LandaJune::Parameters::COLOR_TRIPLET>)
Q_DECLARE_METATYPE(LandaJune::IPropertyTuple)
