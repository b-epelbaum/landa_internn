#include "DarkStyle.h"
#include "QSplashScreen.h"
#include "juneuiwnd.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication a(argc, argv);

	QFile lightStyle(QStringLiteral(":/JuneUIWnd/Resources/theme/qss/QApplication.css"));
	if (lightStyle.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    // set stylesheet
	  const QString qsStylesheet = QString::fromLatin1(lightStyle.readAll());
    a.setStyleSheet(qsStylesheet);
    lightStyle.close();
  }

	//a.setStyle(new DarkStyle);

	QPixmap pixmap(":/JuneUIWnd/Resources/landa_logo.png");
	QSplashScreen splash(pixmap);
	splash.show();
	a.processEvents();

	LandaJune::UI::JuneUIWnd * w = new LandaJune::UI::JuneUIWnd;
	splash.finish(w);
	w->show();
	return a.exec();
}
