// Main_Alg.cpp : Defines the entry point for the console application.
//


#include "Main_Alg.h"

#include <fstream>
#include <iostream>
#include <opencv/cv.h> 
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp> 
#include <time.h>

using namespace std;
using namespace cv;


// functions declarations
float	Detect_Edge_X(const Mat& imImage, int iX1, int iX2, int iY) ;
float	Detect_Edge_Y(const Mat& imImage, int iY1, int iY2, int iX) ;
void	Find_Template_In_Image(const Mat& imImage, const Mat& imTemplate, int iTemplate_X, int iTemplate_Y, int iHalf_iSearch_Size, float& fDx, float& fDy, float& fCorr, int iMode) ;


// when given givel on edge (afEdges) - the linear line coefficients are found
// Use Ransac like method
void	Find_Line_Data(float* afEdges, int iEdges_Len, float& fA, float &fB)
{
	const int STEP = max(iEdges_Len / 50, 1);

	int	iCnt1, iCnt2, iCnt3;
	float fSlope, fY;

	int*	aiCount = new int[iEdges_Len * iEdges_Len];
	memset(aiCount, 0, iEdges_Len * iEdges_Len * sizeof(aiCount[0]));

	for (iCnt1 = 0; iCnt1 < iEdges_Len; iCnt1 += STEP)
		for (iCnt2 = iCnt1 + 10; iCnt2 < iEdges_Len; iCnt2 += STEP) {
			fSlope = (afEdges[iCnt2] - afEdges[iCnt1]) / (iCnt2 - iCnt1);
			fY = afEdges[iCnt1] - fSlope * iCnt1;

			for (iCnt3 = 0; iCnt3 < iEdges_Len; iCnt3 += STEP)
				if (fabs(fY + iCnt3 * fSlope - afEdges[iCnt3]) < 1)
					aiCount[iCnt1 + iCnt2 * iEdges_Len] ++;
		}

	int iMax_Count = 0;
	int iMax_Index1 = 0;
	int iMax_Index2 = 0;
	for (iCnt1 = 0; iCnt1 < iEdges_Len; iCnt1 += STEP)
		for (iCnt2 = iCnt1 + 10; iCnt2 < iEdges_Len; iCnt2 += STEP)
			if (aiCount[iCnt1 + iCnt2 * iEdges_Len] > iMax_Count) {
				iMax_Count = aiCount[iCnt1 + iCnt2 * iEdges_Len];
				iMax_Index1 = iCnt1;
				iMax_Index2 = iCnt2;
			}

	fSlope = (afEdges[iMax_Index2] - afEdges[iMax_Index1]) / (iMax_Index2 - iMax_Index1);
	fY = afEdges[iMax_Index1] - fSlope * iMax_Index1;

	int		iSamples = 0;
	float	fAverage_X = 0;
	float	fAverage_X2 = 0;
	float	fAverage_Y = 0;
	float	fAverage_XY = 0;
	for (iCnt3 = 0; iCnt3 < iEdges_Len; iCnt3++)
		if (fabs(fY + iCnt3 * fSlope - afEdges[iCnt3]) < 1) {
			fAverage_X += iCnt3;
			fAverage_X2 += iCnt3 * iCnt3;
			fAverage_Y += afEdges[iCnt3];
			fAverage_XY += iCnt3 * afEdges[iCnt3];
			iSamples++;
		}

	iSamples = __max(iSamples, 1);

	fAverage_X /= iSamples;
	fAverage_X2 /= iSamples;
	fAverage_Y /= iSamples;
	fAverage_XY /= iSamples;

	fA = (fAverage_XY - fAverage_X * fAverage_Y) / (fAverage_X2 - fAverage_X * fAverage_X);
	fB = fAverage_Y - fA * fAverage_X;

	delete[]aiCount;
}



