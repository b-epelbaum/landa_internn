#include "algo_i2s_impl.h"
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/highgui/highgui.hpp> 

using namespace cv;
using namespace LandaJune;
using namespace Algorithms;

#ifndef byte
typedef unsigned char byte;
#endif

// global variables
float	*g_afTriangle_Edges[NUM_SIDES] = {NULL, NULL, NULL};	// array for positions of paper edge
thread_local Mat	g_imTriangle_Input_GL;
thread_local Mat	g_imTriangle_T;


// function declarations
float	Detect_Edge_X(const Mat& imImage, int iX1, int iX2, int iY);
float	Detect_Edge_Y(const Mat& imImage, int iX1, int iX2, int iY);
void	Find_Line_Data(float* afEdges, int iEdges_Len, float& fA, float &fB, float fThreshold);
void	Draw_Point(Mat& imDisp, float fX, float fY, byte ucR, byte ucG, byte ucB, float fFactor = 1);



void detect_i2s_init(const INIT_PARAMETER& initParam)
{
	detect_i2s_shutdown();

	g_afTriangle_Edges[LEFT] = new float[1000];
	g_afTriangle_Edges[RIGHT] = new float[1000];
	g_afTriangle_Edges[WAVE] = new float[1000];
}



void detect_i2s(PARAMS_I2S_INPUT_PTR input, PARAMS_I2S_OUTPUT_PTR output)
{
	int		iX, iY;							// counters
	float	fCntX, fCntY;					// counters
	float	fTriangle_X, fTriangle_Y;		// triangle position
	float	fAx, fBx;						// Line equation (y=Ax+b) for triangle X edge
	float	fAy, fBy;						// Line equation (y=Ax+b) for triangle Y edge
	int		iLabels;						// number of labels in the image
	Mat		imLabels, imStat, imCentroids;	// data for labeling
	int		iEdges_Len;

	// parameters of 3 triangle edges
	float afX[3];
	float afY[3];
	float afVx[3];
	float afVy[3];
	float aiValid[3];	// is linbe valid

	// triangle position
	fTriangle_X = fTriangle_Y = 0;

	// create and clear overlay
	if (input->GenerateOverlay()) {
		output->_triangleOverlay->create(input->_triangleImageSource->rows, input->_triangleImageSource->cols, CV_8UC3);
		output->_triangleOverlay->setTo(0);
	}

	// convert part to gray levels and HSV
	cvtColor(*input->_triangleImageSource, g_imTriangle_Input_GL, CV_RGB2GRAY);

	threshold(g_imTriangle_Input_GL, g_imTriangle_T, 128, 255, THRESH_BINARY_INV);

	// opening
	erode(g_imTriangle_T, g_imTriangle_T, Mat::ones(9, 9, CV_8U));
	dilate(g_imTriangle_T, g_imTriangle_T, Mat::ones(9, 9, CV_8U));

	// find connected componenets for the triangle detection - just to remove wrong size blobs
	iLabels = cv::connectedComponentsWithStats(g_imTriangle_T, imLabels, imStat, imCentroids, 8, CV_16U);

	// remove labels with wrong side
	for (int iLabel = 1; iLabel < iLabels; iLabel++) {

		int iXS = imStat.at<int>(iLabel, cv::CC_STAT_LEFT);
		int iWH = imStat.at<int>(iLabel, cv::CC_STAT_WIDTH);
		int iYS = imStat.at<int>(iLabel, cv::CC_STAT_TOP);
		int iHT = imStat.at<int>(iLabel, cv::CC_STAT_HEIGHT);
		int iSize = imStat.at<int>(iLabel, cv::CC_STAT_AREA);

		if (iSize < 500 || iSize > 1500)
			g_imTriangle_T(Rect(iXS, iYS, iWH, iHT)) = 0;

		// if too close to the edge
		if (iXS < 10 || iXS >= g_imTriangle_T.cols - 10 || iYS < 10 || iYS >= g_imTriangle_T.rows - 10)
			g_imTriangle_T(Rect(iXS, iYS, iWH, iHT)) = 0;
	}

	// find - again connected componenets for the triangle detection
	iLabels = cv::connectedComponentsWithStats(g_imTriangle_T, imLabels, imStat, imCentroids, 8, CV_16U);

	// 2 labels only - triangle and background 
	if (iLabels == 2) {
		int iXS = imStat.at<int>(1, cv::CC_STAT_LEFT);
		int iYS = imStat.at<int>(1, cv::CC_STAT_TOP);
		int iHT = imStat.at<int>(1, cv::CC_STAT_HEIGHT);
		int iWT = imStat.at<int>(1, cv::CC_STAT_WIDTH);

		// find vertical triangle line
		iEdges_Len = 0;
		for (iY = iYS + 3; iY < iYS + iHT * 2 / 3; iY++) {	// loop on part of the triangle side
			float fMiddle_X = Detect_Edge_X(g_imTriangle_Input_GL, iXS - 5, iXS + 5, iY);	// detect single pixel edge
			if (fMiddle_X >= 0)
				g_afTriangle_Edges[input->_side][iEdges_Len++] = fMiddle_X;
			if (input->GenerateOverlay())
				Draw_Point(*output->_triangleOverlay, fMiddle_X - 5, iY + 3, 0, 128, 0, 1);
		}

		// estimate line for edge-pixels
		Find_Line_Data(g_afTriangle_Edges[input->_side], iEdges_Len, fAx, fBx, 0.25f);
		if (input->GenerateOverlay())
			for (fCntY = 0; fCntY < iEdges_Len; fCntY += 1) {
				float fPos_X = fAx * fCntY + fBx;
				Draw_Point(*output->_triangleOverlay, fPos_X, fCntY + iYS + 3, 0, 255, 0, 1);
			}

		afX[0]	= fBx;
		afY[0]	= iYS + 3.f;
		afVx[0]	= fAx;
		afVy[0]	= 1;
		aiValid[0] = fabsf(fAx) < 0.05 ;


		// find horizontal triangle line
		iEdges_Len = 0;
		// for (iX = iXS + 5; iX < iXS + iWT * 2 / 3; iX++) {	// loop on part of the triangle side
		for (iX = iXS + 3; iX < iXS + iWT - 5; iX++) {	// loop on part of the triangle side
			float fMiddle_Y = Detect_Edge_Y(g_imTriangle_Input_GL, iYS - 5, iYS + 5, iX);	// detect single pixel edge
			if (fMiddle_Y >= 0)
				g_afTriangle_Edges[input->_side][iEdges_Len++] = fMiddle_Y;
			if (input->GenerateOverlay())
				Draw_Point(*output->_triangleOverlay, iX, fMiddle_Y - 5, 0, 128, 0, 1);
		}

		// estimate line for edge-pixels
		Find_Line_Data(g_afTriangle_Edges[input->_side], iEdges_Len, fAy, fBy, 0.25f);
		if (input->GenerateOverlay())
			for (fCntX = 0; fCntX < iEdges_Len; fCntX += 1) {
				float fPos_Y = fAy * fCntX + fBy;
				Draw_Point(*output->_triangleOverlay, fCntX + iXS + 3, fPos_Y, 0, 255, 0, 1);
			}
		afX[1] = iXS + 3.f;
		afY[1] = fBy;
		afVx[1] = 1;
		afVy[1] = fAy;
		aiValid[1] = fabsf(fAy) < 0.05;


		// find slanted triangle line
		iEdges_Len = 0;
		for (iY = iYS + 3; iY < iYS + iHT * 2 / 3; iY++) {	// loop on part of the triangle side
			float fMiddle_X = Detect_Edge_X(g_imTriangle_Input_GL, iXS + 45, iXS + 5, iY);	// detect single pixel edge
			if (fMiddle_X >= 0)
				g_afTriangle_Edges[input->_side][iEdges_Len++] = fMiddle_X;
			if (input->GenerateOverlay())
				Draw_Point(*output->_triangleOverlay, fMiddle_X + 5, iY + 3, 0, 128, 0, 1);
		}

		// estimate line for edge-pixels
		Find_Line_Data(g_afTriangle_Edges[input->_side], iEdges_Len, fAx, fBx, 0.25f);
		if (input->GenerateOverlay())
			for (fCntY = 0; fCntY < iEdges_Len; fCntY += 1) {
				float fPos_X = fAx * fCntY + fBx;
				Draw_Point(*output->_triangleOverlay, fPos_X, fCntY + iYS + 3, 0, 255, 0, 1);
			}
		afX[2] = fBx;
		afY[2] = iYS + 3.f;
		afVx[2] = fAx;
		afVy[2] = 1;
		aiValid[2] = fabsf(fAx+0.744) < 0.05;

		int iValid_Point_Found = false ;

		// loop on lines - the first valid 2 are used for the point
		for (int iPoint = 0 ; iPoint < 3 ; iPoint ++) {
			int iNext_Point = (iPoint + 1) % 3 ;

			if (aiValid [iPoint] && aiValid[iNext_Point]) {

				// meeting between lines
				float fDet = -afVx[iPoint] * afVy[iNext_Point] + afVy[iPoint] * afVx[iNext_Point];
				float fT1 = (afY[iNext_Point] - afY[iPoint]) * afVx[iPoint] - (afX[iNext_Point] - afX[iPoint]) * afVy[iPoint];
				float fT0 = (afY[iNext_Point] - afY[iPoint]) * afVx[iNext_Point] - (afX[iNext_Point] - afX[iPoint]) * afVy[iNext_Point];
	
				fT0 = fT0 / fDet;
				fT1 = fT1 / fDet;

				// triangle position
				fTriangle_X = afVx[iPoint] * fT0 + afX[iPoint];
				fTriangle_Y = afVy[iPoint] * fT0 + afY[iPoint];

				// overlay
				if (input->GenerateOverlay())
					Draw_Point(*output->_triangleOverlay, fTriangle_X, fTriangle_Y, 255, 0, 0);

				// get the corner position
				if (iPoint == 1)
					fTriangle_X -= 36.6 ;
				if (iPoint == 2)
					fTriangle_Y -= 49.7;

				if (iPoint > 0 && input->GenerateOverlay())
					Draw_Point(*output->_triangleOverlay, fTriangle_X, fTriangle_Y, 255, 0, 0);

				iValid_Point_Found = true ;
				break ;
			}
		}

		// one (or more) lines are too slanted
		if (iValid_Point_Found) {
			// set output data
			output->_triangeCorner._x = (int)round((fTriangle_X + input->_approxTriangeROI.left()) * input->Pixel2MM_X() * 1000);
			output->_triangeCorner._y = (int)round((fTriangle_Y + input->_approxTriangeROI.top()) * input->Pixel2MM_Y() * 1000);
			output->_result = ALG_STATUS_SUCCESS;
		}
		else {
			// set output data
			output->_result = ALG_STATUS_FAILED;
		}
	}
	else {
		// set output data for failure
		output->_triangeCorner._x = 0;
		output->_triangeCorner._y = 0;
		output->_result = ALG_STATUS_FAILED;
	}
}



void detect_i2s_shutdown()
{
	if (g_afTriangle_Edges[LEFT] != NULL)
		delete[]g_afTriangle_Edges[LEFT];
	if (g_afTriangle_Edges[RIGHT] != NULL)
		delete[]g_afTriangle_Edges[RIGHT];

	g_afTriangle_Edges[LEFT] = NULL;
	g_afTriangle_Edges[RIGHT] = NULL;

}
