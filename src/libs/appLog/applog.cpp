#include "applog.h"
#include "consolelog.h"

#include <QDir>
#include <QCoreApplication>
#include <QDateTime>
#include <QTextStream>
#include <QMutexLocker>
#include <QFile>

using namespace LandaJune::Loggers;

Q_LOGGING_CATEGORY(logDebug, "Debug");
Q_LOGGING_CATEGORY(logInfo, "Info");
Q_LOGGING_CATEGORY(logInfo1, "Info1");
Q_LOGGING_CATEGORY(logInfo2, "Info2");
Q_LOGGING_CATEGORY(logInfo3, "Info3");
Q_LOGGING_CATEGORY(logInfo4, "Info4");
Q_LOGGING_CATEGORY(logInfo5, "Info5");
Q_LOGGING_CATEGORY(logInfo6, "Info6");
Q_LOGGING_CATEGORY(logInfo7, "Info7");
Q_LOGGING_CATEGORY(logInfo8, "Info8");
Q_LOGGING_CATEGORY(logBright1, "Bright1");
Q_LOGGING_CATEGORY(logBright2, "Bright2");
Q_LOGGING_CATEGORY(logBright3, "Bright3");
Q_LOGGING_CATEGORY(logBright4, "Bright4");
Q_LOGGING_CATEGORY(logBright5, "Bright5");
Q_LOGGING_CATEGORY(logWarning, "Warning");
Q_LOGGING_CATEGORY(logCritical, "Critical");

QSharedPointer<AppLogger> AppLogger::_this;

typedef double (AppLogger::*loggerFunc)(const QString&, int, const QMessageLogContext&);
typedef QMap<std::string, ConsoleLog::E_LOG_STATUS> LOGGER_PARAM_MAP;

static LOGGER_PARAM_MAP loggerParamMap =
{
	  {"Info", ConsoleLog::S_LOG_INFO }
	, {"Info1", ConsoleLog::S_LOG_INFO1 }
	,{ "Info2", ConsoleLog::S_LOG_INFO2 }
	,{ "Info3", ConsoleLog::S_LOG_INFO3 }
	,{ "Info4", ConsoleLog::S_LOG_INFO4 }
	,{ "Info5", ConsoleLog::S_LOG_INFO5 }
	,{ "Info6", ConsoleLog::S_LOG_INFO6 }
	,{ "Info7", ConsoleLog::S_LOG_INFO7 }
	,{ "Info8", ConsoleLog::S_LOG_INFO8 }
	,{ "Bright1", ConsoleLog::S_LOG_BRIGHT1 }
	,{ "Bright2", ConsoleLog::S_LOG_BRIGHT2 }
	,{ "Bright3", ConsoleLog::S_LOG_BRIGHT3 }
	,{ "Bright4", ConsoleLog::S_LOG_BRIGHT4 }
	,{ "Bright5", ConsoleLog::S_LOG_BRIGHT5 }
	,{ "Debug", ConsoleLog::S_LOG_DEBUG }
	,{ "Warning", ConsoleLog::S_LOG_WARNING }
	,{ "Critical", ConsoleLog::S_LOG_ERROR }
};

QSharedPointer<AppLogger> AppLogger::createLogger()
{
	if (!_this.isNull())
		return _this;

	_this = QSharedPointer<AppLogger>::create();
	_this->installMessageHandler();

	return _this;
}

void AppLogger::closeLogger()
{
	_this->_consoleLog->Cleanup();
	_this.clear();
}

QSharedPointer<AppLogger> AppLogger::get()
{
	return createLogger();
}

void AppLogger::installMessageHandler()
{
	const auto logFolder = qApp->applicationDirPath() + QDir::separator() + "Logs" + QDir::separator();
	QDir logPath(logFolder);
	if (!logPath.exists())
	{
		Q_UNUSED(logPath.mkpath("."));
	}

	_consoleLog.reset(new ConsoleLog());
	_consoleLog->InitGeneralLog(
		logFolder
		, "log_"
		, true
		, true
		, LOG_LEVEL_ALL
		, "debug");

	qInstallMessageHandler(appMessageHandler);
}

void AppLogger::appMessageHandler(const QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	if (_this.isNull())
		return;

	_this->_consoleLog->AddLogEntry(msg, loggerParamMap.value(context.category), context);
}

AppLogger::AppLogger(QObject* parent)
{
	Q_UNUSED(parent);
	_mutex.reset(new QMutex(QMutex::Recursive));
}

AppLogger::~AppLogger()
{
	_consoleLog->Cleanup();
}