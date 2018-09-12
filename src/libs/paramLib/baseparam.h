#pragma once

#include "paramlib_global.h"
#include <QVector>
#include <QRect>
#include "common/type_usings.h"
#include <string>

#include <QJsonObject>
#include <utility>

// FLAG USER = true means parameter is editable, USER = false - parameter is readonly

#define DECLARE_PARAM_PROPERTY(x,type,initval,editable) Q_PROPERTY(type x MEMBER _##x READ x WRITE set##x USER editable) private: type _##x = initval; public: type x() const { return _##x; } void set##x(const type val) { _##x = val; }

namespace LandaJune
{
	namespace Parameters
	{
		class BaseParameters : public QObject
		{
			Q_OBJECT
		
		public:
			BaseParameters();
			explicit BaseParameters(const QJsonObject& obj );
			BaseParameters(const BaseParameters& other);
			virtual ~BaseParameters() = default;

			virtual QString	GetMetaClassFullName() const { return "BaseParameters"; }
			virtual QString	GetMetaClassDebugName() const { return "BaseParameters"; }

			virtual IPropertyTuple getPropertyTuple(const QString& strValName) const;
			virtual IPropertyList getEditablePropertyList() const;
			virtual IPropertyList getReadOnlyPropertyList() const;

			virtual bool setPropertyList(const IPropertyList& vals);
			
			virtual QVariant getParamProperty(const QString& strValName) const;
			virtual bool setParamProperty(const QString& strValName, const QVariant& val);

			virtual bool load (QString fileName, QString& error);
			virtual QJsonObject toJson();
			virtual bool fromJson(const QJsonObject& obj, bool bRootObject, QString& error );

			signals:

				void updateCalculated();
				void loaded();

		protected :

			virtual IPropertyList getPropertyList(bool bReadOnly) const;
			virtual void recalculate () {}
			inline static bool _metaTypeRegistered = false;
			using OBJ_PROP_PAIR = QPair<QString, QVariant>;
			using OBJ_PROP_LIST = QVector<OBJ_PROP_PAIR>;

		};
		
		class PARAM_GROUP_HEADER : public BaseParameters
		{
			Q_OBJECT

		public:
			PARAM_GROUP_HEADER() = default;
			PARAM_GROUP_HEADER(const PARAM_GROUP_HEADER& other) = default;
			explicit PARAM_GROUP_HEADER(const QJsonObject& obj ) : BaseParameters(obj) {}
			PARAM_GROUP_HEADER(const char* name) 
				: _GroupName(QString(name)) {}
			explicit PARAM_GROUP_HEADER(QString name) 
				: _GroupName(std::move(name)) {}
			
			PARAM_GROUP_HEADER & operator = (const PARAM_GROUP_HEADER & other )
			{
				_GroupName = other._GroupName;
				return *this;
			}
			~PARAM_GROUP_HEADER() = default;
			DECLARE_PARAM_PROPERTY(GroupName, QString, "Unknown", true)
		};

		class COLOR_TRIPLET_SINGLE : public BaseParameters
		{
			Q_OBJECT

		public:
			COLOR_TRIPLET_SINGLE() = default;
			COLOR_TRIPLET_SINGLE(qint32 H, qint32 S, qint32 V, const QString& color) 
				: _H(H)
				, _S(S)
				, _V(V)
				, _ColorName(color)
			{}

			COLOR_TRIPLET_SINGLE(const QJsonObject& obj ) : BaseParameters(obj) {}
			COLOR_TRIPLET_SINGLE(const COLOR_TRIPLET_SINGLE& other)  = default;
			COLOR_TRIPLET_SINGLE & operator = (const COLOR_TRIPLET_SINGLE & other )
			{
				_H = other._H;
				_S = other._S;
				_V = other._V;
				return *this;
			}
			~COLOR_TRIPLET_SINGLE() = default;
			
			QString toDisplayString() const noexcept
			{
				return QString("(%1,%2,%3)").arg(QString::number(_H)
					, QString::number(_S)
					, QString::number(_V));
			}

			DECLARE_PARAM_PROPERTY(H, qint32, 0, true)
			DECLARE_PARAM_PROPERTY(S, qint32, 0, true)
			DECLARE_PARAM_PROPERTY(V, qint32, 0, true)
			DECLARE_PARAM_PROPERTY(ColorName, QString, "", true)
		};

		class COLOR_TRIPLET: public BaseParameters
		{
			Q_OBJECT
		
		public:
			COLOR_TRIPLET() = default;
			COLOR_TRIPLET(const COLOR_TRIPLET_SINGLE& min, const COLOR_TRIPLET_SINGLE& max, const QString& colorName ) 
				: _Min(min)
				, _Max(max)
				, _ColorName(colorName)
			{}

			COLOR_TRIPLET(const QJsonObject& obj ) : BaseParameters(obj) {}
			COLOR_TRIPLET(const COLOR_TRIPLET& other) 
				: _Min (other._Min)
				, _Max (other._Max)
				, _ColorName(other._ColorName)
				{}
			
			COLOR_TRIPLET & operator = (const COLOR_TRIPLET & other )
			{
				_Min = other._Min;
				_Max = other._Max;
				_ColorName = other._ColorName;
				return *this;
			}
			~COLOR_TRIPLET() = default;

			QString toDisplayString() const noexcept
			{
				return "["+_Min.toDisplayString() + " , " + _Max.toDisplayString() + "]";
			}

			DECLARE_PARAM_PROPERTY(Min, COLOR_TRIPLET_SINGLE, {}, true)
			DECLARE_PARAM_PROPERTY(Max, COLOR_TRIPLET_SINGLE, {}, true)
			DECLARE_PARAM_PROPERTY(ColorName, QString, "", true)
		};
	}
}
Q_DECLARE_METATYPE(QVector<QRect>)
Q_DECLARE_METATYPE(LandaJune::Parameters::BaseParameters)
Q_DECLARE_METATYPE(LandaJune::Parameters::PARAM_GROUP_HEADER)
Q_DECLARE_METATYPE(LandaJune::Parameters::COLOR_TRIPLET_SINGLE)
Q_DECLARE_METATYPE(LandaJune::Parameters::COLOR_TRIPLET)
Q_DECLARE_METATYPE(QVector<LandaJune::Parameters::COLOR_TRIPLET>)
Q_DECLARE_METATYPE(LandaJune::IPropertyTuple)
