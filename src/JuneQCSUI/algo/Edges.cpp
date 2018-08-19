
#include <iostream>
#include "Structures.h"
#include <opencv/cv.h> 
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp> 
#include <time.h>

using namespace std;
using namespace cv;


// float comparison function for qsort
int Compare_Float(const void* arg1, const void* arg2)
{
	if (*((float*)arg1) < *((float*)arg2))
		return -1;
	else if (*((float*)arg1) > *((float*)arg2))
		return 1;
	else
		return 0;
}



// find edge from iX1 (bright) to iX2 (dark)
float	Detect_Edge_X (const Mat& imImage, int iX1, int iX2, int iY)
{
	int		iX;							// counter
	float	afVal[200];					// values perpendicular to edge
	float	fDark, fBright, fMid;			// dark part of line, bright part and middle between them
	int		iPoint_X;						// point along X axis
	int		iStep = (iX1 > iX2) ? -1 : +1;	// step of tested line
	int		iLen = abs(iX2 - iX1 + 1);		// lengthes of tested line
	int		iMin_X = min(iX1, iX2 + 1);

	// values along tested line
	for (iX = iX1; iX != iX2; iX += iStep)
		if (iX >= 0 && iX < imImage.cols)
			afVal[iX - iMin_X] = (float)imImage.at<::byte>(iY, iX);

	// sort to find bright and dark pixels
	qsort(afVal, iLen, sizeof(afVal[0]), Compare_Float);
	fDark = afVal[2];
	fBright = afVal[iLen - 3];
	fMid = (fBright + fDark) / 2;

	// find point (in pixel resolution) which has around middle value
	iPoint_X = 0;
	for (iX = iX1; iX != iX2; iX += iStep)
		if (imImage.at<::byte>(iY, iX) < fMid) {
			iPoint_X = iX;
			break;
		}

	// get again values in the line (+/- 4 pixels)
	int iStart_X = iPoint_X - iStep * 4;
	int iEnd_X = iPoint_X + iStep * 4;
	iMin_X = min(iStart_X, iEnd_X + 1);

	for (iX = iStart_X; iX != iEnd_X; iX += iStep)
		afVal[iX - iMin_X] = (float)imImage.at<::byte>(iY, iX);

	// sort (again) to find bright and dark pixels
	qsort(afVal, 9, sizeof(afVal[0]), Compare_Float);
	fDark = afVal[1];
	fBright = afVal[iLen - 2];
	fMid = (fBright + fDark) / 2;

	// find (again) point (in pixel resolution) which has around middle value
	for (iX = iStart_X + 1; iX != iEnd_X; iX += iStep)
		if (imImage.at<::byte>(iY, iX) < fMid) {
			iPoint_X = iX;
			break;
		}

	// get the sub pixel resolution
	float fVal1 = (float)imImage.at<::byte>(iY, iPoint_X - iStep);
	float fVal2 = (float)imImage.at<::byte>(iY, iPoint_X);
	float fSub_X = (fVal1 - fMid) / (fVal1 - fVal2);

	return iPoint_X - iStep + fSub_X * iStep;
}



// find edge from iX1 (bright) to iX2 (dark)
float	Detect_Edge_Y(const Mat& imImage, int iY1, int iY2, int iX)
{
	int		iY;								// counter
	float	afVal[200];						// values perpendicular to edge
	float	fDark, fBright, fMid;			// dark part of line, bright part and middle between them
	int		iPoint_Y;						// point along Y axis
	int		iStep = (iY1 > iY2) ? -1 : +1;	// step of tested line
	int		iLen = abs(iY2 - iY1 + 1);		// lengthes of tested line
	int		iMin_Y = min(iY1, iY2 + 1);

	// values along tested line
	for (iY = iY1; iY != iY2; iY += iStep)
		if (iY >= 0 && iX < imImage.rows)
			afVal[iY - iMin_Y] = (float)imImage.at<::byte>(iY, iX);

	// sort to find bright and dark pixels
	qsort(afVal, iLen, sizeof(afVal[0]), Compare_Float);
	fDark = afVal[2];
	fBright = afVal[iLen - 3];
	fMid = (fBright + fDark) / 2;

	// find point (in pixel resolution) which has around middle value
	iPoint_Y = 0;
	for (iY = iY1; iY != iY2; iY += iStep)
		if (imImage.at<::byte>(iY, iX) < fMid) {
			iPoint_Y = iY;
			break;
		}

	// get again values in the line (+/- 4 pixels)
	int iStart_Y = iPoint_Y - iStep * 4;
	int iEnd_Y = iPoint_Y + iStep * 4;
	iMin_Y = min(iStart_Y, iEnd_Y + 1);

	for (iY = iStart_Y; iY != iEnd_Y; iY += iStep)
		afVal[iY - iMin_Y] = (float)imImage.at<::byte>(iY, iX);

	// sort (again) to find bright and dark pixels
	qsort(afVal, 9, sizeof(afVal[0]), Compare_Float);
	fDark = afVal[1];
	fBright = afVal[iLen - 2];
	fMid = (fBright + fDark) / 2;

	// find (again) point (in pixel resolution) which has around middle value
	for (iY = iStart_Y + 1; iY != iEnd_Y; iY += iStep)
		if (imImage.at<::byte>(iY, iX) < fMid) {
			iPoint_Y = iY;
			break;
		}

	// get the sub pixel resolution
	float fVal1 = (float)imImage.at<::byte>(iPoint_Y - iStep, iX);
	float fVal2 = (float)imImage.at<::byte>(iPoint_Y, iX);
	float fSub_Y = (fVal1 - fMid) / (fVal1 - fVal2);

	return iPoint_Y - iStep + fSub_Y * iStep;
}

