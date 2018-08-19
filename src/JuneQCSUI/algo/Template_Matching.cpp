

#include <iostream>
#include "Structures.h"
#include <opencv/cv.h> 
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp> 
#include <time.h>

using namespace std;
using namespace cv;


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

			if (iX + iDx < 0 || iX + iDx >= imImage.cols || iY + iDy < 0 || iY + iDy >= imImage.rows)
				continue ;

			float fVal1 = (::byte)imImage.at<::byte>(iY + iDy, iX + iDx);
			float fVal2 = (::byte)imTemplate.at<::byte>(iY, iX);

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



// find a template (imTemplate) in the image (imImage) where the eatimates position is (iTemplate_X, iTemplate_Y)
void Find_Template_In_Image(const Mat& imImage, const Mat& imTemplate, int iTemplate_X, int iTemplate_Y, int iHalf_iSearch_Size, float& fDx, float& fDy, float& fCorr, int iMode)
{
	const int iSearch_Size = iHalf_iSearch_Size * 2 + 1;

	// correlation matrix
	Mat		tCorr_Matrix(iSearch_Size, iSearch_Size, CV_32F);
	tCorr_Matrix.setTo(-1);

	for (int iY1 = -iHalf_iSearch_Size; iY1 <= iHalf_iSearch_Size; iY1++) {
		for (int iX1 = -iHalf_iSearch_Size; iX1 <= iHalf_iSearch_Size; iX1++) {
			float	fCorrelation;
			int		iMatch;

			Correlate_Templates(imImage, imTemplate, iX1 + iTemplate_X - imTemplate.cols / 2, iY1 + iTemplate_Y - imTemplate.rows / 2, fCorrelation, iMatch, iMode);
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
		fSub_Pixel_X = fParabolic_Estimation(tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x - 1), tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x), tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x + 1));
		fSub_Pixel_Y = fParabolic_Estimation(tCorr_Matrix.at<float>(tMax_Pos.y - 1, tMax_Pos.x), tCorr_Matrix.at<float>(tMax_Pos.y, tMax_Pos.x), tCorr_Matrix.at<float>(tMax_Pos.y + 1, tMax_Pos.x));
	}

	fDx = tMax_Pos.x - iHalf_iSearch_Size + fSub_Pixel_X;
	fDy = tMax_Pos.y - iHalf_iSearch_Size + fSub_Pixel_Y;
}
