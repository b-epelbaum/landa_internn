#include "BaseCore.h"
#include "applog.h"

#include <QDirIterator>
#include <QPluginLoader>
#include <QCoreApplication>

#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>

#pragma comment(lib, "appLog.lib")

using namespace LandaJune::Core;
using namespace LandaJune::Parameters;
using namespace LandaJune::FrameProviders;

#define CLASSNAME(x) qPrintable(x->GetMetaClassDebugName())
#define MYCLASSNAME qPrintable(GetMetaClassDebugName())

#define CORE_SCOPED_LOG PRINT_INFO2 << "[Core] : "
#define CORE_SCOPED_ERROR PRINT_ERROR << "[Core] : "
#define CORE_SCOPED_WARNING PRINT_WARNING << "[Core] : "

#define CHECK_IF_INITED if (!_bInited ) \
{ \
	throw CoreEngineException(CORE_ENGINE_ERROR::ERR_CORE_NOT_INITIALIZED, ""); \
}


void BaseCore::loadDefaultConfiguration()
{
}

void BaseCore::loadConfiguration(QIODevice& strJSONFile)
{
}

void BaseCore::loadConfiguration(QString strJSON)
{
}

void BaseCore::init()
{
}

void BaseCore::cleanup()
{
}

const QList<ICore::FrameProviderPtr>& BaseCore::getFrameProviderList() const
{
	return _providerList;
}

ICore::ProcessParameterPtr BaseCore::getProcessParameters()
{
	return _processParameters;
}

void BaseCore::selectFrameProvider(FrameProviderPtr provider)
{
}

ICore::FrameProviderPtr BaseCore::getSelectedFrameProvider() const
{
	return _currentFrameProvider;
}

void BaseCore::start() const
{
}

void BaseCore::stop() const
{
}

bool BaseCore::isBusy()
{
	return false;
}

QString BaseCore::getDefaultConfigurationFileName() const
{
	return "";
}

void BaseCore::saveConfiguration()
{
}

void BaseCore::initGlobalParameters()
{
}

void BaseCore::initFramePool() const
{
}

void BaseCore::initProviders()
{
}
