#include "consolelog.h"

#ifdef WIN32
#	include <tchar.h>
#	include <Windows.h>
#else
#   include <iostream>
#endif

#include <sstream>
#include <QDateTime>
#include <QCoreApplication>
#include "winconsole.h"

using namespace LandaJune::Loggers;

// log max size
#define MAX_LOG_FILE_SIZE	(1024 * 1024 *3) // 3 MB

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



static const char* classesnames[] = 
{
	    "info"
	  , "info1"
	  , "info2"
	  , "info3"
	  , "info4"
	  , "info5"
	  , "info6"
	  , "info7"
	  , "info8"
	  , "bright1"
	  , "bright2"
	  , "bright3"
	  , "bright4"
	  , "bright5"
	  , "debug"
	  , "warning"
	  , "error"

};

#ifdef WIN32
short wStatusColors[] = {
	// infos
	15,//info
	4,	//info1
	14,	//info2
	5,	//info3
	6,	//info4
	9,	//info5
	12,	//info6
	13,	//info7
	10,	//info8
	0x79,	//bright1
	0x9E,	//bright2
	0xC9,		//bright3
	0xF9,	//bright4
	0x1E,	//bright5
	7,		// debug
	224,	// warning
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_INTENSITY		// error
};
#else
std::string wStatusColors[] = {
    // infos
    "\033[1;37m",            //info
    "\033[0;31m",	//info1
    "\033[0;32m",	//info2
    "\033[0;33m",	//info3
    "\033[0;34m",	//info4
    "\033[0;35m",	//info5
    "\033[0;36m",	//info6
    "\033[0;36m",	//info7
    "\033[1;34;42m",	//info8
    "\033[0;37m",	//bright1
    "\033[0;37m",	//bright2
    "\033[0;37m",	//bright3
    "\033[0;37m",	//bright4
    "\033[0;37m",	//bright5
    "\033[0;37m",	// debug
    "\033[1;30;43m",	// warning
    "\033[1;31;47m",    // error
};
#endif
std::string htmlStatusColorsDark[] = {
	// infos
	"#FFFFFF",			// info
	"#800000", 	//info1
	"#ffdb02",	//info2
	"#800080",	//info3
	"#808000",	//info4
	"#0000ff",	//info5
	"#ff0000",	//info6
	"#ff00ff",	//info7
	"#ff00ff",	//info8
	"#808000",	//bright1
	"#0000ff",	//bright2
	"#ff0000",	//bright3
	"#ff00ff",	//bright4
	"#ff00ff",	//bright5
	"#C0C0C0",			// debug
	"#000000",			// warning
	"#FFFFFF"			// error
};


QString htmlStatusColorsLight[] = {
	// infos
	"#000000",	// info
	"#74ab21", 	//info1
	"#0011ff",	//info2
	"#666600",	//info3
	"#6f7672",	//info4
	"#a64dff",	//info5
	"#b30059",	//info6
	"#3b6009",	//info7
	"#00b3b3",	//info8
	"#808000",	//bright1
	"#0000ff",	//bright2
	"#ff0000",	//bright3
	"#ff00ff",	//bright4
	"#ff00ff",	//bright5
	"#C0C0C0",			// debug
	"#000000",			// warning
	"#FFFFFF",			// error
	"#FFFFFF",			// error with details

};

QString colorBackgroundBodyDark = "#101010";
QString colorBackgroundBodyLight = "#FAFAFA";

QString colorBackgroundWarningDark = "#FFFF00";
QString colorBackgroundWarningLight = "#F0F0F0";

QString colorBackgroundErrorDark = "#FF0000";
QString colorBackgroundErrorLight = "#ff0000";

using namespace LandaJune::Loggers;

DumpWorker::DumpWorker(ConsoleLog* parentLog) : QObject(nullptr), _parentLog(parentLog)
{
	const auto strTime = QDateTime::currentDateTime().toString("_yyMMdd_hhmmss");
#ifdef WIN32
	_strCommonPath = QString("%1\\%2_log_%3.html").arg(_parentLog->_strFolder).arg(_parentLog->_strModuleName).arg(strTime);
	_strNameForParts = QString("%1\\%2_log_%3").arg(_parentLog->_strFolder).arg(_parentLog->_strModuleName).arg(strTime);
#else
	_strCommonPath = QString("%1/%2_log_%3.html").arg(_parentLog->_strFolder).arg(_parentLog->_strModuleName).arg(strTime);
	_strNameForParts = QString("%1/%2_log_%3").arg(_parentLog->_strFolder).arg(_parentLog->_strModuleName).arg(strTime);
#endif


	_logFile.reset(new QFile(_strCommonPath));
	_textStream.reset(new QTextStream());
}


