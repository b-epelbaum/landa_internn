#pragma once

#include "paramlib_global.h"
#include <QVector>
#include <QRect>
#include "interfaces/type_usings.h"

#include <QJsonObject>
#include <utility>


#define DECLARE_PARAM_PROPERTY(x,type,initval,editable) Q_PROPERTY(type x MEMBER _##x READ x WRITE set##x USER editable) private: type _##x = initval; public: type x() const { return _##x; } void set##x(const type val) { _##x = val; if (editable) emit propertyChanged(#x); }

namespace LandaJune
{
	namespace Parameters
	{
		class BaseSerializeableParam : public QObject
		{
			Q_OBJECT
		
		public:
			BaseSerializeableParam() = default;
			BaseSerializeableParam(const QJsonObject& obj );
			BaseSerializeableParam(const BaseSerializeableParam& other) = default;
			~BaseSerializeableParam() = default;

			virtual std::string serialize();
			static bool deserialize(const QJsonObject& obj );

			signals:

			void propertyChanged(QString strPropName);

		private :

			using PROP_PAIR = std::pair<std::string, QVariant>;
			using PROP_LIST = std::vector<PROP_PAIR>;

			QJsonObject serialize(const PROP_LIST& propList) const;

		};

		class PARAM_GROUP_HEADER : public BaseSerializeableParam
		{
			Q_OBJECT
		public:
			PARAM_GROUP_HEADER() = default;
			PARAM_GROUP_HEADER(QString name) : _GroupName(std::move(name)) {}
			PARAM_GROUP_HEADER(const QJsonObject& obj ) : BaseSerializeableParam(obj) {}
			PARAM_GROUP_HEADER(const PARAM_GROUP_HEADER& other) : _GroupName(other.GroupName()) {}
			const PARAM_GROUP_HEADER & operator = (const PARAM_GROUP_HEADER & other )
			{
				_GroupName = other._GroupName;
				return *this;
			}
			~PARAM_GROUP_HEADER() = default;

			DECLARE_PARAM_PROPERTY(GroupName, QString, "Unknown", true)
		};
		Q_DECLARE_METATYPE(LandaJune::Parameters::PARAM_GROUP_HEADER)

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
Q_DECLARE_METATYPE(LandaJune::Parameters::COLOR_TRIPLET_SINGLE)
Q_DECLARE_METATYPE(LandaJune::Parameters::COLOR_TRIPLET)
Q_DECLARE_METATYPE(QVector<LandaJune::Parameters::COLOR_TRIPLET>)
Q_DECLARE_METATYPE(LandaJune::IPropertyTuple)
