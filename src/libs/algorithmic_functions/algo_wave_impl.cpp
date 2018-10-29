#include "algo_wave_impl.h"

#include <fstream>
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/highgui/highgui.hpp> 

using namespace cv;
using namespace LandaJune;
using namespace Algorithms;


#ifndef byte
typedef unsigned char byte;
#endif

// function declarations
void	Draw_Point(Mat& imDisp, float fX, float fY, byte ucR, byte ucG, byte ucB, float fFactor = 1);
void	Find_Template_In_Image(const Mat& imImage, const Mat& imTemplate, Mat& tCorr_Matrix, int iTemplate_X, int iTemplate_Y, int iHalf_iSearch_Size, float& fDx, float& fDy, float& fCorr, int iMode) ;
Mat		H_Diff(const Mat& imH1, int iH);
int		Compare_Float(const void* arg1, const void* arg2) ;


// global variables
thread_local	Mat		g_imWave_Part_GL, g_imWave_Part_HSV, g_imWave_Part_GL_Smooth;
thread_local	Mat		g_aimWave_BGR[3];
thread_local	Mat		g_aimWave_HSV[3];
thread_local	Mat		g_imWave_Color_Circle;
thread_local	Mat		g_imWave_Labels, g_imWave_Stat, g_imWave_Centroids;
thread_local	Mat		g_tWave_Corr_Matrix(50, 50, CV_32F);
thread_local	Mat		g_imWave_Display;
thread_local	Mat		g_imWave_Color_Circle_Dil;

thread_local	float	g_afX[MAX_CIRCLES_IN_WAVE];		// position of each circle - X
thread_local	float	g_afY[MAX_CIRCLES_IN_WAVE];		// position of each circle - Y
thread_local	LandaJune::Algorithms::APOINT	g_atCenters_Micron[MAX_CIRCLES_IN_WAVE];

Mat		g_imWave_Template_Smooth;				// template after smoothing


void detect_wave_init(const WAVE_INIT_PARAMETER& initParam)
{
	detect_wave_shutdown();
	blur(initParam._templateImage, g_imWave_Template_Smooth, Size(1, 1));	// actually no blur
}


// float comparison function for qsort
int Compare_Circle_Pos(const void* arg1, const void* arg2)
{
	if (((Circle_Pos*)arg1)->x < ((Circle_Pos*)arg2)->x)
		return -1;
	else if (((Circle_Pos*)arg1)->x > ((Circle_Pos*)arg2)->x)
		return 1;
	else
		return 0;
}



// float comparison function for qsort
int Compare_Circle_Pos_Int(const void* arg1, const void* arg2)
{
	if (((Circle_Pos_Int*)arg1)->x < ((Circle_Pos_Int*)arg2)->x)
		return -1;
	else if (((Circle_Pos_Int*)arg1)->x > ((Circle_Pos_Int*)arg2)->x)
		return 1;
	else
		return 0;
}

/*
// float comparison function for qsort
int Compare_Point(LandaJune::Algorithms::APOINT arg1, LandaJune::Algorithms::APOINT arg2)
{
	if (arg1._x < arg2._x)
		return -1;
	else if (arg1._x > arg2._x)
		return 1;
	else
		return 0;
}
*/


// float comparison function for qsort
int Compare_Point(const void* arg1, const void* arg2)
{
	if (((LandaJune::Algorithms::APOINT*)arg1)->_x < ((LandaJune::Algorithms::APOINT*)arg2)->_x)
		return -1;
	else if (((LandaJune::Algorithms::APOINT*)arg1)->_x > ((LandaJune::Algorithms::APOINT*)arg2)->_x)
		return 1;
	else
		return 0;
}



