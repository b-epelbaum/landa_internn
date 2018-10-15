#include "roitools.h"
#include "roiview.h"

roitools::roitools()
{
}

int roitools::showROITools(QWidget* parentWidget)
{
	auto roiToolsView = roiview(parentWidget);
	auto resVal = roiToolsView.exec();

	return resVal;
}
