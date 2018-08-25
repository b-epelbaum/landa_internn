#include "BaseFrameProvider.h"

#include <QMetaProperty>

using namespace LandaJune;
using namespace FrameProviders;
using namespace Parameters;

#define CLASSNAME(x) qPrintable(x->GetMetaClassDebugName())
#define MYCLASSNAME qPrintable(GetMetaClassDebugName())

#define BASEPROVIDER_SCOPED_LOG PRINT_INFO2 << "[BaseFrameProvider] : "
#define BASEPROVIDER_SCOPED_ERROR PRINT_ERROR << "[BaseFrameProvider] : "
#define BASEPROVIDER_SCOPED_WARNING PRINT_WARNING << "[BaseFrameProvider] : "

/*
void BaseFrameProvider::loadDefaultConfiguration()
{
	auto& strPath = qApp->applicationDirPath() + QDir::separator() + getDefaultConfigurationFileName();
	if ( !QFileInfo(strPath).exists())
	{
		throw ProviderException(FRAME_PROVIDER_ERROR::ERR_PROVIDER_CONFIG_FILE_DOES_NOT_EXIST, "Configuration file " + strPath.toStdString() + " does not exist");
	}
	QFile jsFile(strPath);
	if ( jsFile.open(QIODevice::ReadOnly | QIODevice::Text) )
		loadConfiguration(jsFile);
	else
		throw ProviderException(FRAME_PROVIDER_ERROR::ERR_PROVIDER_INVALID_CONFIG_FILE, "Cannot open configuration file " + strPath.toStdString());
}

void BaseFrameProvider::loadConfiguration(QIODevice& strJSONFile)
{
	loadConfiguration(strJSONFile.readAll());
}

void BaseFrameProvider::loadConfiguration(QString strJSON)
{
	QJsonParseError error{};
	auto jDoc = QJsonDocument::fromJson(strJSON.toUtf8(), &error);
	if (jDoc.isNull())
	{
		throw ProviderException(FRAME_PROVIDER_ERROR::ERR_PROVIDER_INVALID_CONFIG_FILE,
			"Provider Configuration JSON has invalid format; error :  " + error
			.errorString().
			toStdString());
	}

	auto rootObject = jDoc.object();
	if (rootObject.isEmpty())
	{
		throw ProviderException(FRAME_PROVIDER_ERROR::ERR_PROVIDER_INVALID_CONFIG_FILE, "Document root object is empty");
	}
	
	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i<count; ++i)
	{
		auto metaproperty = metaobject->property(i);
		const auto name = metaproperty.name();

		if (strcmp(name, "objectName") != 0)
		{
			auto jVal = rootObject[name];
			if (jVal.isNull())
				continue;
			setProperty(name, jVal.toVariant());
		}
	}
}
*/

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

bool BaseFrameProvider::setProviderProperties(const IPropertyList& vals)
{
	return !_busy;
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

/*
void BaseFrameProvider::saveConfiguration()
{
	auto dumpObject = QJsonObject();
	dumpObject["ProviderName"] = getName();
	dumpObject["ProviderDesc"] = getDescription();

	const auto metaobject = metaObject();
	const auto count = metaobject->propertyCount();
	for (auto i = 0; i < count; ++i)
	{
		auto metaproperty = metaobject->property(i);
		const auto& name = metaproperty.name();
		if (strcmp(name, "objectName") != 0)
			dumpObject[name] = property(name).toJsonValue();
	}

	const auto& savePath = qApp->applicationDirPath() + QDir::separator() + getDefaultConfigurationFileName();
	QFile saveFile(savePath);
	if (saveFile.open(QIODevice::WriteOnly))
	{
		QJsonDocument doc(dumpObject);
		saveFile.write(doc.toJson(QJsonDocument::Indented));
	}
}
*/