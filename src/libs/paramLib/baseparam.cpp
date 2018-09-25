#include "baseparam.h"
#include <QMetaProperty>
#include <QJsonArray>
#include <QFile>
#include "applog.h"
#include <QJsonDocument>

#define PARAM_SCOPED_LOG PRINT_INFO7 << "[BaseParameters] : "
#define PARAM_SCOPED_ERROR PRINT_ERROR << "[BaseParameters] : "
#define PARAM_SCOPED_WARNING PRINT_WARNING << "[BaseParameters] : "


using namespace LandaJune;
using namespace Parameters;

using serializeFunction = std::function<QString(BaseParameters*)>;

static std::map<int, serializeFunction> typesMap;

BaseParameters::BaseParameters()
{
	if (_metaTypeRegistered )
		return;

	qRegisterMetaType<BaseParameters>("BaseParameters");
	qRegisterMetaType<PARAM_GROUP_HEADER>("PARAM_GROUP_HEADER");
	qRegisterMetaType<COLOR_TRIPLET>("COLOR_TRIPLET");
	qRegisterMetaType<COLOR_TRIPLET_SINGLE>("COLOR_TRIPLET_SINGLE");
	
	qRegisterMetaType<QVector<QRect>>("QVector<QRect>");
	qRegisterMetaType<QVector<COLOR_TRIPLET>>("QVector<COLOR_TRIPLET>");
	_metaTypeRegistered = true;

}

BaseParameters::BaseParameters(const BaseParameters& other)
{
	const auto metaobject = other.metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i)
	{
		auto metaproperty = metaobject->property(i);
		const auto name = metaproperty.name();
		setProperty(name, other.property(name));
	}
}

QJsonObject BaseParameters::toJson()
{
	QJsonObject retVal;
	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i)
	{
		auto metaproperty = metaobject->property(i);
		const auto name = metaproperty.name();
		const auto& var = property(name); 
		if (strcmp(name, "objectName") != 0 && metaproperty.isUser() )
		{
			if (var.userType() > 100 ) // user custom type
		{
			if (var.canConvert<COLOR_TRIPLET_SINGLE>())
			{
				retVal[name] = var.value<COLOR_TRIPLET_SINGLE>().toJson();
			}
			else if (var.canConvert<COLOR_TRIPLET>())
			{
				retVal[name] = var.value<COLOR_TRIPLET>().toJson();
			}
			else if (var.canConvert<QVector<COLOR_TRIPLET>>())
			{
				QJsonArray arr;
				auto colorArray = var.value<QVector<COLOR_TRIPLET>>();
				for(auto color: colorArray)
				{
					arr.append(color.toJson());
				}
				retVal[name] = arr;
			}
		}
		else
			retVal[name] = var.toJsonValue();
			
		}
	}
	return retVal;
}

bool BaseParameters::fromJson(const QJsonObject& obj, bool bRootObject, QString& error )
{
	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i)
	{
		const auto& metaProp = metaobject->property(i);
		const auto propName = metaProp.name();
		const auto typeName = metaProp.typeName();
		if (strcmp(propName, "objectName") == 0 || !metaProp.isUser() )
			continue;
			
		if ( !obj.contains(propName) && strcmp(typeName, "PARAM_GROUP_HEADER") != 0 )
		{
			PARAM_SCOPED_WARNING << "Required parameter " << propName << " has not been found";
			continue;
		}

		const auto& paramObj = obj[propName].toObject();
		if (strcmp(typeName, "COLOR_TRIPLET_SINGLE") == 0 )
		{
			COLOR_TRIPLET_SINGLE t;
			if ( t.fromJson(paramObj, false, error) )
			{
				(void)setProperty(propName, QVariant::fromValue(t));
			}
			else
			{
				PARAM_SCOPED_ERROR << "Error while parsing parameter"  << propName;
			}
		}
		else if (strcmp(typeName, "COLOR_TRIPLET" ) == 0 )
		{
			COLOR_TRIPLET t;
			if ( t.fromJson(paramObj, false, error) )
			{
				(void)setProperty(propName, QVariant::fromValue(t));
			}
			else
			{
				PARAM_SCOPED_ERROR << "Error while parsing parameter"  << propName;
			}
		}
		else if (strcmp(typeName, "QVector<COLOR_TRIPLET>") == 0 )
		{
			auto arr = obj[propName].toArray();
			QVector<COLOR_TRIPLET> t;
			for ( auto j = 0; j < arr.count(); j++ )
			{
				COLOR_TRIPLET color;
				if ( color.fromJson(arr.at(j).toObject(), false, error) )
				{
					t << color;
				}
				else
				{
					PARAM_SCOPED_ERROR << "Error while parsing parameter"  << propName;
				}
			}
			(void)setProperty(propName, QVariant::fromValue(t));
		}
		else
		{
			auto srcVar = obj[propName].toVariant();
			if ( !srcVar.isNull() )
			{
				setProperty(propName, srcVar);
			}
		}
	}

	if (bRootObject)
	{
		emit loaded();
		recalculate();
	}
	return true;
}

IPropertyList BaseParameters::getEditablePropertyList() const
{
	return getPropertyList(false);
}

IPropertyList BaseParameters::getReadOnlyPropertyList() const
{
	return getPropertyList(true);
}

IPropertyList BaseParameters::getPropertyList(bool bReadOnly) const
{
	IPropertyList retVal;
	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i)
	{
		auto metaproperty = metaobject->property(i);
		const auto name = metaproperty.name();
		if (strcmp(name, "objectName") != 0 && metaproperty.isUser() != bReadOnly)
			retVal.push_back(IPropertyTuple(name, property(name), metaproperty.isUser()));
	}
	return retVal;
}

IPropertyTuple BaseParameters::getPropertyTuple(const QString& strValName) const
{
	IPropertyTuple retVal;
	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i)
	{
		auto metaproperty = metaobject->property(i);
		const auto name = metaproperty.name();
		if (strcmp(name, strValName.toLocal8Bit()) == 0 )
		{
			return IPropertyTuple(name, property(name), metaproperty.isUser());
		}
	}
	return retVal;
}

bool BaseParameters::setPropertyList(const IPropertyList& vals)
{
	return false;
}

QVariant BaseParameters::getParamProperty(const QString& strValName) const
{
	QVariant retVal;
	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i)
	{
		auto metaproperty = metaobject->property(i);
		const auto name = metaproperty.name();
		if (strValName == name)
			return property(name);
	}
	return QVariant{};
}

bool BaseParameters::setParamProperty(const QString& strValName, const QVariant& val)
{
	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i)
	{
		auto metaproperty = metaobject->property(i);
		const auto name = metaproperty.name();
		if (strValName == name)
		{
			const auto retVal = setProperty(name, val);
			if (retVal)
			{
				recalculate();
				return retVal;
			}
		}
	}
	return false;
}

bool BaseParameters::load(QString fileName, QString& error)
{
	QFile jsonFile(fileName);
	if (!jsonFile.open(QFile::ReadOnly) )
	{
		error = jsonFile.errorString();
		return false;
	}

	_jsonFilePath = fileName;
	QJsonParseError pError;
	auto const doc = QJsonDocument::fromJson(jsonFile.readAll(), &pError);
	if (pError.error != QJsonParseError::NoError )
	{
		error = pError.errorString();
		return false;
	}

	return fromJson(doc.object(), true, error);

}
