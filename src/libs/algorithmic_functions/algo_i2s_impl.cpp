#include "algo_i2s_impl.h"
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/highgui/highgui.hpp> 

using namespace LandaJune::Algorithms;
using namespace cv;

#ifndef byte
typedef unsigned char byte;
#endif

// global variables
float	*g_afTriangle_Edges[NUM_SIDES] = {NULL, NULL};	// array for positions of paper edge
thread_local Mat	g_imTriangle_Input_GL;
thread_local Mat	g_imTriangle_T;


// function declarations
float	Detect_Edge_X(const Mat& imImage, int iX1, int iX2, int iY);
float	Detect_Edge_Y(const Mat& imImage, int iX1, int iX2, int iY);
void	Find_Line_Data(float* afEdges, int iEdges_Len, float& fA, float &fB);
void	Draw_Point(Mat& imDisp, float fX, float fY, byte ucR, byte ucG, byte ucB, float fFactor = 1);



void detect_i2s_init(const INIT_PARAMETER& initParam)
{
	detect_i2s_shutdown();

	g_afTriangle_Edges[LEFT] = new float[1000];
	g_afTriangle_Edges[RIGHT] = new float[1000];
}



void detect_i2s(const PARAMS_I2S_INPUT& input, PARAMS_I2S_OUTPUT& output)
{
	int		iX, iY;							// counters
	float	fCntX, fCntY;					// counters
	float	fTriangle_X, fTriangle_Y;		// triangle position
	float	fAx, fBx;						// Line equation (y=Ax+b) for triangle X edge
	float	fAy, fBy;						// Line equation (y=Ax+b) for triangle Y edge
	int		iLabels;						// number of labels in the image
	Mat		imLabels, imStat, imCentroids;	// data for labeling
	int		iEdges_Len;

	fTriangle_X = fTriangle_Y = 0;

	// create and clear overlay
	if (input.GenerateOverlay()) {
		output._triangleOverlay.create(input._triangleImageSource.rows, input._triangleImageSource.cols, CV_8UC3);
		output._triangleOverlay.setTo(0);
	}

	// convert part to gray levels and HSV
	cvtColor(input._triangleImageSource, g_imTriangle_Input_GL, CV_RGB2GRAY);

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
	}

	// find - again connected componenets for the triangle detection
	iLabels = cv::connectedComponentsWithStats(g_imTriangle_T, imLabels, imStat, imCentroids, 8, CV_16U);

	// 2 labels only - triangle and background 
	if (iLabels == 2) {
		int iXS = imStat.at<int>(1, cv::CC_STAT_LEFT);
		int iYS = imStat.at<int>(1, cv::CC_STAT_TOP);
		int iHT = imStat.at<int>(1, cv::CC_STAT_HEIGHT);
		int iWT = imStat.at<int>(1, cv::CC_STAT_WIDTH);

		// find one triangle line
		iEdges_Len = 0;
		for (iY = iYS + 5; iY < iYS + iHT * 2 / 3; iY++) {	// loop on part of the triangle side
			float fMiddle_X = Detect_Edge_X(g_imTriangle_Input_GL, iXS - 10, iXS + 10, iY);	// detect single pixel edge
			g_afTriangle_Edges[input._side][iEdges_Len++] = fMiddle_X;
		}

		// estimate line for edge-pixels
		Find_Line_Data(g_afTriangle_Edges[input._side], iEdges_Len, fAx, fBx);
		if (input.GenerateOverlay())
			for (fCntY = 0; fCntY < iEdges_Len; fCntY += 1) {
				float fPos_X = fAx * fCntY + fBx;
				Draw_Point(output._triangleOverlay, fPos_X, fCntY + iYS + 5, 0, 255, 0, 1);
			}
		float fX0 = fBx;
		float fY0 = iYS + 5.f;
		float fV0x = fAx;
		float fV0y = 1;

		// find other triangle line
		iEdges_Len = 0;
		for (iX = iXS + 5; iX < iXS + iWT * 2 / 3; iX++) {	// loop on part of the triangle side
			float fMiddle_Y = Detect_Edge_Y(g_imTriangle_Input_GL, iYS - 10, iYS + 10, iX);	// detect single pixel edge
			g_afTriangle_Edges[input._side][iEdges_Len++] = fMiddle_Y;
		}

		// estimate line for edge-pixels
		Find_Line_Data(g_afTriangle_Edges[input._side], iEdges_Len, fAy, fBy);
		if (input.GenerateOverlay())
			for (fCntX = 0; fCntX < iEdges_Len; fCntX += 1) {
				float fPos_Y = fAy * fCntX + fBy;
				Draw_Point(output._triangleOverlay, fCntX + iXS + 5, fPos_Y, 0, 255, 0, 1);
			}
		float fX1 = iXS + 5.f;
		float fY1 = fBy;
		float fV1x = 1;
		float fV1y = fAy;

		// meeting betwen lines
		float fDet = -fV0x * fV1y + fV0y * fV1x;
		float fT1 = (fY1 - fY0) * fV0x - (fX1 - fX0) * fV0y;
		float fT0 = (fY1 - fY0) * fV1x - (fX1 - fX0) * fV1y;
	
		fT0 = fT0 / fDet;
		fT1 = fT1 / fDet;

		// triangle position
		fTriangle_X = fV0x * fT0 + fX0;
		fTriangle_Y = fV0y * fT0 + fY0;

		// overlay
		if (input.GenerateOverlay())
			Draw_Point(output._triangleOverlay, fTriangle_X, fTriangle_Y, 255, 0, 0);

		float ff = input.Pixel2MM_X() ;
		// set output data
		output._triangeCorner._x = (int)round((fTriangle_X + input._approxTriangeROI.left()) * input.Pixel2MM_X());
		output._triangeCorner._y = (int)round((fTriangle_Y + input._approxTriangeROI.top()) * input.Pixel2MM_Y());
		output._result = ALG_STATUS_SUCCESS;

		//if (input.GenerateOverlay())
		//	imwrite("e:\\temp\\res2.tif", output._triangleOverlay);	// ***
	}
	else {
		// set output data for failure
		output._triangeCorner._x = 0;
		output._triangeCorner._y = 0;
		output._result = ALG_STATUS_FAILED;
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
