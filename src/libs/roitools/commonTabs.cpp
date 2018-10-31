#include "commonTabs.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QSettings>

#include "applog.h"
#include "common/june_defs.h"

#define COMMTAB_SCOPED_LOG PRINT_INFO3 << "[roiTools] : "
#define COMMTAB_SCOPED_WARNING PRINT_WARNING << "[roiTools] : "
#define COMMTAB_SCOPED_ERROR PRINT_ERROR << "[roiTools] : "


commonTabs::commonTabs()
{
}


commonTabs::~commonTabs()
{
}

QString commonTabs::selectBitmapFile(QWidget * parentWidget, QString strPrompt, QString strLastFileKey)
{
	QSettings settings(REG_COMPANY_NAME, REG_ROOT_KEY);
	auto lastBitmapFolder = settings.value(strLastFileKey).toString();

	if ( lastBitmapFolder.isEmpty() )
	{
		lastBitmapFolder = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation)[0];
	}

	auto fileName = QFileDialog::getOpenFileName(parentWidget,
    strPrompt, lastBitmapFolder, QObject::tr("Bitmap files (*.BMP *bmp)"));

	if (fileName.isEmpty())
	{
		COMMTAB_SCOPED_LOG << "Loading registration image cancelled";
		return "";
	}

	COMMTAB_SCOPED_LOG << "Loading registration image file  : " << fileName;
	settings.setValue(strLastFileKey, QFileInfo(fileName).absoluteDir().absolutePath());

	return fileName;
}
