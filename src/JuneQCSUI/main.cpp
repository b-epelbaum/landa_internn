#include "DarkStyle.h"
#include "juneuiwnd.h"
#include <QtWidgets/QApplication>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication a(argc, argv);
	a.setStyle(new DarkStyle);

	QPixmap pixmap(":/JuneUIWnd/Resources/landa_logo");
	QSplashScreen splash(pixmap);
	splash.setGeometry(200, 200, 800, 260);
	splash.show();
	a.processEvents();

	LandaJune::UI::JuneUIWnd * w = new LandaJune::UI::JuneUIWnd;
	splash.finish(w);

	w->show();
	return a.exec();
}
