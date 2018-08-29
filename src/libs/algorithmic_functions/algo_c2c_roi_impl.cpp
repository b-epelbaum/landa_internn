#include "algo_c2c_roi_impl.h"

#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/highgui/highgui.hpp> 

using namespace cv;
using namespace LandaJune::Algorithms;

//#include <fstream>
//std::fstream tData ("e:\\temp\\hsv.txt", std::ios::app) ;
//#define TEMPLATE_PATH "d:\\Template1.tif"

#ifndef byte
typedef unsigned char byte;
#endif

// function declarations
void	Draw_Point(Mat& imDisp, float fX, float fY, byte ucR, byte ucG, byte ucB, float fFactor = 1) ;

// global variables
thread_local	Mat		g_imPart_GL, g_imPart_HSV, g_imPart_GL_Smooth;
thread_local	Mat		g_aimBGR[3];
thread_local	Mat		g_aimHSV[3];
//thread_local	Mat		g_imMin_Channel;
//thread_local	Mat		g_imPart_T;
thread_local	Mat		g_imColor_Circle ;
//thread_local	Mat		g_imCyan, g_imYellow, g_imMagenta, g_imBlack;
thread_local	Mat		g_imLabels, g_imStat, g_imCentroids;
thread_local	Mat		g_imUsed_Proc;
thread_local	Mat		g_tCorr_Matrix(50, 50, CV_32F);
				Mat		g_imTemplate_Smooth;				// template after smoothing


Mat	H_Diff(const Mat& imH1, int iH)
{
	Mat	imDiff1 = abs(imH1 - iH);
	Mat	imDiff2 = abs(imH1 - iH + 255);
	Mat	imDiff3 = abs(imH1 - iH - 255);
	Mat imDiff = min(min(imDiff1, imDiff2), imDiff2);

	return imDiff;
}



// Correlate template in an image
// Pixels are not tested if has different Z (not used now)
// fCorrelation - the correlation
// iMatch - number of matched pixels
// iMode - what part to correlate (0 - all template) - currently it is the only used mode
void	Correlate_Templates(const Mat& imImage, const Mat& imTemplate, int iDx, int iDy, float& fCorrelation, int& iMatch, int iMode)
{
	int iX, iY;
	iMatch = 0;

	float fSum_1 = 0;
	float fSum_11 = 0;
	float fSum_2 = 0;
	float fSum_22 = 0;
	float fSum_12 = 0;

	for (iX = 0; iX < imTemplate.cols; iX++)
		for (iY = 0; iY < imTemplate.cols; iY++) {

			// out of image - correlatiion is minimum
			if (iX + iDx < 0 || iX + iDx >= imImage.cols - 1 || iY + iDy < 0 || iY + iDy >= imImage.rows - 1) {
				fCorrelation = -1;
				return;
			}

			float fVal1 = (float)imImage.at<byte>(iY + iDy, iX + iDx);
			float fVal2 = (float)imTemplate.at<byte>(iY, iX);
		
			if (iMode == 1 && iX < imTemplate.cols / 2)
				continue;
			if (iMode == 2 && iX > imTemplate.cols / 2)
				continue;
			if (iMode == 3 && iY < imTemplate.rows / 2)
				continue;
			if (iMode == 4 && iY > imTemplate.rows / 2)
				continue;

			if (fVal2 > 0) {
				fSum_1 += fVal1;
				fSum_11 += fVal1 * fVal1;
				fSum_2 += fVal2;
				fSum_22 += fVal2 * fVal2;
				fSum_12 += fVal1 * fVal2;
				iMatch++;
			}
		}

	if (iMatch > 0) {
		fSum_1 /= iMatch;
		fSum_11 /= iMatch;
		fSum_2 /= iMatch;
		fSum_22 /= iMatch;
		fSum_12 /= iMatch;
	}

	float fDenom = sqrtf((fSum_11 - fSum_1 * fSum_1) * (fSum_22 - fSum_2 * fSum_2));
	float fNomin = (fSum_12 - fSum_1 * fSum_2);

	fCorrelation = (fDenom > 0) ? fNomin / fDenom : 0;
}



