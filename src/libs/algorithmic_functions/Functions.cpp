
#include <opencv/cv.h> 

using namespace cv;

#ifndef byte
typedef unsigned char byte;
#endif


short	g_anCount[40000];



// draw point on image - for overlay
void	Draw_Point(Mat& imDisp, float fX, float fY, byte ucR, byte ucG, byte ucB, float fFactor = 1)
{
	int	iPos_X = (int)round(fX * fFactor);
	int	iPos_Y = (int)round(fY * fFactor);

	if (iPos_X < 0 || iPos_X >= imDisp.cols || iPos_Y < 0 || iPos_Y >= imDisp.rows)
		return;

	imDisp.at<Vec3b>(iPos_Y, iPos_X)[0] = ucB;
	imDisp.at<Vec3b>(iPos_Y, iPos_X)[1] = ucG;
	imDisp.at<Vec3b>(iPos_Y, iPos_X)[2] = ucR;
}



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
float	Detect_Edge_X(const Mat& imImage, int iX1, int iX2, int iY)
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
			afVal[iX - iMin_X] = (float)imImage.at<byte>(iY, iX);

	// sort to find bright and dark pixels
	qsort(afVal, iLen, sizeof(afVal[0]), Compare_Float);
	fDark = afVal[2];
	fBright = afVal[iLen - 3];
	fMid = (fBright + fDark) / 2;

	// no edge
	if (fBright - fDark < 50)
		return -999 ;

	// find point (in pixel resolution) which has around middle value
	iPoint_X = 0;
	for (iX = iX1; iX != iX2; iX += iStep)
		if (imImage.at<byte>(iY, iX) < fMid) {
			iPoint_X = iX;
			break;
		}

	// get again values in the line (+/- 4 pixels)
	int iStart_X = min(max(iPoint_X - iStep * 4, 0), imImage.cols - 1);;
	int iEnd_X = min(max(iPoint_X + iStep * 4, 0), imImage.cols - 1);
	iMin_X = min(iStart_X, iEnd_X + 1);

	for (iX = iStart_X; iX != iEnd_X; iX += iStep)
		afVal[iX - iMin_X] = (float)imImage.at<byte>(iY, iX);

	// sort (again) to find bright and dark pixels
	qsort(afVal, 9, sizeof(afVal[0]), Compare_Float);
	fDark = afVal[1];
	fBright = afVal[iLen - 2];
	fMid = (fBright + fDark) / 2;

	// find (again) point (in pixel resolution) which has around middle value
	for (iX = iStart_X + 1; iX != iEnd_X; iX += iStep)
		if (imImage.at<byte>(iY, iX) < fMid) {
			iPoint_X = iX;
			break;
		}

	// get the sub pixel resolution
	float fVal1 = (float)imImage.at<byte>(iY, iPoint_X - iStep);
	float fVal2 = (float)imImage.at<byte>(iY, iPoint_X);
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
			afVal[iY - iMin_Y] = (float)imImage.at<byte>(iY, iX);

	// sort to find bright and dark pixels
	qsort(afVal, iLen, sizeof(afVal[0]), Compare_Float);
	fDark = afVal[2];
	fBright = afVal[iLen - 3];
	fMid = (fBright + fDark) / 2;

	// find point (in pixel resolution) which has around middle value
	iPoint_Y = 0;
	for (iY = iY1; iY != iY2; iY += iStep)
		if (imImage.at<byte>(iY, iX) < fMid) {
			iPoint_Y = iY;
			break;
		}

	// get again values in the line (+/- 4 pixels)
	int iStart_Y = max(iPoint_Y - iStep * 4, 0);
	int iEnd_Y = min(iPoint_Y + iStep * 4, imImage.rows - 1);
	iMin_Y = min(iStart_Y, iEnd_Y + 1);

	for (iY = iStart_Y; iY != iEnd_Y; iY += iStep)
		afVal[iY - iMin_Y] = (float)imImage.at<byte>(iY, iX);

	// sort (again) to find bright and dark pixels
	qsort(afVal, 9, sizeof(afVal[0]), Compare_Float);
	fDark = afVal[1];
	fBright = afVal[iLen - 2];
	fMid = (fBright + fDark) / 2;

	// find (again) point (in pixel resolution) which has around middle value
	for (iY = iStart_Y + 1; iY != iEnd_Y; iY += iStep)
		if (imImage.at<byte>(iY, iX) < fMid) {
			iPoint_Y = iY;
			break;
		}

	// get the sub pixel resolution
	float fVal1 = (float)imImage.at<byte>(iPoint_Y - iStep, iX);
	float fVal2 = (float)imImage.at<byte>(iPoint_Y, iX);
	float fSub_Y = (fVal1 - fMid) / (fVal1 - fVal2);

	return iPoint_Y - iStep + fSub_Y * iStep;
}



// when given points on edge (afEdges) - the linear line coefficients are found
// Use Ransac like method
void	Find_Line_Data(float* afEdges, int iEdges_Len, float& fA, float &fB)
{
	const int STEP = max(iEdges_Len / 50, 1);

	if (iEdges_Len > 200) {
		// too many edges - overflow of aiCount
		exit(88);
	}

	int	iCnt1, iCnt2, iCnt3;
	float fSlope, fY;

	memset(g_anCount, 0, iEdges_Len * iEdges_Len * sizeof(g_anCount[0]));

	for (iCnt1 = 0; iCnt1 < iEdges_Len; iCnt1 += STEP)
		for (iCnt2 = iCnt1 + 10; iCnt2 < iEdges_Len; iCnt2 += STEP) {
			fSlope = (afEdges[iCnt2] - afEdges[iCnt1]) / (iCnt2 - iCnt1);
			fY = afEdges[iCnt1] - fSlope * iCnt1;

			for (iCnt3 = 0; iCnt3 < iEdges_Len; iCnt3 += STEP)
				if (fabs(fY + iCnt3 * fSlope - afEdges[iCnt3]) < 1)
					g_anCount[iCnt1 + iCnt2 * iEdges_Len] ++;
		}

	int iMax_Count = 0;
	int iMax_Index1 = 0;
	int iMax_Index2 = 0;
	for (iCnt1 = 0; iCnt1 < iEdges_Len; iCnt1 += STEP)
		for (iCnt2 = iCnt1 + 10; iCnt2 < iEdges_Len; iCnt2 += STEP)
			if (g_anCount[iCnt1 + iCnt2 * iEdges_Len] > iMax_Count) {
				iMax_Count = g_anCount[iCnt1 + iCnt2 * iEdges_Len];
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
}
