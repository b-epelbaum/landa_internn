#include "BaseFrameProvider.h"
#include "baseparam.h"
#include <QMetaProperty>

using namespace LandaJune;
using namespace FrameProviders;
using namespace Parameters;

#define CLASSNAME(x) qPrintable(x->GetMetaClassDebugName())
#define MYCLASSNAME qPrintable(GetMetaClassDebugName())

#define BASEPROVIDER_SCOPED_LOG PRINT_INFO2 << "[BaseFrameProvider] : "
#define BASEPROVIDER_SCOPED_ERROR PRINT_ERROR << "[BaseFrameProvider] : "
#define BASEPROVIDER_SCOPED_WARNING PRINT_WARNING << "[BaseFrameProvider] : "


IPropertyList BaseFrameProvider::getProviderProperties() const
{
	IPropertyList retVal;
	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i) 
	{
		auto metaproperty = metaobject->property(i);
		const auto& name = metaproperty.name();
		if ( strcmp(name, "objectName") != 0)
			retVal.push_back(IPropertyTuple(name, property(name), metaproperty.isUser()));
	}
	return retVal;
}

QVariant BaseFrameProvider::getProviderProperty(const QString& strValName) const
{
	QVariant retVal;
	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i)
	{
		auto metaproperty = metaobject->property(i);
		const auto name = metaproperty.name();
		if ( strValName == name )
			return property(name);
	}
	return QVariant{};
}

bool BaseFrameProvider::setProviderProperty(const QString& strValName, const QVariant& val)
{
	if (_busy)
		return false;
	
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
				return retVal;
			}
		}
	}
	return false;
}


void BaseFrameProvider::onUpdateParameters()
{
	validateParameters(_providerParameters);
}