void DumpWorker::startWorker()
{
	if (_logFile->open(QFile::ReadWrite | QFile::Text))
	{
		_textStream->setDevice(_logFile.data());
		_parentLog->m_bFileOpen = true;
	}
	initFileHeaders();
}


void DumpWorker::initFileHeaders() const
{
	const auto szWarningColor = colorBackgroundWarningDark;
	const auto szErrorColor = colorBackgroundErrorDark;

	*_textStream << "<!DOCTYPE html>\r\n";
	*_textStream << "<html lang=\"en-US\">\r\n";
	*_textStream << "<head>\r\n";
	*_textStream << "<style>\r\n";
	*_textStream << ".body_dark { font-family: monospace;  font-size: 14px; margin: 0px; background-color : " << colorBackgroundBodyDark << "}\r\n";
	*_textStream << ".body_light { font-family: monospace;  font-size: 14px; margin: 0px; background-color : " << colorBackgroundBodyLight << "}\r\n";
	*_textStream << ".date { color: #dadada; }\r\n";
	*_textStream << ".time { color: #107896; }\r\n";
	*_textStream << ".info .message_dark{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_INFO] << "; }\r\n";
	*_textStream << ".info1 .message_dark{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_INFO1] << "; }\r\n";
	*_textStream << ".info2 .message_dark{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_INFO2] << "; }\r\n";
	*_textStream << ".info3 .message_dark{ color:" <<	htmlStatusColorsLight[ConsoleLog::S_LOG_INFO3] << "; }\r\n";
	*_textStream << ".info4 .message_dark{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_INFO4] << "; }\r\n";
	*_textStream << ".info5 .message_dark{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_INFO5] << "; }\r\n";
	*_textStream << ".info6 .message_dark{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_INFO6] << "; }\r\n";
	*_textStream << ".info7 .message_dark{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_INFO7] << "; }\r\n";
	*_textStream << ".info8 .message_dark{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_INFO8] << "; }\r\n";
	*_textStream << ".debug .message_dark{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_DEBUG] << "; }\r\n";
	*_textStream << ".warning .message_dark{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_WARNING] << "; background-color : " << szWarningColor << "; }\r\n";
	*_textStream << ".error .message_dark{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_ERROR] << "; background-color : " << szErrorColor << "; font-weight: bold; }\r\n";
	*_textStream << ".error .message_dark{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_ERROR_DETAILS] << "; background-color : " << szErrorColor << "; font-weight: bold; }\r\n";


	*_textStream << ".info .message_light{ color: " <<	htmlStatusColorsLight[ConsoleLog::S_LOG_INFO] << "; }\r\n";
	*_textStream << ".info1 .message_light{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_INFO1] << "; }\r\n";
	*_textStream << ".info2 .message_light{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_INFO2] << "; }\r\n";
	*_textStream << ".info3 .message_light{ color:" <<	htmlStatusColorsLight[ConsoleLog::S_LOG_INFO3] << "; }\r\n";
	*_textStream << ".info4 .message_light{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_INFO4] << "; }\r\n";
	*_textStream << ".info5 .message_light{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_INFO5] << "; }\r\n";
	*_textStream << ".info6 .message_light{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_INFO6] << "; }\r\n";
	*_textStream << ".info7 .message_light{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_INFO7] << "; }\r\n";
	*_textStream << ".info8 .message_light{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_INFO8] << "; }\r\n";
	*_textStream << ".debug .message_light{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_DEBUG] << "; }\r\n";
	*_textStream << ".warning .message_light{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_WARNING] << "; background-color : " << szWarningColor << "; }\r\n";
	*_textStream << ".error .message_light{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_ERROR] << "; background-color : " << szErrorColor << "; font-weight: bold; }\r\n";
	*_textStream << ".error .message_light{ color: " << htmlStatusColorsLight[ConsoleLog::S_LOG_ERROR_DETAILS] << "; background-color : " << szErrorColor << "; font-weight: bold; }\r\n";


	*_textStream << ".fixed{ position: fixed ;} \r\n.header{ top: 0; left: 0; right: 0; height: 30px; width: 200 px; background-color: #000; opacity: 0.5; }r\n";
	*_textStream << "</style>\r\n</head>\r\n<body class=\"body_light\">\r\n";
	*_textStream <<
		R"(<script>function setDark() { document.body.className = "body_dark"; }function setLight() {  document.body.className = "body_light"; }</script>)";
	*_textStream << "<div class = \"fixed header\"><button onclick=\"setDark()\">Dark</button><button onclick=\"setLight()\">Light</button></div>\r\n";
	_textStream->flush();
}



