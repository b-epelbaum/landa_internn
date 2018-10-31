#pragma once

#include "roitools_global.h"
#include "common/type_usings.h"
#include <QWidget>

class ROITOOLS_EXPORT roitools
{
public:
	roitools();

	int showROITools(LandaJune::BaseParametersPtr params, QWidget* parentWidget) const;

};
