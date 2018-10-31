#include "roitools.h"
#include "roiToolManWnd.h"
#include "common/june_errors.h"

#include "ProcessParameters.h"
#include <QDialog>

using namespace LandaJune;
using namespace Parameters;

roitools::roitools()
= default;

int roitools::showROITools(LandaJune::BaseParametersPtr params, QWidget* parentWidget) const
{
	const auto processParams = std::dynamic_pointer_cast<ProcessParameters>(params);
	if (!processParams)
	{
		return -1;
	}

	auto roiToolsView = new roiToolMainWnd(processParams, parentWidget);
	roiToolsView->setAttribute(Qt::WA_DeleteOnClose);
	roiToolsView->setWindowModality(Qt::ApplicationModal);
	roiToolsView->show();
	return 0;
}