QString DumpWorker::addHTMLLine(const int dwStatus, QString strMsg) const
{
	QString ret;
	QTextStream oStream(&ret);
	if (strMsg == " ")
		oStream << "<br>";
	else
	{
		strMsg.replace("\t", "<span style=\"padding: 0 20px\">&nbsp;</span>");
		strMsg.replace("\r\n", "<br>");

		oStream << "<div class = \"" << classesnames[dwStatus] << "\">\r\n";
		oStream << "<span class = \"date\">";
		oStream << QDate::currentDate().toString("yy-MM-dd");
		oStream << "</span>\r\n";
		oStream << "<span class = \"time\">";
		oStream << QTime::currentTime().toString("hh:mm:ss:zzz");
		oStream << "</span>\r\n";
		oStream << "<span class = \"message_light\">";
		oStream << strMsg;
		oStream << "</span>\r\n</div>\r\n";
	}
	return *oStream.string();
}

void DumpWorker::onData(int iStatus, const QString& dataLine)
{
	_queue.append(logEntry(iStatus, dataLine));	
	if (_bPaused)
	{
		process();
	}
}


void DumpWorker::abort()
{
	*_textStream << "</body></html>";
	_textStream->flush();
	_textStream.reset();
	_logFile.reset();
	_bAborted = true;
	emit finished();
}

void DumpWorker::process()
{
	_bPaused = false;
	while (!_queue.empty())
	{
		const auto strData = _queue.dequeue();
        if ( !_parentLog->m_bFileOpen )
            return;

		auto outDebugStr = "[" + _parentLog->_strOutputDebugPreffix + "]*";
		outDebugStr += strData._strLine;
		outDebugStr += "\n";
#ifdef WIN32
		OutputDebugStringA(outDebugStr.toUtf8().data());
#endif
		const int iStartPos = _textStream->pos();
		*_textStream << addHTMLLine(strData._iStatus, strData._strLine);
		_textStream->flush();

		const int iEndPos = _textStream->pos();
		_logFileSize += (iEndPos - iStartPos);

		if (_logFileSize > MAX_LOG_FILE_SIZE)
		{
			_textStream->device()->close();

			_logFileSize = 0;
			_iPartsCount++;

			_strCommonPath = QString("%1_%2.html").arg(_strNameForParts).arg(_iPartsCount);

			_logFile.reset(new QFile(_strCommonPath));
			_textStream.reset(new QTextStream());

            _parentLog->m_bFileOpen = false;

            if (_logFile->open(QFile::ReadWrite | QFile::Text))
            {
                _textStream->setDevice(_logFile.data());
                _parentLog->m_bFileOpen = true;
                initFileHeaders();
            }
		}
	}
	_bPaused = true;
}



ConsoleLog::ConsoleLog(QObject * parent) : QObject(parent)
{
	InitializeCriticalSection(&_consoleCs);
	_threadId = QThread::currentThreadId();
}

void ConsoleLog::Init() const
{
}

void ConsoleLog::InitGeneralLog(const QString& strFolder, const QString& strModule, const bool bLogToConsole, const bool bLogToFile, const LOG_LEVEL logLevel, const QString
                                & strPreffix)
{
	if (logLevel == LOG_LEVEL_NONE)
		return;

	if ( m_bInitialized )
		return;

	_strOutputDebugPreffix = strPreffix;
	_strFolder = strFolder;
	_strModuleName = strModule;
	_logLevel = logLevel;


    m_bShowTime = m_bShowThreadID = m_bShowFile = m_bShowLine = true;

	m_bFileOut = bLogToFile;
#ifdef WIN32
	if( bLogToConsole )
	{
		m_console = new WinLogConsole;
		bool bUseExisting = false;
		m_bIsConsole =  m_console->Create((QString("%1 debug Log").arg(QCoreApplication::applicationName())).toStdWString().c_str(), true, bUseExisting);
		if ( m_bIsConsole )
		{
			m_console->Color( FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE );
			m_addInfoColor = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
		}
	}
#else
    m_bIsConsole = bLogToConsole;
#endif

	if ( bLogToFile )
	{
		startDumpThread();
	}
    m_bInitialized = true;
}


void ConsoleLog::Cleanup()
{
	if (!m_bInitialized)
		return;

	if (m_bFileOut)
	{
		stopDumpThread();
	}

#ifdef WIN32
	if (m_bIsConsole)
	{
		Sleep(1000);
		m_console->Close();
		delete m_console;
	}
#endif
	m_bInitialized = false;
}


