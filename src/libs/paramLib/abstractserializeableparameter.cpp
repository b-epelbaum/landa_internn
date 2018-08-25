#include "abstractserializeableparameter.h"

#include <QMetaProperty>
#include <QJsonDocument>

using namespace LandaJune;
using namespace Parameters;


std::string AbstractSerializeableParameter::serialize()
{
	PROP_LIST propList;
	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i)
	{
		auto metaproperty = metaobject->property(i);
		const auto name = metaproperty.name();

		if (strcmp(name, "objectName") != 0)
		{
			propList.emplace_back( PROP_PAIR{ std::string(name), property(name) });
		}
	}

	QJsonDocument doc(serialize(propList));
	return QString(doc.toJson(QJsonDocument::Indented)).toStdString();
}

bool AbstractSerializeableParameter::deserialize(const QJsonObject& obj)
{
	return true;
}

QJsonObject AbstractSerializeableParameter::serialize(const PROP_LIST& propList) const
{
	auto retObj = QJsonObject();
	std::for_each(propList.begin(), propList.end(), 
		[&retObj] (auto valPair)
	{
		retObj[QString::fromStdString(valPair.first)] = valPair.second.toJsonValue();
	});

	return std::move(retObj);
}