// draw point on image - for overlay
void	Draw_Point(Mat& imDisp, float fX, float fY, ::byte ucR, ::byte ucG, ::byte ucB, float fFactor = 1)
{
#ifndef OVERLAY
	return;
#endif
	int	iPos_X = (int)round(fX * fFactor);
	int	iPos_Y = (int)round(fY * fFactor);

	imDisp.at<Vec3b>(iPos_Y, iPos_X)[0] = ucB;
	imDisp.at<Vec3b>(iPos_Y, iPos_X)[1] = ucG;
	imDisp.at<Vec3b>(iPos_Y, iPos_X)[2] = ucR;
}



//void	Run_Single_Image (const Mat& imLoad, const Mat& imTemplate, int iFirst_X, int iFirst_Y, int iDy, int iID, fstream& sImagePlacement_Data, const char* sMain_Path)
void	Run_Single_Image(const Mat& imLoad, const Mat& imTemplate, const ROI& atTriangle_ROI, const ROI* atROIs, int iID, const char* sCSV_Output, const char* sMain_CSV_Output, Output_Data& tOutput_Data)
{
	// pixel to micron ration
	// *** this should be passed to the API from the SW
	const float PIXEL_TO_MICRON_X = 0.08466683f;
	const float PIXEL_TO_MICRON_Y = 0.08660258f;

	int		iCnt, iImage;					// counters
	int		iX, iY;							// counters
	float	fCntX, fCntY;					// counters
	int		nLabels;						// number of labels in the image
	Mat		imTemplate_Smooth;				// template after smoothing
	float	afX[5][4];						// position of each circle - X
	float	afY[5][4];						// position of each circle - Y
	float	fTriangle_X, fTriangle_Y;		// triangle position
	float	fAx, fBx;						// Line equation (y=Ax+b) for triangle X edge
	float	fAy, fBy;						// Line equation (y=Ax+b) for triangle Y edge
	float	afEdges[1000];					// array for positions of edge
	int		iEdges_Len;						// length of edge array
	float	fPaper_X_Pos = 0;				// position of paper - X axis at Y position of triangle
	char	sOutput_Image_Name[1024];		// output image name - for overlay
	int		iFail_Paper = 0;				// fail flag - paper detection
	int		iFail_Tri = 0;					// fail flag - triangle detection
	int		iFail_Circles = 0;				// fail flag - circle detection
	Mat		imPart_Disp;					// on this image the overlay is written
	int		iStart_X, iStart_Y;			// start point of ROI
	int		iEnd_X, iEnd_Y;					// end point of ROI
	Mat		imLabels, imStat, imCentroids;	// data for labeling

	fstream sImagePlacement_Data(sMain_CSV_Output, ios::app);

	blur(imTemplate, imTemplate_Smooth, Size(1, 1));	// actually no blur

														// cpnvert to gray scale
	Mat	imInput_GL;
	cvtColor(imLoad, imInput_GL, CV_RGB2GRAY);
#ifdef OVERLAY
	Mat imLoad_Disp = imLoad.clone();
#else
	Mat imLoad_Disp (1, 1, CV_8U) ;
#endif

	// paper edge
	iEdges_Len = 0;
	for (iY = 100; iY < imInput_GL.rows - 100; iY += 50) {
		float fMiddle_Paper = Detect_Edge_X(imInput_GL, 60, 20, iY);
		afEdges[iEdges_Len++] = fMiddle_Paper;
	}
	Find_Line_Data(afEdges, iEdges_Len, fAx, fBx);
	for (iY = 100; iY < imInput_GL.rows - 100; iY++) {
		float fPos_X = fAx * float(iY - 100) / 50 + fBx;
		Draw_Point(imLoad_Disp, fPos_X, (float)iY, 0, 255, 0);
	}

	float fPaper_A = fAx / 50;
	float fPaper_B = fBx - 2;


	// detect triangle
	// ---------------
	fTriangle_X = fTriangle_Y = 0;

	// ROI of target
	iStart_X = atTriangle_ROI.iX;
	iEnd_X = iStart_X + atTriangle_ROI.iXsize;
	iStart_Y = atTriangle_ROI.iY;
	iEnd_Y = iStart_Y + atTriangle_ROI.iYsize;

	// Cut the image part
	Mat imPart_Triangle = imLoad(Rect(iStart_X, iStart_Y, iEnd_X - iStart_X + 1, iEnd_Y - iStart_Y + 1)).clone();

	// convert part to gray levels and HSV
	Mat	imPart_Triangle_GL, imPart_Triangle_T;
	cvtColor(imPart_Triangle, imPart_Triangle_GL, CV_RGB2GRAY);

	threshold(imPart_Triangle_GL, imPart_Triangle_T, 128, 255, THRESH_BINARY_INV);

	// opening
	erode(imPart_Triangle_T, imPart_Triangle_T, Mat::ones(9, 9, CV_8U));
	dilate(imPart_Triangle_T, imPart_Triangle_T, Mat::ones(9, 9, CV_8U));


	nLabels = cv::connectedComponentsWithStats(imPart_Triangle_T, imLabels, imStat, imCentroids, 8, CV_16U);

	for (int iLabel = 1; iLabel < nLabels; iLabel++) {

		int iXS = imStat.at<int>(iLabel, cv::CC_STAT_LEFT);
		int iWH = imStat.at<int>(iLabel, cv::CC_STAT_WIDTH);
		int iYS = imStat.at<int>(iLabel, cv::CC_STAT_TOP);
		int iHT = imStat.at<int>(iLabel, cv::CC_STAT_HEIGHT);
		int iSize = imStat.at<int>(iLabel, cv::CC_STAT_AREA);

		if (iSize < 700 || iSize > 1700)
			imPart_Triangle_T(Rect(iXS, iYS, iWH, iHT)) = 0;
	}

	nLabels = cv::connectedComponentsWithStats(imPart_Triangle_T, imLabels, imStat, imCentroids, 8, CV_16U);

	if (nLabels == 2) {
		int iXS = imStat.at<int>(1, cv::CC_STAT_LEFT);
		int iYS = imStat.at<int>(1, cv::CC_STAT_TOP);
		int iHT = imStat.at<int>(1, cv::CC_STAT_HEIGHT);
		int iWT = imStat.at<int>(1, cv::CC_STAT_WIDTH);

		// not work on narrow part of triangle
		iEdges_Len = 0;
		for (iY = iYS + 5; iY < iYS + iHT * 2 / 3; iY++) {
			float fMiddle_X = Detect_Edge_X(imPart_Triangle_GL, iXS - 10, iXS + 10, iY);
			afEdges[iEdges_Len++] = fMiddle_X;
		}

		Find_Line_Data(afEdges, iEdges_Len, fAx, fBx);
		for (fCntY = 0; fCntY < iEdges_Len; fCntY += 0.25) {
			float fPos_X = fAx * fCntY + fBx;
			Draw_Point(imPart_Disp, fPos_X, fCntY + iYS + 5, 0, 255, 0, 4);
		}
		float fX0 = fBx;
		float fY0 = iYS + 5.f;
		float fV0x = fAx;
		float fV0y = 1;

		iEdges_Len = 0;
		for (iX = iXS + 5; iX < iXS + iWT * 2 / 3; iX++) {
			float fMiddle_Y = Detect_Edge_Y(imPart_Triangle_GL, iYS - 10, iYS + 10, iX);
			afEdges[iEdges_Len++] = fMiddle_Y;
		}

		fAy, fBy;
		Find_Line_Data(afEdges, iEdges_Len, fAy, fBy);
		for (fCntX = 0; fCntX < iEdges_Len; fCntX += 0.25) {
			float fPos_Y = fAy * fCntX + fBy;
			Draw_Point(imPart_Disp, fCntX + iXS + 5, fPos_Y, 0, 255, 0, 4);
		}
		float fX1 = iXS + 5.f;
		float fY1 = fBy;
		float fV1x = 1;
		float fV1y = fAy;


		float fDet = -fV0x * fV1y + fV0y * fV1x;
		float fT1 = (fY1 - fY0) * fV0x - (fX1 - fX0) * fV0y;
		float fT0 = (fY1 - fY0) * fV1x - (fX1 - fX0) * fV1y;

		fT0 = fT0 / fDet;
		fT1 = fT1 / fDet;

		float fFinal_X = fV0x * fT0 + fX0;
		float fFinal_Y = fV0y * fT0 + fY0;
		Draw_Point(imPart_Disp, fFinal_X, fFinal_Y, 255, 0, 0, 4.f);
		Draw_Point(imPart_Disp, fFinal_X - 0.25f, fFinal_Y, 255, 0, 0, 4.f);
		Draw_Point(imPart_Disp, fFinal_X + 0.25f, fFinal_Y, 255, 0, 0, 4.f);
		Draw_Point(imPart_Disp, fFinal_X, fFinal_Y - 0.25f, 255, 0, 0, 4.f);
		Draw_Point(imPart_Disp, fFinal_X, fFinal_Y + 0.25f, 255, 0, 0, 4.f);

		fTriangle_X = fFinal_X + iStart_X;
		fTriangle_Y = fFinal_Y + iStart_Y;

		tOutput_Data.tTriangle_Edge_Pos.fX = round((fTriangle_X - fPaper_X_Pos) * PIXEL_TO_MICRON_X);
		tOutput_Data.tTriangle_Edge_Pos.fY = round(fTriangle_Y * PIXEL_TO_MICRON_Y);
		tOutput_Data.tTriangle_Edge_Pos.eStat = ALG_STATUS_SUCCESS;
	}
	else {
		tOutput_Data.tTriangle_Edge_Pos.fX = 0;
		tOutput_Data.tTriangle_Edge_Pos.fY = 0;
		tOutput_Data.tTriangle_Edge_Pos.eStat = ALG_STATUS_FAILED;

		iFail_Tri = 1;
	}

	fPaper_X_Pos = fPaper_A * fTriangle_Y + fPaper_B;

	// put paper data in output struct
	tOutput_Data.tPaper_Edge_Pos.fX = round(fPaper_X_Pos * PIXEL_TO_MICRON_X);
	tOutput_Data.tPaper_Edge_Pos.fY = 0;
	tOutput_Data.tPaper_Edge_Pos.eStat = ALG_STATUS_SUCCESS;


//	tAll_Data << fPaper_X_Pos << '\t' << fTriangle_X << '\t' << fTriangle_Y << '\t';



	// looo on targets
	for (iImage = 0; iImage < 5; iImage++) {

		// ROI of target
		iStart_X = atROIs[iImage].iX;
		iEnd_X = iStart_X + atROIs[iImage].iXsize;
		iStart_Y = atROIs[iImage].iY;
		iEnd_Y = iStart_Y + atROIs[iImage].iYsize;

		// Cut the image part
		Mat imPart = imLoad(Rect(iStart_X, iStart_Y, iEnd_X - iStart_X + 1, iEnd_Y - iStart_Y + 1)).clone();

		char sPart[255];
		sprintf_s(sPart, "d:\\temp\\d%d.tif", iImage);
		//imwrite (sPart, imPart) ;

#ifdef OVERLAY
		resize(imPart, imPart_Disp, Size(0, 0), 4, 4);
#endif

		// convert part to gray levels and HSV
		Mat	imPart_GL, imPart_HSV, imPart_T;
		cvtColor(imPart, imPart_GL, CV_RGB2GRAY);
		cvtColor(imPart, imPart_HSV, CV_RGB2HSV);

		Mat imPart_GL_Smooth;
		blur(imPart_GL, imPart_GL_Smooth, Size(1, 1));

		Mat	aimBGR[3];
		Mat	aimHSV[3];
		split(imPart, aimBGR);
		split(imPart_HSV, aimHSV);

		Mat imMin_Channel = min(min(aimBGR[0], aimBGR[1]), aimBGR[2]);

		threshold(imMin_Channel, imPart_T, 128, 255, THRESH_BINARY_INV);

		erode(imPart_T, imPart_T, Mat::ones(9, 9, CV_8U));
		dilate(imPart_T, imPart_T, Mat::ones(9, 9, CV_8U));

		nLabels = cv::connectedComponentsWithStats(imPart_T, imLabels, imStat, imCentroids, 8, CV_16U);

		for (int iLabel = 1; iLabel < nLabels; iLabel++) {

			int iXS = imStat.at<int>(iLabel, cv::CC_STAT_LEFT);
			int iWH = imStat.at<int>(iLabel, cv::CC_STAT_WIDTH);
			int iYS = imStat.at<int>(iLabel, cv::CC_STAT_TOP);
			int iHT = imStat.at<int>(iLabel, cv::CC_STAT_HEIGHT);
			int iSize = imStat.at<int>(iLabel, cv::CC_STAT_AREA);

			if (iSize < 100 || iSize > 600 || abs(iWH - iHT) > 10)
				imPart_T(Rect(iXS, iYS, iWH, iHT)) = 0;
		}


		// if (iImage > 0)
		//	tAll_Data << "0\t0\t";

		// binary image of each circle
		Mat imCyan = imPart_T & (aimHSV[0] > 0) & (aimHSV[0] < 30) & aimHSV[1] > 100;
		Mat imYellow = imPart_T & (aimHSV[0] > 80) & (aimHSV[0] < 110) & aimHSV[1] > 100;
		Mat imMagenta = imPart_T & (aimHSV[0] > 110) & (aimHSV[0] < 140) & aimHSV[1] > 100;
		Mat imBlack = imPart_T & (aimHSV[2] < 128) & (aimHSV[1] < 100);

		//imwrite("d:\\temp\\imCyan.tif", imCyan);
		//imwrite("d:\\temp\\imYellow.tif", imYellow);
		//imwrite("d:\\temp\\imMagenta.tif", imMagenta);
		//imwrite("d:\\temp\\imBlack.tif", imBlack);

		int iFail = 0;

		for (iCnt = 0; iCnt < 4; iCnt++) {
			afX[iImage][iCnt] = 0;
			afY[iImage][iCnt] = 0;
		}

		// color for overlay
		Scalar aiColors[] = { Scalar(255, 128, 0), Scalar(0, 0, 0), Scalar(0, 255, 255), Scalar(255, 0, 255) };

		tOutput_Data.ptTarget_Res[iImage].eMain_Stat = ALG_STATUS_SUCCESS;

		// loop on circles
		for (iCnt = 0; iCnt < 4; iCnt++) {
			Mat imUsed, imUsed_Proc;
			switch (iCnt) {
			case 0:	imUsed = imCyan.clone();	break;
			case 1:	imUsed = imBlack.clone();	break;
			case 2:	imUsed = imYellow.clone();	break;
			case 3:	imUsed = imMagenta.clone();	break;
			}

			erode(imUsed, imUsed_Proc, Mat::ones(9, 9, CV_8U));
			dilate(imUsed_Proc, imUsed_Proc, Mat::ones(9, 9, CV_8U));

			nLabels = cv::connectedComponentsWithStats(imUsed_Proc, imLabels, imStat, imCentroids, 8, CV_16U);

			if (nLabels == 2) {
				afX[iImage][iCnt] = (float)imCentroids.at<double>(2) + (float)iStart_X;
				afY[iImage][iCnt] = (float)imCentroids.at<double>(3) + (float)iStart_Y;

				float afDx[5], afDy[5], afCorr[5];
				for (int iMode = 0; iMode < 1; iMode++)
					Find_Template_In_Image(imPart_GL_Smooth, imTemplate_Smooth, (int)imCentroids.at<double>(2), (int)imCentroids.at<double>(3), 5, afDx[iMode], afDy[iMode], afCorr[iMode], iMode);
				//Find_Template_In_Image(imPart_GL, imTemplate, (int)imCentroids.at<double>(2), (int)imCentroids.at<double>(3), 5, afDx[iMode], afDy[iMode], afCorr[iMode], iMode);

				//qsort (afDx, 5, sizeof (afDx[0]), Compare_Float) ;
				//qsort (afDy, 5, sizeof (afDy[0]), Compare_Float) ;
				afX[iImage][iCnt] = (int)imCentroids.at<double>(2) + afDx[0] + (float)iStart_X;
				afY[iImage][iCnt] = (int)imCentroids.at<double>(3) + afDy[0] + (float)iStart_Y;

				tOutput_Data.ptTarget_Res[iImage].atResults[iCnt].fX = round((afX[iImage][iCnt] - fTriangle_X) * PIXEL_TO_MICRON_X * 1000);
				tOutput_Data.ptTarget_Res[iImage].atResults[iCnt].fY = round((afX[iImage][iCnt] - fTriangle_Y) * PIXEL_TO_MICRON_Y * 1000);
				tOutput_Data.ptTarget_Res[iImage].atResults[iCnt].eStat = ALG_STATUS_SUCCESS;
			}
			else {
				tOutput_Data.ptTarget_Res[iImage].atResults[iCnt].fX = 0;
				tOutput_Data.ptTarget_Res[iImage].atResults[iCnt].fY = 0;
				tOutput_Data.ptTarget_Res[iImage].atResults[iCnt].eStat = ALG_STATUS_FAILED;
				tOutput_Data.ptTarget_Res[iImage].eMain_Stat = ALG_STATUS_FAILED;

				iFail++;
				iFail_Circles = 1;
			}

			circle(imPart_Disp, Point((int)round((afX[iImage][iCnt] - iStart_X) * 4), (int)round((afY[iImage][iCnt] - iStart_Y) * 4)), 13 * 4, aiColors[iCnt], 2);
		}

//		if (!iFail)
//			for (iCnt = 0; iCnt < 4; iCnt++)
//				tAll_Data << afX[iImage][iCnt] << '\t' << afY[iImage][iCnt] << '\t';
//		else
//			for (iCnt = 0; iCnt < 4; iCnt++)
//				tAll_Data << 0 << '\t' << 0 << '\t';
//		tAll_Data << endl;

		if (iFail > 0)
			sprintf_s(sOutput_Image_Name, "E:\\Work\\Landa\\Database\\Results\\_Result_%d_%06d.jpg", iImage, iID);
		else
			if (iImage == 0)
				sprintf_s(sOutput_Image_Name, "E:\\Work\\Landa\\Database\\Results\\aResult_%d_%06d.jpg", iImage, iID);
			else
				sprintf_s(sOutput_Image_Name, "E:\\Work\\Landa\\Database\\Results\\Result_%d_%06d.jpg", iImage, iID);

#ifdef OVERLAY
		imwrite(sOutput_Image_Name, imPart_Disp);
#endif
	}

#ifdef OVERLAY
	sprintf_s(sOutput_Image_Name, "E:\\Work\\Landa\\Database\\Results\\bResult_%06d.jpg", iID);
	imwrite(sOutput_Image_Name, imLoad_Disp);
#endif


	// write CSV
	fstream tOutput_File(sCSV_Output, ios::out);
	tOutput_File << "Pattern Type :" << ',';
	tOutput_File << "Registration" << endl;
	tOutput_File << endl;
	tOutput_File << "Job Id :" << ',';
	tOutput_File << "121" << endl;
	tOutput_File << "Flat ID :" << ',';
	tOutput_File << iID + 1 << endl;
	tOutput_File << "ImageIndex ID :" << ',';
	tOutput_File << (iID % 11) + 1 << endl;
	tOutput_File << "Registration Side :" << ',';
	tOutput_File << "Left" << endl;
	tOutput_File << "Registration Overall Status :" << ',';
	tOutput_File << ((!iFail_Circles && !iFail_Tri) ? "Success" : "Failed") << endl;
	tOutput_File << "Ink\\Sets" << ',';
	tOutput_File << "Set #1 :" << ',';
	tOutput_File << "Success" << ',';
	tOutput_File << "Set #2 :" << ',';
	tOutput_File << "Success" << ',';
	tOutput_File << "Set #3 :" << ',';
	tOutput_File << "Success" << ',';
	tOutput_File << "Set #4 :" << ',';
	tOutput_File << "Success" << ',';
	tOutput_File << "Set #5 :" << ',';
	tOutput_File << "Success" << endl;


	for (iCnt = 0; iCnt < 4; iCnt++) {
		switch (iCnt) {
		case 0:	tOutput_File << "Cyan" << ',';		break;
		case 1:	tOutput_File << "Black" << ',';		break;
		case 2:	tOutput_File << "Yellow" << ',';	break;
		case 3:	tOutput_File << "Magenta" << ',';	break;
		}

		for (iImage = 0; iImage < 5; iImage++) {
			tOutput_File << round((afY[iImage][iCnt] - fTriangle_Y) * PIXEL_TO_MICRON_Y * 1000) << ',';
			tOutput_File << round((afX[iImage][iCnt] - fTriangle_X) * PIXEL_TO_MICRON_X * 1000);
			if (iImage < 4)
				tOutput_File << ',';
			else
				tOutput_File << endl;
		}
	}

	tOutput_File.close();

	sImagePlacement_Data << iID << ',' << (iID - 1) % 11 + 1 << ',';
	sImagePlacement_Data << ((iFail_Tri || iFail_Paper) ? "Failed" : "Success");
	sImagePlacement_Data << ",0,0,0,0,0,0,0,0,";
	sImagePlacement_Data << fTriangle_Y * PIXEL_TO_MICRON_Y * 1000 << ',';
	sImagePlacement_Data << (fTriangle_X - fPaper_X_Pos) * PIXEL_TO_MICRON_X * 1000 << ',';
	sImagePlacement_Data << "0,0,0" << endl;

	sImagePlacement_Data.close();
}




