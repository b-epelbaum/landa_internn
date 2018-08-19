#include "algoTest.h"
#include "Main_Alg.h"
#include "frameRef.h"

#include <sstream>
#include <iomanip>
#include "jutils.h"

#ifdef _DEBUG
#pragma comment (lib, "opencv_world342d.lib")
#else
#pragma comment (lib, "opencv_world342.lib")
#endif


using namespace LandaJune::Algo;
using namespace LandaJune::Core;

static const int iFirst_X = 200 ; // 200 ; //90;
static const int iFirst_Y = 200 ; // 190 ; // 243;

static const int iFirstWidth = 120;
static const int iFirstHeight = 100;

static const int iDy = 1847;
static const int iDx = -5;

/*
ROI atROIs[5];
for (iCnt = 0; iCnt < 5; iCnt++) {
	atROIs[iCnt].iX = iFirst_X - 50;
	atROIs[iCnt].iY = iFirst_Y - 180 + iCnt * iDy;
	atROIs[iCnt].iXsize = 100;
	atROIs[iCnt].iYsize = 360;
}
*/


algoTest::algoTest(const IPInputParams& inParams, int iIndex, const QImage& targetImage)
{
	Main_Alg ma;
	ma.Init();

	const auto sourceImage = ImageParam{ targetImage.constBits(), targetImage.width(), targetImage.height() };
	auto templateImage = static_cast<Image>(inParams.templateImageParam());
	
	Input_Data inputData;

	inputData.imImage = static_cast<Image>(sourceImage);


	inputData.tTarget_Def.iCircles_Num = 1;
	inputData.tTarget_Def.aimTemplates = &templateImage;

	ROI atROIs[5];
	for (auto iCnt = 0; iCnt < 5; iCnt++) {
		atROIs[iCnt].iX = iFirst_X - 50 + iCnt * iDx;
		atROIs[iCnt].iY = iFirst_Y - 180 + iCnt * iDy;
		atROIs[iCnt].iXsize = 100;
		atROIs[iCnt].iYsize = 360;
	}

	inputData.tTriangle_Pos.iX = atROIs[0].iX;
	inputData.tTriangle_Pos.iY = atROIs[0].iY;
	inputData.tTriangle_Pos.iXsize = atROIs[0].iXsize;
	inputData.tTriangle_Pos.iYsize = atROIs[0].iYsize;


	std::ostringstream ss;
	ss << std::setw(6) << std::setfill('0') << iIndex;

	const auto& firstRegion = inParams.regions().at(0);
	auto csvPath = inParams.targetFolder().toStdString();
	csvPath.append("\\frame_")
		.append(ss.str())
		.append("_region_[")
		.append(std::to_string(firstRegion.left()))
		.append(", ")
		.append(std::to_string(firstRegion.top()))
		.append(", (")
		.append(std::to_string(firstRegion.width()))
		.append("x")
		.append(std::to_string(firstRegion.height()))
		.append(")].csv");

	auto csvOverallPath = inParams.targetFolder().toStdString();
	csvOverallPath.append("\\frames_final.csv");
		

	inputData.atTarget_Pos = atROIs;
	inputData.iTargets_Num = 5;
	inputData.iPaper_Edge_Pos = 20;

	Output_Data oData;
	oData.ptTarget_Res = new Target_Result[5];
	for (int iCnt = 0 ; iCnt < 5 ; iCnt ++)
		oData.ptTarget_Res[iCnt].Set (4) ;

	ma.Detect(inputData, oData, iIndex, csvPath.c_str(),  csvOverallPath.c_str());
	ma.ShutDown();
}
