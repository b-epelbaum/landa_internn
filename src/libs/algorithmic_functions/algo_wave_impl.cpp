#include "algo_wave_impl.h"

#include <fstream>
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/highgui/highgui.hpp> 

using namespace cv;
using namespace LandaJune::Algorithms;


#ifndef byte
typedef unsigned char byte;
#endif

// function declarations
void	Draw_Point(Mat& imDisp, float fX, float fY, byte ucR, byte ucG, byte ucB, float fFactor = 1);
void	Find_Template_In_Image(const Mat& imImage, const Mat& imTemplate, Mat& tCorr_Matrix, int iTemplate_X, int iTemplate_Y, int iHalf_iSearch_Size, float& fDx, float& fDy, float& fCorr, int iMode) ;
Mat		H_Diff(const Mat& imH1, int iH);


// global variables
thread_local	Mat		g_imWave_Part_GL, g_imWave_Part_HSV, g_imWave_Part_GL_Smooth;
thread_local	Mat		g_aimWave_BGR[3];
thread_local	Mat		g_aimWave_HSV[3];
thread_local	Mat		g_imWave_Color_Circle;
thread_local	Mat		g_imWave_Labels, g_imWave_Stat, g_imWave_Centroids;
thread_local	Mat		g_tWave_Corr_Matrix(50, 50, CV_32F);
Mat		g_imWave_Template_Smooth;				// template after smoothing
thread_local	Mat		g_imWave_Display;
thread_local	Mat		g_imWave_Color_Circle_Dil;

thread_local	float	g_afX[MAX_CIRCLES_IN_WAVE];		// position of each circle - X
thread_local	float	g_afY[MAX_CIRCLES_IN_WAVE];		// position of each circle - Y


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


void detect_wave(std::shared_ptr<LandaJune::Algorithms::PARAMS_WAVE_INPUT> input, std::shared_ptr<LandaJune::Algorithms::PARAMS_WAVE_OUTPUT> output)
{
	int		iLabel;
	int		iCircle;
	int		iLabels;
	float	fX, fY, fCorr;

	static int iSeq = 0;

	// define and clear overlay image
	if (input->_GenerateOverlay) {
		output->_colorOverlay->create(input->_waveImageSource->rows, input->_waveImageSource->cols, CV_8UC3);
		output->_colorOverlay->setTo(255);
//		output->overlay = input->_waveImageSource.clone();
	}

	cvtColor(*input->_waveImageSource, g_imWave_Part_GL, CV_RGB2GRAY);
	cvtColor(*input->_waveImageSource, g_imWave_Part_HSV, CV_BGR2HSV);

	blur(g_imWave_Part_GL, g_imWave_Part_GL_Smooth, Size(1, 1));
	split(*input->_waveImageSource, g_aimWave_BGR);
	split(g_imWave_Part_HSV, g_aimWave_HSV);


	// find center of H
	int iH_Center, iH_Range;
	if (input->_circleColor._min._iH < input->_circleColor._max._iH) {
		iH_Center = (input->_circleColor._min._iH + input->_circleColor._max._iH) / 2;
		iH_Range = input->_circleColor._max._iH - input->_circleColor._min._iH;
	}
	else {
		iH_Center = (input->_circleColor._min._iH + input->_circleColor._max._iH - 180) / 2;
		iH_Range = input->_circleColor._max._iH - input->_circleColor._min._iH + 180;
		if (iH_Center < 0)
			iH_Center += 180;
	}

	int iS_Center = (input->_circleColor._min._iS + 3 * input->_circleColor._max._iS) / 4;
	int iV_Center = input->_circleColor._max._iV / 2;

	g_imWave_Color_Circle = H_Diff(g_aimWave_HSV[0], iH_Center) <= iH_Range &
		g_aimWave_HSV[1] >= input->_circleColor._min._iS & g_aimWave_HSV[1] <= input->_circleColor._max._iS & g_aimWave_HSV[2] >= input->_circleColor._min._iV & g_aimWave_HSV[2] <= input->_circleColor._max._iV;

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
		if (((iSize < 40 || iSize > 200) || abs(iWH - iHT) > 7) && iSize < 10000)
			g_imWave_Color_Circle(Rect(iXS, iYS, iWH, iHT)) = 0;
	}