void	Main_Alg::Init (void)
{}

void	Main_Alg::ShutDown (void)
{}

void	Main_Alg::Detect (const Input_Data& tInput, Output_Data& tOutput, int iID, const char* sCSV_Output, const char* sMain_CSV_Output)
{
	int	iX, iY ;
	Mat	imImage ;
	Mat	imTemplate ;

	//fstream aa ("d:\\temp\\o.txt", ios::app);
	imImage.create (tInput.imImage.iYsize, tInput.imImage.iXsize, CV_8UC3) ;
	imTemplate.create (tInput.tTarget_Def.aimTemplates[0].iYsize, tInput.tTarget_Def.aimTemplates[0].iXsize, CV_8U);
	//aa<<clock()<<'\t';
	memcpy(imImage.data, tInput.imImage.aucData, tInput.imImage.iXsize * tInput.imImage.iYsize * 3);
	/*
	for (iY = 0 ; iY < tInput.imImage.iYsize ; iY ++)
		for (iX = 0; iX < tInput.imImage.iXsize; iX ++) {
//			imImage.at<::byte>(iY, iX) = tInput.imImage.aucData [(iX + iY *tInput.imImage.iXsize) * 4 + 0] ;
			imImage.at<::Vec3b>(iY, iX)[0] = tInput.imImage.aucData [(iX + iY *tInput.imImage.iXsize) * 3 + 0] ;
			imImage.at<::Vec3b>(iY, iX)[1] = tInput.imImage.aucData [(iX + iY *tInput.imImage.iXsize) * 3 + 1] ;
			imImage.at<::Vec3b>(iY, iX)[2] = tInput.imImage.aucData [(iX + iY *tInput.imImage.iXsize) * 3 + 2] ;
		}
	*/

	for (iY = 0; iY < tInput.tTarget_Def.aimTemplates[0].iYsize; iY++)
		for (iX = 0; iX < tInput.tTarget_Def.aimTemplates[0].iXsize; iX++)
			imTemplate.at<::byte>(iY, iX) = tInput.tTarget_Def.aimTemplates[0].aucData[iX + iY * tInput.tTarget_Def.aimTemplates[0].iXsize];

	//imwrite ("d:\\temp\\a1.bmp", imImage) ;
	//imwrite ("d:\\temp\\a2.bmp", imTemplate) ;
	//aa<<clock()<<'\t';
	Run_Single_Image(imImage, imTemplate, tInput.tTriangle_Pos, tInput.atTarget_Pos, iID, sCSV_Output, sMain_CSV_Output, tOutput) ;
	//aa<<clock()<<endl;
	//aa.close () ;

	
}



/*
int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}
*/

