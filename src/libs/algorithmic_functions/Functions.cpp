
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

	if (iY < 0 || iY >= imImage.rows)
		return -999 ;

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
		if (iX >= 0 && iX < imImage.cols && imImage.at<byte>(iY, iX) < fMid) {
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

	if (iX < 0 || iX >= imImage.cols)
		return -999 ;

	// values along tested line
	for (iY = iY1; iY != iY2; iY += iStep)
		if (iY >= 0 && iY < imImage.rows) {
			unsigned char dd = imImage.at<byte>(iY, iX) ;
			afVal[iY - iMin_Y] = (float)imImage.at<byte>(iY, iX);
			float yy = afVal[iY - iMin_Y] ;
		}

	// sort to find bright and dark pixels
	qsort(afVal, iLen, sizeof(afVal[0]), Compare_Float);
	fDark = afVal[2];
	fBright = afVal[iLen - 3];
	fMid = (fBright + fDark) / 2;

	// find point (in pixel resolution) which has around middle value
	iPoint_Y = 0;
	for (iY = iY1; iY != iY2; iY += iStep)
		if (iY >= 0 && iY < imImage.rows && imImage.at<byte>(iY, iX) < fMid) {
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

	// check in image
	if (iPoint_Y - iStep < 0)
		return 0 ;

	// get the sub pixel resolution
	float fVal1 = (float)imImage.at<byte>(iPoint_Y - iStep, iX);
	float fVal2 = (float)imImage.at<byte>(iPoint_Y, iX);
	float fSub_Y = (fVal1 - fMid) / (fVal1 - fVal2);

	return iPoint_Y - iStep + fSub_Y * iStep;
}



// when given points on edge (afEdges) - the linear line coefficients are found
// Use Ransac like method
void	Find_Line_Data(float* afEdges, int iEdges_Len, float& fA, float &fB, float fThreshold)
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
				if (fabs(fY + iCnt3 * fSlope - afEdges[iCnt3]) < fThreshold)
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
		if (fabs(fY + iCnt3 * fSlope - afEdges[iCnt3]) < fThreshold) {
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




Mat	H_Diff(const Mat& imH1, int iH)
{
	Mat	imDiff1 = abs(imH1 - iH);
	Mat	imDiff2 = abs(imH1 - iH + 180);
	Mat	imDiff3 = abs(imH1 - iH - 180);
	Mat imDiff = min(min(imDiff1, imDiff2), imDiff2);

	return imDiff;
}



void	Template_Data (const Mat& imTemplate, float& fAve, float& fStd, int iMode)
{
	int iX, iY;
	int iMatch = 0;

	float fSum_1 = 0;
	float fSum_11 = 0;
	
	for (iX = 0; iX < imTemplate.cols; iX++)
		for (iY = 0; iY < imTemplate.rows; iY++) {

			float fVal1 = (float)imTemplate.at<byte>(iY, iX);

			if (iMode == 1 && iX < imTemplate.cols / 2)
				continue;
			if (iMode == 2 && iX > imTemplate.cols / 2)
				continue;
			if (iMode == 3 && iY < imTemplate.rows / 2)
				continue;
			if (iMode == 4 && iY > imTemplate.rows / 2)
				continue;

			if (fVal1 > 0) {
				fSum_1 += fVal1;
				fSum_11 += fVal1 * fVal1;
				iMatch++;
			}
		}

	if (iMatch > 0) {
		fSum_1 /= iMatch;
		fSum_11 /= iMatch;
	}

	fAve = fSum_1 ;
	fStd = sqrtf (fSum_11 - fSum_1 * fSum_1);
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
		for (iY = 0; iY < imTemplate.rows; iY++) {

			// out of image - correlatiion is minimum
			if (iX + iDx < 0 || iX + iDx >= imImage.cols - 1 || iY + iDy < 0 || iY + iDy >= imImage.rows - 1)
				continue ;

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

	if (iMatch < imTemplate.cols * imTemplate.rows * 0.75) {
		fCorrelation = -1 ;
		return ;
	}

	float fDenom = sqrtf((fSum_11 - fSum_1 * fSum_1) * (fSum_22 - fSum_2 * fSum_2));
	float fNomin = (fSum_12 - fSum_1 * fSum_2);

	fCorrelation = (fDenom > 0) ? fNomin / fDenom : 0;
}



// Correlate template in an image
// Pixels are not tested if has different Z (not used now)
// fCorrelation - the correlation
// iMatch - number of matched pixels
// iMode - what part to correlate (0 - all template) - currently it is the only used mode
void	Correlate_Templates(const Mat& imImage, const Mat& imTemplate, int iDx, int iDy, float& fCorrelation, int& iMatch, float fTempl_Ave, float fTempl_Std, int iMode)
{
	int iX, iY;
	iMatch = 0;

	float fSum_1 = 0;
	float fSum_11 = 0;
	float fSum_12 = 0;

	fCorrelation = -1 ;

	if (iDx < -5 || iDy < -5 || iDx + imTemplate.cols >= imImage.cols + 5 || iDy + imTemplate.rows >= imImage.rows + 5)
		return;

	for (iX = 0; iX < imTemplate.cols; iX++)
		for (iY = 0; iY < imTemplate.rows; iY++) {

			// out of image - correlatiion is minimum
			if (iX + iDx < 0 || iX + iDx >= imImage.cols - 1 || iY + iDy < 0 || iY + iDy >= imImage.rows - 1)
				continue ;

			float fVal1 = (float)imImage.at<byte>(iY + iDy, iX + iDx);
			float fVal2 = (float)imTemplate.at<byte>(iY, iX);

			if (iMode > 0) {
				if (iMode == 1 && iX < imTemplate.cols / 2)
					continue;
				if (iMode == 2 && iX > imTemplate.cols / 2)
					continue;
				if (iMode == 3 && iY < imTemplate.rows / 2)
					continue;
				if (iMode == 4 && iY > imTemplate.rows / 2)
					continue;
			}

			if (fVal2 > 0) {
				fSum_1 += fVal1;
				fSum_11 += fVal1 * fVal1;
				fSum_12 += fVal1 * fVal2;
				iMatch++;
			}
		}

	if (iMatch > 0) {
		fSum_1 /= iMatch;
		fSum_11 /= iMatch;
		fSum_12 /= iMatch;
	}

	if (iMatch < imTemplate.cols * imTemplate.rows * 0.75) {
		fCorrelation = -1;
		return;
	}

	float fDenom = sqrtf(fSum_11 - fSum_1 * fSum_1) * fTempl_Std ;
	float fNomin = (fSum_12 - fSum_1 * fTempl_Ave);

	fCorrelation = (fDenom > 0) ? fNomin / fDenom : 0;
}



// 3*3 parabolic estimation
// a*X^2 + b*Y^2 + c*XY + d*X + e*Y + f
void	Estimate_Parabolic_3x3(const Mat& imCorr, float& fDx, float& fDy)
{
	float fA00 = imCorr.at<float>(0, 0);
	float fA10 = imCorr.at<float>(1, 0);
	float fA20 = imCorr.at<float>(2, 0);
	float fA01 = imCorr.at<float>(0, 1);
	float fA11 = imCorr.at<float>(1, 1);
	float fA21 = imCorr.at<float>(2, 1);
	float fA02 = imCorr.at<float>(0, 2);
	float fA12 = imCorr.at<float>(1, 2);
	float fA22 = imCorr.at<float>(2, 2);
	float fCX2 = +fA00 + fA10 + fA20 + fA02 + fA12 + fA22;
	float fCY2 = +fA00 + fA01 + fA02 + fA20 + fA21 + fA22;
	float fCXY = +fA00 - fA02 - fA20 + fA22;
	float fCX1 = -fA00 - fA10 - fA20 + fA02 + fA12 + fA22;
	float fCY1 = -fA00 - fA01 - fA02 + fA20 + fA21 + fA22;
	float fC1 = +fA00 + fA10 + fA20 + fA01 + fA11 + fA21 + fA02 + fA12 + fA22;

	float fC = fCXY / 4;
	float fD = fCX1 / 6;
	float fE = fCY1 / 6;

	Mat tABF = (Mat_<float>(3, 3) << 0.5000, 0.0000, -0.3333, 0.0000, 0.5000, -0.3333, -0.3333, -0.3333, 0.5556);
	Mat tRes = tABF * (Mat_<float>(3, 1) << fCX2, fCY2, fC1);
	float fA = tRes.at<float>(0);
	float fB = tRes.at<float>(1);
	float fF = tRes.at<float>(2);

	Mat tMin = (Mat_<float>(2, 2) << 2 * fA, fC, fC, 2 * fB);
	tMin = tMin.inv();
	tRes = tMin * (Mat_<float>(2, 1) << -fD, -fE);
	fDx = tRes.at<float>(0);
	fDy = tRes.at<float>(1);
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



void Find_Template_In_Image(const Mat& imImage, const Mat& imTemplate, Mat& tCorr_Matrix, int iTemplate_X, int iTemplate_Y, int iHalf_iSearch_Size, float& fDx, float& fDy, float& fCorr, int iMode)
{
	const int iSearch_Size = iHalf_iSearch_Size * 2 + 1;

	// correlation matrix
	tCorr_Matrix.setTo(-1);

	// temnplate average anb s.t.d
	float fTempl_Ave, fTempl_Std ;
	Template_Data (imTemplate, fTempl_Ave, fTempl_Std, iMode) ;

	for (int iY1 = -iHalf_iSearch_Size; iY1 <= iHalf_iSearch_Size; iY1++) {
		for (int iX1 = -iHalf_iSearch_Size; iX1 <= iHalf_iSearch_Size; iX1++) {
			float	fCorrelation;
			int		iMatch;

//			Correlate_Templates(imImage, imTemplate, iX1 + iTemplate_X - imTemplate.cols / 2, iY1 + iTemplate_Y - imTemplate.rows / 2, fCorrelation, iMatch, iMode);
			Correlate_Templates(imImage, imTemplate, iX1 + iTemplate_X - imTemplate.cols / 2, iY1 + iTemplate_Y - imTemplate.rows / 2, fCorrelation, iMatch, fTempl_Ave, fTempl_Std, iMode);
			tCorr_Matrix.at<float>(iY1 + iHalf_iSearch_Size, iX1 + iHalf_iSearch_Size) = fCorrelation;
		}
	}

	Point tMax_Pos, tMin_Pos;
	double dMax_Val, dMin_Val;
	minMaxLoc(tCorr_Matrix, &dMin_Val, &dMax_Val, &tMin_Pos, &tMax_Pos);

	fCorr = (float)dMax_Val;

	// clear data of at edge of correlation array
	float	fSub_Pixel_X = 0;
	float	fSub_Pixel_Y = 0;

	// If edge of correlation matrix - failed
	if (tMax_Pos.x == 0 || tMax_Pos.x >= iSearch_Size - 1 || tMax_Pos.y == 0 || tMax_Pos.y >= iSearch_Size - 1)
		fCorr = 0;
	else {	// assign parabolic distance

		//Estimate_Parabolic_3x3 (tCorr_Matrix(Rect(tMax_Pos.x - 1, tMax_Pos.y - 1, 3, 3)), fSub_Pixel_X, fSub_Pixel_Y) ;
		fSub_Pixel_X = fParabolic_Estimation(tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x - 1), tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x), tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x + 1));
		fSub_Pixel_Y = fParabolic_Estimation(tCorr_Matrix.at<float>(tMax_Pos.y - 1, tMax_Pos.x), tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x), tCorr_Matrix.at<float>(tMax_Pos.y + 1, tMax_Pos.x));
	}

	fDx = tMax_Pos.x - iHalf_iSearch_Size + fSub_Pixel_X;
	fDy = tMax_Pos.y - iHalf_iSearch_Size + fSub_Pixel_Y;
}