void detect_wave(PARAMS_WAVE_INPUT_PTR input, PARAMS_I2S_OUTPUT_PTR waveTriangleOut, PARAMS_WAVE_OUTPUT_PTR output)
{
	int		iCnt, iLabel;
	int		iLabels;
	float	fX, fY, fCorr;

	/////////////////////////////////////////////////////
	///////  TRIANGLE X, Y
	//if (waveTriangleOut->_result == ALG_STATUS_SUCCESS)
	//{
	//	auto X = waveTriangleOut->_triangeCorner._x;
	//	auto Y = waveTriangleOut->_triangeCorner._y;
	//}

	static int iSeq = 0; 

	// define and clear overlay image
	if (input->_GenerateOverlay) {
		output->_colorOverlay->create(input->_waveImageSource->rows, input->_waveImageSource->cols, CV_8UC3);
		output->_colorOverlay->setTo(0);
//		output->overlay = input->_waveImageSource.clone();
	}

	cvtColor(*input->_waveImageSource, g_imWave_Part_GL, CV_RGB2GRAY);
	cvtColor(*input->_waveImageSource, g_imWave_Part_HSV, CV_BGR2HSV);

	blur(g_imWave_Part_GL, g_imWave_Part_GL_Smooth, Size(1, 1));
	split(*input->_waveImageSource, g_aimWave_BGR);
	split(g_imWave_Part_HSV, g_aimWave_HSV);

	const auto& circleColor = input->_circleColor;
	// find center of H
	int iH_Center, iH_Range;
	if (circleColor._min._iH < circleColor._max._iH) {
		iH_Center = (circleColor._min._iH + circleColor._max._iH) / 2;
		iH_Range = (circleColor._max._iH - circleColor._min._iH) / 2 ;
	}
	else {
		iH_Center = (circleColor._min._iH + circleColor._max._iH - 180) / 2;
		iH_Range = (circleColor._max._iH - circleColor._min._iH + 180) / 2 ;
		if (iH_Center < 0)
			iH_Center += 180;
	}

	int iS_Center = (circleColor._min._iS + 3 * circleColor._max._iS) / 4;
	int iV_Center = circleColor._max._iV ;

	g_imWave_Color_Circle = H_Diff(g_aimWave_HSV[0], iH_Center) <= iH_Range &
		g_aimWave_HSV[1] >= circleColor._min._iS & g_aimWave_HSV[1] <= circleColor._max._iS & g_aimWave_HSV[2] >= circleColor._min._iV & g_aimWave_HSV[2] <= circleColor._max._iV;

//	imwrite("e:\\temp\\Overlay_00.tif", g_imWave_Color_Circle);
	dilate(g_imWave_Color_Circle, g_imWave_Color_Circle, Mat::ones(3, 3, CV_8U));
	erode(g_imWave_Color_Circle, g_imWave_Color_Circle, Mat::ones(5, 5, CV_8U));
	dilate(g_imWave_Color_Circle, g_imWave_Color_Circle, Mat::ones(3, 3, CV_8U));

//	imwrite("e:\\temp\\Overlay_01.tif", g_imWave_Color_Circle);

	// color for overlay
	Mat tHSV(1, 1, CV_8UC3);
	Mat tRGB(1, 1, CV_8UC3);
	tHSV.at<Vec3b>(0)[0] = iH_Center;
	tHSV.at<Vec3b>(0)[1] = iS_Center;
	tHSV.at<Vec3b>(0)[2] = iV_Center;
	cvtColor(tHSV, tRGB, CV_HSV2RGB);

	// find connected componenets for circle detection
	iLabels = cv::connectedComponentsWithStats(g_imWave_Color_Circle, g_imWave_Labels, g_imWave_Stat, g_imWave_Centroids, 8, CV_16U);

	for (iLabel = 0; iLabel < iLabels; iLabel++) {

		int iXS = g_imWave_Stat.at<int>(iLabel, cv::CC_STAT_LEFT);
		int iWH = g_imWave_Stat.at<int>(iLabel, cv::CC_STAT_WIDTH);
		int iYS = g_imWave_Stat.at<int>(iLabel, cv::CC_STAT_TOP);
		int iHT = g_imWave_Stat.at<int>(iLabel, cv::CC_STAT_HEIGHT);
		int iSize = g_imWave_Stat.at<int>(iLabel, cv::CC_STAT_AREA);

		// rem ve blobls - to small, too large, high aspect ratio
		// iSize < 10000 for not removing the non-circles blob of the entire frame
		if (((iSize < 50 || iSize > 200) || abs(iWH - iHT) > 7) && iSize < 10000)
			g_imWave_Color_Circle(Rect(iXS, iYS, iWH, iHT)) = 0;
	}
//	imwrite("e:\\temp\\Overlay_02.tif", g_imWave_Color_Circle);

	iLabels = cv::connectedComponentsWithStats(g_imWave_Color_Circle, g_imWave_Labels, g_imWave_Stat, g_imWave_Centroids, 8, CV_16U);

	for (iLabel = 1; iLabel < iLabels; iLabel++) {
		int iCircle = iLabel - 1;
		Find_Template_In_Image(g_imWave_Part_GL_Smooth, g_imWave_Template_Smooth, g_tWave_Corr_Matrix, (int)g_imWave_Centroids.at<double>(iLabel * 2), (int)g_imWave_Centroids.at<double>(iLabel * 2 + 1), 5, fX, fY, fCorr, 0);

		if (iCircle >= MAX_CIRCLES_IN_WAVE)
			break ;

		if (fCorr >= 0.5) {
			//		g_afX[iCircle] = g_imWave_Centroids.at<double>(iLabel * 2) ;
			//		g_afY[iCircle] = g_imWave_Centroids.at<double>(iLabel * 2 + 1) ;
			g_afX[iCircle] = (int)g_imWave_Centroids.at<double>(iLabel * 2) + fX;
			g_afY[iCircle] = (int)g_imWave_Centroids.at<double>(iLabel * 2 + 1) + fY;

			cv::Point tCircle_Center = cv::Point((int)round(g_afX[iCircle]), round(g_afY[iCircle]));

			g_atCenters_Micron[iCircle]._x = (int)round((g_afX[iCircle] + input->_waveROI.left()) * input->Pixel2MM_X() * 1000) ;
			g_atCenters_Micron[iCircle]._y = (int)round((g_afY[iCircle] + input->_waveROI.top()) * input->Pixel2MM_Y() * 1000) ;

//			tPoint._x = (int)round((g_afX[iCircle] + input->_waveROI.left()) * input->Pixel2MM_X() * 1000);
//			tPoint._y = (int)round((g_afY[iCircle] + input->_waveROI.top()) * input->Pixel2MM_Y() * 1000);
//			output->_colorCenters.push_back (tPoint) ;
//			output->_colorDetectionResults.push_back(ALG_STATUS_SUCCESS);

			if (input->_GenerateOverlay) {
				// draw cross around center
				for (int iY = -5; iY < 5; iY++) {
					Draw_Point(*output->_colorOverlay, g_afX[iCircle], g_afY[iCircle] + iY, tRGB.at<Vec3b>(0)[0], tRGB.at<Vec3b>(0)[1], tRGB.at<Vec3b>(0)[2]);
					Draw_Point(*output->_colorOverlay, g_afX[iCircle] + iY, g_afY[iCircle], tRGB.at<Vec3b>(0)[0], tRGB.at<Vec3b>(0)[1], tRGB.at<Vec3b>(0)[2]);
				}
			}
		}
		else {
			LandaJune::Algorithms::APOINT tPoint;
			tPoint._x = 0 ;
			tPoint._y = 0 ;
			//output->_colorCenters.push_back(tPoint);
			//output->_colorDetectionResults.push_back(ALG_STATUS_FAILED);
		}
	}

	qsort (g_atCenters_Micron, iLabels - 1, sizeof (g_atCenters_Micron [0]), Compare_Point) ;

//	for (iLabel = 0 ; iLabel < iLabels - 1; iLabel++) {
//		output->_colorCenters.push_back(g_atCenters_Micron[iLabel]);
//		output->_colorDetectionResults.push_back(ALG_STATUS_SUCCESS);
//	}
//	std::fstream tRes("e:\\temp\\Wave.txt", std::ios::app);

	// find average distance in X between circles
	// initial average is found, and then only distances near this average are use to find more accurate average
	float fMean_Dist_X = 0 ;
	int iSamples = 0 ;
	for (iCnt = 1; iCnt < iLabels - 1; iCnt++) {
		float fDiff_X = g_atCenters_Micron[iCnt]._x - g_atCenters_Micron[iCnt - 1]._x ;

		if (g_atCenters_Micron[iCnt]._x > 0 && g_atCenters_Micron[iCnt-1]._x > 0) {
			fMean_Dist_X += fDiff_X ;
			iSamples ++ ;
		}
	}
	float fAverage_Circle_Dist = fMean_Dist_X /= max (iSamples, 1) ;

	fMean_Dist_X = 0;
	iSamples = 0;
	for (iCnt = 1; iCnt < iLabels - 1; iCnt++) {
		float fDiff_X = g_atCenters_Micron[iCnt]._x - g_atCenters_Micron[iCnt - 1]._x;

		if (g_atCenters_Micron[iCnt]._x > 0 && g_atCenters_Micron[iCnt - 1]._x > 0 && fabsf (fDiff_X - fAverage_Circle_Dist)  < 500) {
			fMean_Dist_X += fDiff_X;
			iSamples++;
		}
	}
	fAverage_Circle_Dist = fMean_Dist_X /= max(iSamples, 1);


	// assign closest circle to 'synthetic grid' - from the fAverage_Circle_Dist
	int iIndex_Center = (output->_colorCenters.size() - 1) / 2;
	for (iCnt = 0 ; iCnt < output->_colorCenters.size () ; iCnt ++) {
		int iEstimated_X = waveTriangleOut->_triangeCorner._x + (iCnt - iIndex_Center) * fAverage_Circle_Dist ; // 2713.6 ;

		int iMin_Dist = 500 ;
		int iMin_Dist_Index = -1 ;
		for (iLabel = 0; iLabel < iLabels - 1; iLabel++) {
			int iDist = abs (g_atCenters_Micron[iLabel]._x - iEstimated_X) ;
			if (iDist < iMin_Dist) {
				iMin_Dist = iDist ;
				iMin_Dist_Index = iLabel ;
			}
		}

		if (iMin_Dist_Index < 0) {
			output->_colorCenters [iCnt]._x = 0 ;
			output->_colorCenters[iCnt]._y = 0;
			output->_colorDetectionResults [iCnt] = ALG_STATUS_FAILED ;
		}
		else
		{
			output->_colorCenters[iCnt] = g_atCenters_Micron[iMin_Dist_Index] ;
			output->_colorDetectionResults[iCnt] = ALG_STATUS_SUCCESS;
		}
		//tRes << iCnt << '\t' << iMin_Dist << std::endl ;
	}
	//tRes.close () ;


	if (input->_GenerateOverlay) {
		int iX, iY;
		dilate(g_imWave_Color_Circle, g_imWave_Color_Circle_Dil, Mat::ones(3, 3, CV_8U));
		g_imWave_Color_Circle_Dil = g_imWave_Color_Circle_Dil - g_imWave_Color_Circle;

		int iStart_X = 0;
		int iStart_Y = 0;
		int iEnd_X = g_imWave_Color_Circle_Dil.cols ;
		int iEnd_Y = g_imWave_Color_Circle_Dil.rows ;

		// draw detection overlay - after threshold result
		for (iY = iStart_Y; iY < iEnd_Y; iY++)
			for (iX = iStart_X; iX < iEnd_X; iX++)
				if (g_imWave_Color_Circle_Dil.at<byte>(iY, iX)) {
					(*output->_colorOverlay).at<Vec3b>(iY, iX)[0] = tRGB.at<Vec3b>(0)[2] ;
					(*output->_colorOverlay).at<Vec3b>(iY, iX)[1] = tRGB.at<Vec3b>(0)[1] ;
					(*output->_colorOverlay).at<Vec3b>(iY, iX)[2] = tRGB.at<Vec3b>(0)[0] ;
				}
	}

	//std::sort(output->_colorCenters.begin(), output->_colorCenters.end(), Compare_Point);

#if 0
	char sOvl_Name[256];
	sprintf_s(sOvl_Name, "e:\\temp\\wv_%03d.jpg", iSeq);
	imwrite(sOvl_Name, *output->_colorOverlay);
	iSeq++;
#endif

// for checking accuracy
#if 0
	int iCnt;
	thread_local	Circle_Pos atPos[1000];
	thread_local	float	afError_X [1000] ;
	thread_local	float	afError_Y [1000] ;

	int iAcc_Samples = 0 ;
	for (iCnt = 0; iCnt < iLabels - 1; iCnt++)
		if (output->_colorDetectionResults[iCnt] == ALG_STATUS_SUCCESS) {
			atPos[iAcc_Samples].x = g_afX[iCnt];
			atPos[iAcc_Samples].y = g_afY[iCnt];
			iAcc_Samples ++ ;
		}
	qsort(atPos, iAcc_Samples, sizeof(atPos[0]), Compare_Circle_Pos);

	float fErr_X = 0;
	float fErr_Y = 0;
	for (iCnt = 1; iCnt < iAcc_Samples - 1; iCnt++) {
//		float fCurrent_Err_X = atPos[iCnt].x - (atPos[iCnt - 1].x + atPos[iCnt + 1].x + atPos[iCnt + 2].x + atPos[iCnt - 2].x) / 4 ;
//		float fCurrent_Err_Y = atPos[iCnt].y - (atPos[iCnt - 1].y + atPos[iCnt + 1].y + atPos[iCnt + 2].y + atPos[iCnt - 2].y) / 4;
		float fCurrent_Err_X = atPos[iCnt].x - (atPos[iCnt - 1].x + atPos[iCnt + 1].x) / 2;
		float fCurrent_Err_Y = atPos[iCnt].y - (atPos[iCnt - 1].y + atPos[iCnt + 1].y) / 2;
		fErr_X += fabsf(fCurrent_Err_X);
		fErr_Y += fabsf(fCurrent_Err_Y);

		afError_X [iCnt - 1] = fCurrent_Err_X ;
		afError_Y [iCnt - 1] = fCurrent_Err_Y ;

//		if (fabsf(fCurrent_Err_X) > 1 || fabsf(fCurrent_Err_Y) > 1)
//			fCurrent_Err_X = fCurrent_Err_X ;
	}

	std::fstream tRes("e:\\temp\\Wave.txt", std::ios::app);


	if (iAcc_Samples > 30) {
		for (iCnt = 0 ; iCnt < iAcc_Samples ; iCnt++) {
			// tRes << atPos[iCnt].x - atPos[iCnt - 1].x << '\t' << atPos[iCnt].y - atPos[iCnt - 1].y << '\t';
			tRes.precision(8);
			tRes << atPos[iCnt].x << '\t' << atPos[iCnt].y << '\t';
		}
		tRes << std::endl ;
/*
		qsort(afError_X, iAcc_Samples - 2, sizeof(afError_X[0]), Compare_Float);
		qsort(afError_Y, iAcc_Samples - 2, sizeof(afError_Y[0]), Compare_Float);

	//	tRes << "Number, Error X, Y: " << iLabels  << '\t' << iAcc_Samples << '\t' << fErr_X / (iAcc_Samples - 2) << '\t' << fErr_Y / (iAcc_Samples - 1 - 2) << std::endl;
		int i95 = floor ((iAcc_Samples - 2) * 0.975f + 0.5f) ;
		int i05 = floor ((iAcc_Samples - 2) * 0.025f + 0.5f) ;
		tRes << "Number, Error X, Y: " << iLabels << '\t' << iAcc_Samples << '\t' << afError_X [i95] - afError_X[i05] << '\t' << afError_Y[i95] - afError_Y[i05] << std::endl;
*/
	}

//	else
//		tRes << "Number, Error X, Y: " << iLabels << '\t' << iAcc_Samples << '\t' << -1 << '\t' << -1 << std::endl;
	tRes.close () ;
#endif
}



void detect_wave_shutdown()
{
	
}