//	imwrite("e:\\temp\\Overlay_02.tif", g_imWave_Color_Circle);

	iLabels = cv::connectedComponentsWithStats(g_imWave_Color_Circle, g_imWave_Labels, g_imWave_Stat, g_imWave_Centroids, 8, CV_16U);

	for (iLabel = 1; iLabel < iLabels; iLabel++) {
		int iCircle = iLabel - 1;
		Find_Template_In_Image(g_imWave_Part_GL_Smooth, g_imWave_Template_Smooth, g_tWave_Corr_Matrix, (int)g_imWave_Centroids.at<double>(iLabel * 2), (int)g_imWave_Centroids.at<double>(iLabel * 2 + 1), 5, fX, fY, fCorr, 0);

		if (fCorr >= 0.5) {
			//		g_afX[iCircle] = g_imWave_Centroids.at<double>(iLabel * 2) ;
			//		g_afY[iCircle] = g_imWave_Centroids.at<double>(iLabel * 2 + 1) ;
			g_afX[iCircle] = (int)g_imWave_Centroids.at<double>(iLabel * 2) + fX;
			g_afY[iCircle] = (int)g_imWave_Centroids.at<double>(iLabel * 2 + 1) + fY;

			Point tCircle_Center = cv::Point((int)round(g_afX[iCircle]), round(g_afY[iCircle]));

			output->_colorCenters[iLabel-1]._x = (int)round((g_afX[iCircle] + input->_waveROI.left()) * input->Pixel2MM_X() * 1000);
			output->_colorCenters[iLabel-1]._y = (int)round((g_afY[iCircle] + input->_waveROI.left()) * input->Pixel2MM_X() * 1000);
			output->_colorDetectionResults[iLabel-1] = ALG_STATUS_SUCCESS ;
		}
		else {
			output->_colorCenters[iLabel - 1]._x = 0 ;
			output->_colorCenters[iLabel - 1]._y = 0 ;
			output->_colorDetectionResults[iLabel - 1] = ALG_STATUS_FAILED;
		}

		if (input->_GenerateOverlay) {
			int iX, iY;
			dilate(g_imWave_Color_Circle, g_imWave_Color_Circle_Dil, Mat::ones(3, 3, CV_8U));
			g_imWave_Color_Circle_Dil = g_imWave_Color_Circle_Dil - g_imWave_Color_Circle;

			// cout << iCircle << '\t' << g_afX[iCircle] << '\t' << g_afY[iCircle] << endl;

			int iStart_X = max((int)g_afX[iCircle] - 20, 0);
			int iEnd_X = min((int)g_afX[iCircle] + 20, g_imWave_Color_Circle_Dil.cols - 1);
			int iStart_Y = max((int)g_afY[iCircle] - 20, 0);
			int iEnd_Y = min((int)g_afY[iCircle] + 20, g_imWave_Color_Circle_Dil.rows - 1);

			// draw detection overlay - after threshold result
			for (iY = iStart_Y; iY < iEnd_Y; iY++)
				for (iX = iStart_X; iX < iEnd_X; iX++)
					if (g_imWave_Color_Circle_Dil.at<byte>(iY, iX))
						Draw_Point(*output->_colorOverlay, iX, iY, tRGB.at<Vec3b>(0)[0], tRGB.at<Vec3b>(0)[1], tRGB.at<Vec3b>(0)[2]);

			// draw cross around center
			for (iY = -5; iY < 5; iY++) {
				Draw_Point(*output->_colorOverlay, g_afX[iCircle], g_afY[iCircle] + iY, tRGB.at<Vec3b>(0)[0], tRGB.at<Vec3b>(0)[1], tRGB.at<Vec3b>(0)[2]);
				Draw_Point(*output->_colorOverlay, g_afX[iCircle] + iY, g_afY[iCircle], tRGB.at<Vec3b>(0)[0], tRGB.at<Vec3b>(0)[1], tRGB.at<Vec3b>(0)[2]);
			}
		}
	}

#if 0
// for checking accuracy
	int iCnt;
	Circle_Pos atPos[1000];
	for (iCnt = 0; iCnt < iLabels - 1; iCnt++) {
		atPos[iCnt].x = g_afX[iCnt];
		atPos[iCnt].y = g_afY[iCnt];
	}
	qsort(atPos, iLabels - 1, sizeof(atPos[0]), Compare_Circle_Pos);

	float fErr_X = 0;
	float fErr_Y = 0;
	for (iCnt = 1; iCnt < iLabels - 2; iCnt++) {
		float fCurrent_Err_X = atPos[iCnt].x - (atPos[iCnt - 1].x + atPos[iCnt + 1].x) / 2;
		float fCurrent_Err_Y = atPos[iCnt].y - (atPos[iCnt - 1].y + atPos[iCnt + 1].y) / 2;
		fErr_X += fabsf(fCurrent_Err_X);
		fErr_Y += fabsf(fCurrent_Err_Y);
	}

	std::fstream tRes ("e:\\temp\\Wave.txt", std::ios::app) ;
	tRes << "Number, Error X, Y: " << iLabels  << '\t' << fErr_X / iLabels << '\t' << fErr_Y / iLabels << std::endl;
	tRes.close () ;
#endif

//	char sOvl_Name[256];
//	sprintf_s(sOvl_Name, "e:\\temp\\wv_%03d.jpg", iSeq);
//	imwrite(sOvl_Name, output->_colorOverlay);
//	iSeq++;

}



void detect_wave_shutdown()
{
	
}
