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
void	Find_Template_In_Image(const Mat& imImage, const Mat& imTemplate, Mat& tCorr_Matrix, int iTemplate_X, int iTemplate_Y, int iHalf_iSearch_Size, float& fDx, float& fDy, float& fCorr, int iMode);
Mat		H_Diff (const Mat& imH1, int iH) ;

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
thread_local	Mat		g_imDisplay;
thread_local	Mat		g_imColor_Circle_Dil;



void detect_c2c_roi_init(const C2C_ROI_INIT_PARAMETER& initParam)
{
	detect_c2c_roi_shutdown();

	//Mat imTemplate = imread (TEMPLATE_PATH, CV_LOAD_IMAGE_GRAYSCALE) ;
	blur(initParam._templateImage, g_imTemplate_Smooth, Size(1, 1));	// actually no blur
}



void detect_c2c_roi(std::shared_ptr<PARAMS_C2C_ROI_INPUT> input, std::shared_ptr<PARAMS_C2C_ROI_OUTPUT> output)
{
	int		iCircle;
	int		iLabels;
	float	afX[10];						// position of each circle - X
	float	afY[10];						// position of each circle - Y
	float	afDx[5], afDy[5], afCorr[5];

	static int iSeq = 0 ;

	output->_result = ALG_STATUS_SUCCESS ;

	int iColor_Num = input->_colors.size() ;		// number of clircles

	// define and clear overlay image
	if (input->GenerateOverlay()) 
	{
		output->_colorOverlay->create(input->_ROIImageSource->rows, input->_ROIImageSource->cols, CV_8UC3);
		output->_colorOverlay->setTo(0);
	}

	// size of color centers
	// output->_colorCenters.resize(iColor_Num);
	// output->_colorStatuses.resize(iColor_Num);

	// convert part to gray levels and HSV
	cvtColor(*input->_ROIImageSource, g_imPart_GL, CV_RGB2GRAY);
	cvtColor(*input->_ROIImageSource, g_imPart_HSV, CV_BGR2HSV);

	blur(g_imPart_GL, g_imPart_GL_Smooth, Size(1, 1));

	split(*input->_ROIImageSource, g_aimBGR);

	split(g_imPart_HSV, g_aimHSV);

	// use to determine H, S and V of circels
	// somehow it is different than regulat RGB to HSV formula
	//imwrite("c:\\temp\\a1_h.tif", g_aimHSV[0]);
	//imwrite("c:\\temp\\a1_s.tif", g_aimHSV[1]);
	//imwrite("c:\\temp\\a1_v.tif", g_aimHSV[2]);


	int iFail = 0;
	int iFail_Circles = 0;

	// loop on circles
	for (iCircle = 0; iCircle < input->_colors.size(); iCircle++) {

		afX[iCircle] = 0;
		afY[iCircle] = 0;

		// find center of H
		int iH_Center, iH_Range ;
		if (input->_colors[iCircle]._min._iH < input->_colors[iCircle]._max._iH) {
			iH_Center = (input->_colors[iCircle]._min._iH + input->_colors[iCircle]._max._iH) / 2 ;
			iH_Range = (input->_colors[iCircle]._max._iH - input->_colors[iCircle]._min._iH) / 2 ;
		}
		else {
			iH_Center = (input->_colors[iCircle]._min._iH + input->_colors[iCircle]._max._iH - 180) / 2;
			iH_Range = (input->_colors[iCircle]._max._iH - input->_colors[iCircle]._min._iH + 180) / 2;
			if (iH_Center < 0)
				iH_Center += 180 ;
		}

		// int iS_Center = (input->_colors[iCircle]._min._iS + input->_colors[iCircle]._max._iS) / 2 ;
		int iS_Center = (input->_colors[iCircle]._min._iS + 3 * input->_colors[iCircle]._max._iS) / 4 ;
		//		int iV_Center = (input->_colors[iCircle]._min._iV + input->_colors[iCircle]._max._iV) / 2 ;
		int iV_Center = input->_colors[iCircle]._max._iV ;

		g_imColor_Circle = H_Diff(g_aimHSV[0], iH_Center) <= iH_Range &
							g_aimHSV[1] >= input->_colors[iCircle]._min._iS & g_aimHSV[1] <= input->_colors[iCircle]._max._iS &
							g_aimHSV[2] >= input->_colors[iCircle]._min._iV & g_aimHSV[2] <= input->_colors[iCircle]._max._iV ;

		
		dilate(g_imColor_Circle, g_imColor_Circle, Mat::ones(5, 5, CV_8U));
		erode(g_imColor_Circle, g_imColor_Circle, Mat::ones(9, 9, CV_8U));
		dilate(g_imColor_Circle, g_imColor_Circle, Mat::ones(5, 5, CV_8U));

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
			if (iSize < 80 || iSize > 400 || abs(iWH - iHT) > 12)
				g_imColor_Circle(Rect(iXS, iYS, iWH, iHT)) = 0;
		}

		iLabels = cv::connectedComponentsWithStats(g_imColor_Circle, g_imLabels, g_imStat, g_imCentroids, 8, CV_16U);

		if (iLabels == 2) {
			afX[iCircle] = (float)g_imCentroids.at<double>(2); // + (float)iStart_X;
			afY[iCircle] = (float)g_imCentroids.at<double>(3); // + (float)iStart_Y;

			for (int iMode = 0; iMode < 1; iMode++)
				Find_Template_In_Image(g_imPart_GL_Smooth, g_imTemplate_Smooth, g_tCorr_Matrix, (int)g_imCentroids.at<double>(2), (int)g_imCentroids.at<double>(3), 5, afDx[iMode], afDy[iMode], afCorr[iMode], iMode);
			//Find_Template_In_Image(imPart_GL, imTemplate, (int)imCentroids.at<double>(2), (int)imCentroids.at<double>(3), 5, afDx[iMode], afDy[iMode], afCorr[iMode], iMode);

			//qsort (afDx, 5, sizeof (afDx[0]), Compare_Float) ;
			//qsort (afDy, 5, sizeof (afDy[0]), Compare_Float) ;
			afX[iCircle] = (int)g_imCentroids.at<double>(2) + afDx[0];
			afY[iCircle] = (int)g_imCentroids.at<double>(3) + afDy[0] ;

			output->_colorCenters[iCircle]._x = (int)round((afX[iCircle] + (float)input->_ROI.left()) *	input->Pixel2MM_X() * 1000);
			output->_colorCenters[iCircle]._y = (int)round((afY[iCircle] + (float)input->_ROI.top()) *	input->Pixel2MM_Y() * 1000);
			output->_colorStatuses[iCircle] =  ALG_STATUS_SUCCESS ;
		}
		else {
			output->_colorCenters[iCircle]._x = 0;
			output->_colorCenters[iCircle]._y = 0;
			output->_colorStatuses[iCircle] = (iLabels > 2 ? ALG_STATUS_TOO_MANY_CIRCLES : ALG_STATUS_CIRCLE_NOT_FOUND);
			output->_result = ALG_STATUS_FAILED ;
		}

		Point tCircle_Center = cv::Point((int)round(afX[iCircle]), (int)round(afY[iCircle])) ;

		if (input->GenerateOverlay()) 
		{
			int iX, iY ;
			dilate (g_imColor_Circle, g_imColor_Circle_Dil, Mat::ones(3, 3, CV_8U)) ;
			g_imColor_Circle_Dil = g_imColor_Circle_Dil - g_imColor_Circle ;

			// draw detection overlay - after threshold result
			for (iY = 0 ; iY < g_imColor_Circle_Dil.rows ; iY ++)
				for (iX = 0; iX < g_imColor_Circle_Dil.cols; iX++)
					if (g_imColor_Circle_Dil.at<byte>(iY, iX))
						Draw_Point(*output->_colorOverlay, iX, iY, tRGB.at<Vec3b>(0)[0], tRGB.at<Vec3b>(0)[1], tRGB.at<Vec3b>(0)[2]);

			// draw cross around center
			for (iY = -5 ; iY < 5; iY++) {
				Draw_Point(*output->_colorOverlay, afX[iCircle], afY[iCircle] + iY, tRGB.at<Vec3b>(0)[0], tRGB.at<Vec3b>(0)[1], tRGB.at<Vec3b>(0)[2]);
				Draw_Point(*output->_colorOverlay, afX[iCircle] + iY, afY[iCircle], tRGB.at<Vec3b>(0)[0], tRGB.at<Vec3b>(0)[1], tRGB.at<Vec3b>(0)[2]);
			}

//			for (float fAng = 0 ;fAng < 2 * 3.14159; fAng += 0.03) {
//				float fPos_X = afX[iCircle] + 13 * cosf (fAng) ;
//				float fPos_Y = afY[iCircle] + 13 * sinf (fAng);
//				Draw_Point (output._colorOverlay, fPos_X, fPos_Y, tRGB.at<Vec3b>(0)[2], tRGB.at<Vec3b>(0)[1], tRGB.at<Vec3b>(0)[0]) ;
//			}

		}
	}
//	char sOvl_Name[256];
//	sprintf_s(sOvl_Name, "c:\\temp\\cc_%03d.jpg", iSeq);
//	imwrite(sOvl_Name, output._colorOverlay);
//	iSeq++;
}



void detect_c2c_roi_shutdown()
{
	
}