void ConsoleLog::startDumpThread()
{
	_pDumpThread.reset(new QThread());
	_pDumpWorker.reset(new DumpWorker(this));
	_pDumpWorker->moveToThread(_pDumpThread.get());

	// standard connections
	connect(_pDumpThread.get(), &QThread::started,		_pDumpWorker.get(),	&DumpWorker::startWorker);
	connect(_pDumpWorker.get(), &DumpWorker::finished,	_pDumpThread.get(),	&QThread::quit, Qt::DirectConnection);
	connect(_pDumpWorker.get(), &DumpWorker::finished,	_pDumpWorker.get(),	&DumpWorker::deleteLater);
	connect(_pDumpThread.get(), &QThread::finished,		_pDumpThread.get(),	&QThread::deleteLater);
	
	connect(this,		  &ConsoleLog::abortRequest,		_pDumpWorker.get(),	&DumpWorker::abort);
	connect(this,		  &ConsoleLog::data,				_pDumpWorker.get(),	&DumpWorker::onData);
	
	_pDumpThread->start();	
}

void ConsoleLog::stopDumpThread()
{
	emit abortRequest();
	_pDumpThread->wait();
}


void ConsoleLog::AddLogEntry(const QString& msg, const E_LOG_STATUS status, const QMessageLogContext& context)
{
	if (!m_bInitialized)
		return;

	_threadId == QThread::currentThreadId() ? AddLine(status, msg, copyableContext(context)) : QMetaObject::invokeMethod(this, "AddLine", Qt::QueuedConnection,
		Q_ARG(int, static_cast<int>(status)),
		Q_ARG(QString, msg),
		Q_ARG(copyableContext, copyableContext(context)));
}


void ConsoleLog::AddOther (const QtMsgType type, const copyableContext& context, const QString& msg)
{
    if (!m_bInitialized)
    {
        return;
    }

    auto dwStatus = S_LOG_DEBUG;

    if ( type == QtCriticalMsg || type == QtFatalMsg )
    {
        dwStatus = S_LOG_ERROR;
        QString strExt;
        strExt.reserve(msg.size() + 200);
        strExt = msg;
        strExt += "   [File : ";
        strExt += context._file;
        strExt += "] [Function : ";
        strExt += context._function;
        strExt += "] [Line : ";
        strExt += context._line;
        strExt += "]";

        if (m_bIsConsole)
        {
            AddConsoleLine(dwStatus, strExt);
        }
        emit data(dwStatus, strExt);
        return;
    }

    switch (type)
    {
        case QtInfoMsg : dwStatus = S_LOG_INFO; break;
        case QtDebugMsg : dwStatus = S_LOG_DEBUG; break;
        default:
            break;
    }

    if ( m_bIsConsole )
    {
        AddConsoleLine( dwStatus, msg);
    }
    emit data(dwStatus, msg);
}


void ConsoleLog::AddLine (const int dwStatus, const QString& msg, const copyableContext& context)
{
	if (_logLevel == LOG_LEVEL_NONE)
		return;

	if (_logLevel != LOG_LEVEL_ALL)
	{
		if (_logLevel == LOG_LEVEL_ERRORS_ONLY && dwStatus != S_LOG_ERROR)
		{
			return;
		}

		if (_logLevel == LOG_LEVEL_ERRORS_AND_WARNINGS && !(dwStatus == S_LOG_ERROR || dwStatus == S_LOG_ERROR_DETAILS || dwStatus == S_LOG_WARNING))
		{
			return;
		}
	}
	
	if ( dwStatus == S_LOG_ERROR_DETAILS)
	{
		QString strExt; 
		strExt.reserve(msg.size() + 200);
		strExt = msg;
		strExt += "\r\n\t[File     : ";
		strExt += context._file;
		strExt += "]\r\n\t[Function : ";
		strExt += context._function;
		strExt += "]\r\n\t[Line     : ";
		strExt += QString::number(context._line);
        strExt += "]\r\n";
		
		if (m_bIsConsole)
		{
			AddConsoleLine(dwStatus, strExt);
		}
		emit data(dwStatus, strExt);
		return;
	}

	if ( m_bIsConsole )
	{
		AddConsoleLine( dwStatus, msg);
	}
	emit data(dwStatus, msg);
}


void ConsoleLog::AddConsoleLine(const int dwStatus, const QString& msg)
{
#ifdef WIN32
	if ( !m_bInitialized || !m_bIsConsole || m_console == nullptr )
		return;

	const WORD statusColor = wStatusColors[dwStatus];

	EnterCriticalSection(&_consoleCs);
	m_console->Color( statusColor );
	m_console->Output( msg.toStdWString().c_str() );
	m_console->Output(nullptr);
	LeaveCriticalSection(&_consoleCs);
#else
   std::cout << wStatusColors[dwStatus] << qPrintable(msg) <<"\033[0m\n" ;
#endif
}

#ifdef WIN32
HANDLE ConsoleLog::getConsoleHandle() const
{
	if ( m_console )
	{
		return m_console->GetHandle();
	}
	return nullptr;
}
#endif

ConsoleLog::~ConsoleLog()
{
	_pDumpWorker.take();
	_pDumpThread.take();

	DeleteCriticalSection(&_consoleCs);
}

