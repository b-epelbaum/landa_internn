#pragma once

#include <QObject>

class appRunner : public QObject
{
	Q_OBJECT

public:
	appRunner(QApplication& app, int argc, char** argv);
	~appRunner();

	int runApp();

private:

	void parseArguments(const QApplication& app);
	QApplication& _app;

	QString _runMode;
	QString _logLevel;
    bool _saveToLogFile = false;
    QString _recipeFile;
	QString _logRootPath;
};
