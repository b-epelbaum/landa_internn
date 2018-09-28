#pragma once

#include "applog_global.h"
#include <QLoggingCategory>
#include <QSharedPointer>

class QMutex;
class QFile;

enum LOG_LEVEL
{
	LOG_LEVEL_NONE = 0
	, LOG_LEVEL_ERRORS_ONLY = 1
	, LOG_LEVEL_ERRORS_AND_WARNINGS = 2
	, LOG_LEVEL_ALL = 100
};


APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logDebug)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logInfo)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logInfo1)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logInfo2)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logInfo3)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logInfo4)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logInfo5)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logInfo6)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logInfo7)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logInfo8)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logBright1)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logBright2)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logBright3)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logBright4)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logBright5)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logWarning)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logCritical)
APPLOG_EXPORT Q_DECLARE_LOGGING_CATEGORY(logCriticalDetails)


namespace LandaJune
{
	namespace Loggers
	{
		class ConsoleLog;

#define PRINT_MESSAGE(x)	AppLogger::get()->printLogMessage(x)
#define PRINT_DEBUG		qDebug(logDebug()) 
#define PRINT_INFO		qDebug(logInfo()) 
#define PRINT_INFO1		qDebug(logInfo1()) 
#define PRINT_INFO2		qDebug(logInfo2()) 
#define PRINT_INFO3		qDebug(logInfo3()) 
#define PRINT_INFO4		qDebug(logInfo4()) 
#define PRINT_INFO5		qDebug(logInfo5()) 
#define PRINT_INFO6		qDebug(logInfo6()) 
#define PRINT_INFO7		qDebug(logInfo7()) 
#define PRINT_INFO8		qDebug(logInfo8())
#define PRINT_BRIGHT1	qDebug(logBright1())
#define PRINT_BRIGHT2	qDebug(logBright2())
#define PRINT_BRIGHT3	qDebug(logBright3())
#define PRINT_BRIGHT4	qDebug(logBright4())
#define PRINT_BRIGHT5	qDebug(logBright5())
#define PRINT_WARNING	qDebug(logWarning()) 
#define PRINT_ERROR		qDebug(logCritical()) 
#define PRINT_ERROR_DETAILS		qDebug(logCriticalDetails()) 

#define PRINT_DEBUG_BREAK		PRINT_DEBUG		<< "\r\n";
#define PRINT_DEBUG_LINE		PRINT_DEBUG		<< " : ---------------------------------------------------------------------";
#define PRINT_DEBUG_DBLINE		PRINT_DEBUG		<< " : =====================================================================";
#define PRINT_DEBUG_PLUSLINE	PRINT_DEBUG		<< " : +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
#define PRINT_DEBUG_TIME		PRINT_DEBUG		<< QTime::currentTime();

		class ConsoleLog;

		class APPLOG_EXPORT AppLogger : public QObject
		{
			Q_OBJECT
				friend class QSharedPointer<AppLogger>;

		public:
			virtual ~AppLogger();

			static LOG_LEVEL stringToLevel (const QString& levelStr, bool& bOk);
			static QSharedPointer<AppLogger> createLogger(LOG_LEVEL logLevel,  bool bLogToFile);
			static void closeLogger();
			static QSharedPointer<AppLogger> get();
			QString getLogFilePath() const { return _logFilePath; }

		private:
			explicit AppLogger(LOG_LEVEL logLevel = LOG_LEVEL_ERRORS_ONLY,  bool bLogToFile = false, QObject* parent = Q_NULLPTR);

			void installMessageHandler();
			static void appMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

			LOG_LEVEL							_logLevel;
			bool								_bLogToFile = false;
			QString								_logFilePath;
			QScopedPointer<QMutex>				_mutex;
			static QSharedPointer<AppLogger>	_this;
			QScopedPointer<ConsoleLog>			_consoleLog;
		};
	}
}