// return the estimated maximum using parabolic fit, assumin fV2 is the maximum
float fParabolic_Estimation(float fV1, float fV2, float fV3)
{
	// parabolic coeficients Ax^2+Bx+C = 0
	float	fA = (fV1 + fV3) / 2 - fV2;
	float	fB = (fV3 - fV1) / 2;

	if (fA == 0)
		return 0;
	else
		return -fB / (2 * fA);
}



void Find_Template_In_Image(const Mat& imImage, const Mat& imTemplate, int iTemplate_X, int iTemplate_Y, int iHalf_iSearch_Size, float& fDx, float& fDy, float& fCorr, int iMode)
{
	const int iSearch_Size = iHalf_iSearch_Size * 2 + 1;

	//	Mat		imTemplate1 ;

		// correlation matrix
	g_tCorr_Matrix.setTo(-1);

	// match points should be above this value
//	int iMatch_Threshold = imTemplate1.rows * imTemplate1.cols / 4 ;

	for (int iY1 = -iHalf_iSearch_Size; iY1 <= iHalf_iSearch_Size; iY1++) {
		for (int iX1 = -iHalf_iSearch_Size; iX1 <= iHalf_iSearch_Size; iX1++) {
			float	fCorrelation;
			int		iMatch;

			Correlate_Templates(imImage, imTemplate, iX1 + iTemplate_X - imTemplate.cols / 2, iY1 + iTemplate_Y - imTemplate.rows / 2, fCorrelation, iMatch, iMode);

			//			if (iMatch > iMatch_Threshold)
			g_tCorr_Matrix.at<float>(iY1 + iHalf_iSearch_Size, iX1 + iHalf_iSearch_Size) = fCorrelation;
		}
	}

	Point tMax_Pos, tMin_Pos;
	double dMax_Val, dMin_Val;
	minMaxLoc(g_tCorr_Matrix, &dMin_Val, &dMax_Val, &tMin_Pos, &tMax_Pos);

	fCorr = (float)dMax_Val;

	// clear data of at edge of correlation array
	float	fSub_Pixel_X = 0;
	float	fSub_Pixel_Y = 0;

	// If edge of correlation matrix - failed
	if (tMax_Pos.x == 0 || tMax_Pos.x >= iSearch_Size - 1 || tMax_Pos.y == 0 || tMax_Pos.y >= iSearch_Size - 1)
		fCorr = 0;
	else {	// assign parabolic distance
		fSub_Pixel_X = fParabolic_Estimation(g_tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x - 1), g_tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x), g_tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x + 1));
		fSub_Pixel_Y = fParabolic_Estimation(g_tCorr_Matrix.at<float>(tMax_Pos.y - 1, tMax_Pos.x), g_tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x), g_tCorr_Matrix.at<float>(tMax_Pos.y + 1, tMax_Pos.x));
	}

	fDx = tMax_Pos.x - iHalf_iSearch_Size + fSub_Pixel_X;
	fDy = tMax_Pos.y - iHalf_iSearch_Size + fSub_Pixel_Y;
}



void detect_c2c_roi_init(const C2C_ROI_INIT_PARAMETER& initParam)
{
	detect_c2c_roi_shutdown();

	//Mat imTemplate = imread (TEMPLATE_PATH, CV_LOAD_IMAGE_GRAYSCALE) ;
	blur(initParam._templateImage, g_imTemplate_Smooth, Size(1, 1));	// actually no blur
}



