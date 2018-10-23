#include "roiview.h"
#include <QFileDialog>


#define ROIVIEW_SCOPED_LOG PRINT_INFO6 << "[roiview] : "
#define ROIVIEW_SCOPED_WARNING PRINT_WARNING << "[roiview] : "
#define ROIVIEW_SCOPED_ERROR PRINT_ERROR << "[roiview] : "


roiview::roiview(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

roiview::~roiview()
{
}
