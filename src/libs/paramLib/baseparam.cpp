#include "baseparam.h"
#include <QMetaProperty>

using namespace LandaJune;
using namespace Parameters;

IPropertyList BaseParameter::getPropertyList() const
{
	IPropertyList retVal;
	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i)
	{
		auto metaproperty = metaobject->property(i);
		const auto name = metaproperty.name();
		if (strcmp(name, "objectName") != 0)
			retVal.push_back(IPropertyPair(name, property(name)));
	}
	return retVal;
}

bool BaseParameter::setPropertyList(const IPropertyList& vals)
{
	return false;
}

QVariant BaseParameter::getParamProperty(const QString& strValName) const
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

bool BaseParameter::setParamProperty(const QString& strValName, const QVariant& val)
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
				emit propertyChanged(name);
				//saveConfiguration();
				return retVal;
			}
		}
	}
	return false;
}
