#include "apprunner.h"
#include "juneuiwnd.h"

#include <QCommandLineParser>
#include <QSplashScreen>

appRunner::appRunner( QApplication& app, int argc, char** argv)
	: QObject(nullptr)
	, _app(app)
{
	parseArguments(_app);
}

appRunner::~appRunner()
{
}

void appRunner::parseArguments(const QApplication& app)
{
	QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();
	parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);


	QCommandLineOption saveLogToFile("logfile", QCoreApplication::translate("main", "Save log to file"));
    parser.addOption(saveLogToFile);
	
    // An option with a value

	///////////////////////////////////////////
	// -mode=ui|noui[console,headless]|batch

	QCommandLineOption modeOption(QStringList() << "mode",
            QCoreApplication::translate("main", "Application run mode"),
            QCoreApplication::translate("main", "mode"), "ui");
    parser.addOption(modeOption);

	///////////////////////////////////////////
	// -recipe=<path_to_file>

    QCommandLineOption configFileOption(QStringList() << "recipe",
            QCoreApplication::translate("main", "Load processing recipe file"),
            QCoreApplication::translate("main", "file"));
    parser.addOption(configFileOption);

	///////////////////////////////////////////
	// -logroot=<path_to_log_folder>

	QCommandLineOption logRootFolderOption(QStringList() << "logroot",
            QCoreApplication::translate("main", "Log files root folder"),
            QCoreApplication::translate("main", "logroot"));
    parser.addOption(logRootFolderOption);

	///////////////////////////////////////////
	// -loglevel=all|warning|errors

	QCommandLineOption logLevelOption(QStringList() << "loglevel",
            QCoreApplication::translate("main", "Set a log level"),
            QCoreApplication::translate("main", "level"));
    parser.addOption(logLevelOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    //.const QStringList args = parser.positionalArguments();
    // source is args.at(0), destination is args.at(1)

    _runMode = parser.value(modeOption);
    _saveToLogFile = parser.isSet(saveLogToFile);
    _recipeFile = parser.value(configFileOption);
	_logRootPath = parser.value(logRootFolderOption);
	_saveToLogFile = !_logRootPath.isEmpty();
	_logLevel = parser.value(logLevelOption);
}


int appRunner::runApp()
{
	bool bOk = false;
	const auto mode = LandaJune::UI::JuneUIWnd::stringToMode(_runMode, &bOk);

	if ( mode == LandaJune::UI::RUN_UI)
	{
		QFile lightStyle(QStringLiteral(":/JuneUIWnd/Resources/theme/qss/QApplication.css"));
		if (lightStyle.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			// set stylesheet
			const QString qsStylesheet = QString::fromLatin1(lightStyle.readAll());
			_app.setStyleSheet(qsStylesheet);
			lightStyle.close();
		}

		//a.setStyle(new DarkStyle);

		const QPixmap pixmap(":/JuneUIWnd/Resources/landa_logo.png");
		QSplashScreen splash(pixmap);
		splash.show();
		_app.processEvents();
		LandaJune::UI::JuneUIWnd * w = new LandaJune::UI::JuneUIWnd(_runMode, _logLevel, _saveToLogFile, _logRootPath, _recipeFile);
		splash.finish(w);
		w->show();
		return _app.exec();
	}

	LandaJune::UI::JuneUIWnd * w = new LandaJune::UI::JuneUIWnd(_runMode, _logLevel, _saveToLogFile, _logRootPath, _recipeFile);
	return _app.exec();
}
