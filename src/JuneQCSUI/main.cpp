#include "DarkStyle.h"
#include "framelesswindow.h"
#include "juneuiwnd.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication a(argc, argv);
	a.setStyle(new DarkStyle);

	QPixmap pixmap(":/JuneUIWnd/Resources/landa_logo");
	QSplashScreen splash(pixmap);
	splash.show();
	a.processEvents();

	//FramelessWindow framelessWindow;
	//framelessWindow.setWindowIcon(a.style()->standardIcon(QStyle::SP_DesktopIcon));
	LandaJune::UI::JuneUIWnd * w = new LandaJune::UI::JuneUIWnd;
	//framelessWindow.setContent(w);
	//framelessWindow.show();
	splash.finish(w);

	w->show();
	return a.exec();
}
