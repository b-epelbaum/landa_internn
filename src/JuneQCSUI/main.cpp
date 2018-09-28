#include <QtWidgets/QApplication>
#include "apprunner.h"
#include "common/version.h"


int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QCoreApplication::setApplicationName("qcsagent");
    QCoreApplication::setApplicationVersion(CQT_APP_VERSION);

	QApplication a(argc, argv);

	appRunner runner(a ,argc, argv );
	return runner.runApp();
	
}