void detect_c2c_roi(const PARAMS_C2C_ROI_INPUT& input, PARAMS_C2C_ROI_OUTPUT& output)
{
	int		iCnt;
	int		iLabels;
	float	afX[10];						// position of each circle - X
	float	afY[10];						// position of each circle - Y
	float	afDx[5], afDy[5], afCorr[5];

	static int iSeq = 0 ;

	output._result = ALG_STATUS_SUCCESS ;

	int iColor_Num = input._colors.size() ;		// number of clircles

	// define and clear overlay image
	if (input.GenerateOverlay()) {
		output._colorOverlay.create(input._ROIImageSource.rows, input._ROIImageSource.cols, CV_8UC3);
		output._colorOverlay.setTo(0);
	}

	// size of color centers
	// output._colorCenters.resize(iColor_Num);
	// output._colorStatuses.resize(iColor_Num);

	// convert part to gray levels and HSV
	cvtColor(input._ROIImageSource, g_imPart_GL, CV_RGB2GRAY);
	cvtColor(input._ROIImageSource, g_imPart_HSV, CV_RGB2HSV);

	blur(g_imPart_GL, g_imPart_GL_Smooth, Size(1, 1));

	split(input._ROIImageSource, g_aimBGR);

	split(g_imPart_HSV, g_aimHSV);

	// use to determine H, S and V of circels
	// somehow it is different than regulat RGB to HSV formula
	//mwrite("c:\\temp\\a1_h.tif", g_aimHSV[0]);
	//mwrite("c:\\temp\\a1_s.tif", g_aimHSV[1]);
	//mwrite("c:\\temp\\a1_v.tif", g_aimHSV[2]);


	int iFail = 0;
	int iFail_Circles = 0;


	// loop on circles
	for (iCnt = 0; iCnt < 4; iCnt++) {

		afX[iCnt] = 0;
		afY[iCnt] = 0;

		// find center of H
		int iH_Center, iH_Range ;
		if (input._colors[iCnt]._min._iH < input._colors[iCnt]._max._iH) {
			iH_Center = (input._colors[iCnt]._min._iH + input._colors[iCnt]._max._iH) / 2 ;
			iH_Range = input._colors[iCnt]._max._iH - input._colors[iCnt]._min._iH ;
		}
		else {
			iH_Center = (input._colors[iCnt]._min._iH + input._colors[iCnt]._max._iH - 256) / 2;
			iH_Range = input._colors[iCnt]._max._iH - input._colors[iCnt]._min._iH + 256 ;
			if (iH_Center < 0)
				iH_Center += 256 ;
		}

		int iS_Center = (input._colors[iCnt]._min._iS + input._colors[iCnt]._max._iS) / 2 ;
		int iV_Center = (input._colors[iCnt]._min._iV + input._colors[iCnt]._max._iV) / 2 ;

		g_imColor_Circle = H_Diff(g_aimHSV[0], iH_Center) <= iH_Range &
							g_aimHSV[1] >= input._colors[iCnt]._min._iS & g_aimHSV[1] <= input._colors[iCnt]._max._iS &
							g_aimHSV[2] >= input._colors[iCnt]._min._iV & g_aimHSV[2] <= input._colors[iCnt]._max._iV ;

		
		erode(g_imColor_Circle, g_imColor_Circle, Mat::ones(9, 9, CV_8U));
		dilate(g_imColor_Circle, g_imColor_Circle, Mat::ones(9, 9, CV_8U));

		// color for overlay
		Mat tHSV (1, 1, CV_8UC3) ;
		Mat tRGB(1, 1, CV_8UC3);
		tHSV.at<Vec3b>(0)[0] = iH_Center ;
		tHSV.at<Vec3b>(0)[1] = iS_Center;
		tHSV.at<Vec3b>(0)[2] = iV_Center;
		cvtColor(tHSV, tRGB, CV_HSV2RGB);

//		tData << "*" << std::endl ;
//		tData << "CSV:\t" << (int)tHSV.at<Vec3b>(0)[0] << '\t' << (int)tHSV.at<Vec3b>(0)[1] << '\t' << (int)tHSV.at<Vec3b>(0)[2] << std::endl ;
//		tData << "RGB:\t" << (int)tRGB.at<Vec3b>(0)[0] << '\t' << (int)tRGB.at<Vec3b>(0)[1] << '\t' << (int)tRGB.at<Vec3b>(0)[2] << std::endl;

		// find connected componenets for circle detection
		iLabels = cv::connectedComponentsWithStats(g_imColor_Circle, g_imLabels, g_imStat, g_imCentroids, 8, CV_16U);

		for (int iLabel = 1; iLabel < iLabels; iLabel++) {

			int iXS = g_imStat.at<int>(iLabel, cv::CC_STAT_LEFT);
			int iWH = g_imStat.at<int>(iLabel, cv::CC_STAT_WIDTH);
			int iYS = g_imStat.at<int>(iLabel, cv::CC_STAT_TOP);
			int iHT = g_imStat.at<int>(iLabel, cv::CC_STAT_HEIGHT);
			int iSize = g_imStat.at<int>(iLabel, cv::CC_STAT_AREA);

			// remove blbls - to small, too large, high aspect ratio
			if (iSize < 80 || iSize > 400 || abs(iWH - iHT) > 10)
				g_imColor_Circle(Rect(iXS, iYS, iWH, iHT)) = 0;
		}

		iLabels = cv::connectedComponentsWithStats(g_imColor_Circle, g_imLabels, g_imStat, g_imCentroids, 8, CV_16U);

		if (iLabels == 2) {
			afX[iCnt] = (float)g_imCentroids.at<double>(2); // + (float)iStart_X;
			afY[iCnt] = (float)g_imCentroids.at<double>(3); // + (float)iStart_Y;

			for (int iMode = 0; iMode < 1; iMode++)
				Find_Template_In_Image(g_imPart_GL_Smooth, g_imTemplate_Smooth, (int)g_imCentroids.at<double>(2), (int)g_imCentroids.at<double>(3), 5, afDx[iMode], afDy[iMode], afCorr[iMode], iMode);
			//Find_Template_In_Image(imPart_GL, imTemplate, (int)imCentroids.at<double>(2), (int)imCentroids.at<double>(3), 5, afDx[iMode], afDy[iMode], afCorr[iMode], iMode);

			//qsort (afDx, 5, sizeof (afDx[0]), Compare_Float) ;
			//qsort (afDy, 5, sizeof (afDy[0]), Compare_Float) ;
			afX[iCnt] = (int)g_imCentroids.at<double>(2) + afDx[0];
			afY[iCnt] = (int)g_imCentroids.at<double>(3) + afDy[0] ;

			output._colorCenters[iCnt]._x = (int)round(afX[iCnt] * input.Pixel2MM_X() * 1000);
			output._colorCenters[iCnt]._y = (int)round(afX[iCnt] * input.Pixel2MM_Y() * 1000);
			output._colorStatuses[iCnt] =  ALG_STATUS_SUCCESS ;
		}
		else {
			output._colorCenters[iCnt]._x = (int)round((afX[iCnt] + (float)input._ROI.left()) *	input.Pixel2MM_X() * 1000);
			output._colorCenters[iCnt]._y = (int)round((afY[iCnt] + (float)input._ROI.top()) *	input.Pixel2MM_Y() * 1000);
			output._colorStatuses[iCnt] = (iLabels > 2 ? ALG_STATUS_TOO_MANY_CIRCLES : ALG_STATUS_CIRCLE_NOT_FOUND);
			output._result = ALG_STATUS_FAILED ;
		}

		Point tCircle_Center = cv::Point((int)round(afX[iCnt]), (int)round(afY[iCnt])) ;

		if (input._GenerateOverlay) {
			for (float fAng = 0 ;fAng < 2 * 3.14159; fAng += 0.03) {
				float fPos_X = afX[iCnt] + 13 * cosf (fAng) ;
				float fPos_Y = afY[iCnt] + 13 * sinf (fAng);
				Draw_Point (output._colorOverlay, fPos_X, fPos_Y, tRGB.at<Vec3b>(0)[2], tRGB.at<Vec3b>(0)[1], tRGB.at<Vec3b>(0)[0]) ;
			}
		}

//		char sOvl_Name [256] ;
//		sprintf_s (sOvl_Name, "c:\\temp\\cc_%03d.jpg", iSeq) ;
//		imwrite (sOvl_Name, output._colorOverlay) ;
//		iSeq ++ ;
	}
}



void detect_c2c_roi_shutdown()
{
	
}
