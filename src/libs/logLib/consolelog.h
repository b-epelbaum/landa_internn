#pragma once

#ifdef WIN32
#	include <Windows.h>
#endif

#include <QVariant>
#include <QQueue>
#include <QFile>
#include <QThread>

#include "applog.h"
#include <utility>

namespace LandaJune
{
	namespace Loggers
	{

#ifdef WIN32
		class WinLogConsole;
#endif
		class ConsoleLog;

		class copyableContext
		{
		public:
			copyableContext() = default;
			explicit copyableContext(const QMessageLogContext& context) :
				_version(context.version)
				, _line(context.line)
				, _file(context.file)
				, _function(context.function)
				, _category(context.category)
			{}

			copyableContext(const copyableContext& other) : _version(other._version)
				, _line(other._line)
				, _file(other._file)
				, _function(other._function)
				, _category(other._category)
			{
			}

			copyableContext & operator = (const copyableContext & other)
			{
				_version = other._version;
				_line = other._line;
				_file = other._file;
				_function = other._function;
				_category = other._category;
				return *this;
			}

			~copyableContext() = default;

			int _version = 0;
			int _line = 0;
			QString _file;
			QString _function;
			QString _category;
		};

		Q_DECLARE_METATYPE(copyableContext);

		class DumpWorker : public QObject
		{
			Q_OBJECT

		public:
			explicit DumpWorker(ConsoleLog * parentLog);
			~DumpWorker() = default;

		public slots:

			void onData(int iStatus, const QString& dataLine);
			void abort();
			void startWorker();

		signals:

			void finished();

		private:

			struct logEntry
			{
				logEntry(const int iStatus, QString dataLine) :
					_iStatus(iStatus)
					, _strLine(std::move(dataLine)) {}

				int _iStatus;
				QString _strLine;
			};


			void process();
			void initFileHeaders() const;
			QString addHTMLLine(int dwStatus, QString strMsg) const;


			bool _bAborted = false;
			bool _bPaused = true;
			int _logFileSize = 0;
			int _iPartsCount = 0;

			QQueue<logEntry> _queue;
			ConsoleLog * _parentLog;

			QScopedPointer<QFile>			_logFile;
			QScopedPointer<QTextStream>		_textStream;

			QString _strCommonPath;
			QString _strNameForParts;

		};

		class ConsoleLog : public QObject
		{
			Q_OBJECT

				friend class DumpWorker;

		public:


			explicit ConsoleLog(QObject * parent = nullptr);
			virtual ~ConsoleLog();

			void InitGeneralLog(const QString& strFolder, const QString& strModule, bool bLogToConsole, bool bLogToFile, LOG_LEVEL logLevel, const QString& strPreffix);
			void Init() const;
			void Cleanup();
			void SetConsoleOut(const bool bConsole) { m_bIsConsole = bConsole; }
			bool GetConsoleOut() const
			{
				return m_bIsConsole;
			}
			void SetFileLogOut(bool bFileOut) { m_bFileOut = bFileOut; }
			bool GetFileLogOut() const
			{
				return m_bFileOut;
			}

#ifdef WIN32
			HANDLE getConsoleHandle() const;
#endif

			enum E_LOG_STATUS
			{
				S_LOG_INFO = 0,
				S_LOG_INFO1,
				S_LOG_INFO2,
				S_LOG_INFO3,
				S_LOG_INFO4,
				S_LOG_INFO5,
				S_LOG_INFO6,
				S_LOG_INFO7,
				S_LOG_INFO8,
				S_LOG_BRIGHT1 = 9,
				S_LOG_BRIGHT2,
				S_LOG_BRIGHT3,
				S_LOG_BRIGHT4,
				S_LOG_BRIGHT5,
				S_LOG_DEBUG,
				S_LOG_WARNING,
				S_LOG_ERROR,
			};


			void AddLogEntry(const QString& msg, const E_LOG_STATUS status, const QMessageLogContext& context);

		signals:

			void data(int iStatus, QString strData);
			void abortRequest();

		private slots:

			void AddLine(int dwStatus, const QString& msg, const copyableContext& context);

		protected:

			void AddOther(const QtMsgType type, const copyableContext& context, const QString& msg);
			void AddConsoleLine(int dwStatus, const QString& msg);

			void startDumpThread();
			void stopDumpThread();


			bool m_bInitialized = false;
			LOG_LEVEL _logLevel = LOG_LEVEL_NONE;

#ifdef WIN32
			WinLogConsole  * m_console = nullptr;
#endif

			bool m_bShowTime = false;
			bool m_bShowThreadID = false;
			bool m_bShowFile = false;
			bool m_bShowLine = false;

			bool m_bIsConsole = false;
			bool m_bFileOpen = false;
			short m_addInfoColor = 0;
			bool m_bFileOut = false;

			QScopedPointer<QThread>		_pDumpThread;
			QScopedPointer<DumpWorker>	_pDumpWorker;

			QString _strFolder;
			QString _strModuleName;
			QString _strOutputDebugPreffix;

			QScopedPointer<QFile>			_logFile;
			QScopedPointer<QTextStream>		_textStream;

			CRITICAL_SECTION _consoleCs;

			Qt::HANDLE _threadId; 
		};
	}
}
	