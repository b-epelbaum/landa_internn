#include "algo_edge_impl.h"
#include "cpu_load.h"
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/highgui/highgui.hpp> 

using namespace cv;
using namespace LandaJune;
using namespace Algorithms;

#ifndef byte
typedef unsigned char byte;
#endif


// global variables
float	*g_afPaperEdge_Edges = NULL;	// array for positions of paper edge
thread_local  Mat		g_imPaperEdge_Input_GL;

// function declarations
float	Detect_Edge_X(const Mat& imImage, int iX1, int iX2, int iY);
void	Find_Line_Data(float* afEdges, int iEdges_Len, float& fA, float &fB);
void	Draw_Point(Mat& imDisp, float fX, float fY, byte ucR, byte ucG, byte ucB, float fFactor = 1);



void detect_edge_init(const INIT_PARAMETER& initParam)
{
	detect_edge_shutdown();
	g_afPaperEdge_Edges = new float[1000];
}



void detect_edge(PARAMS_PAPEREDGE_INPUT_PTR input, PARAMS_PAPEREDGE_OUTPUT_PTR output)
{
	int		iY;				// counter
	float	fAx, fBx;		// Line equation (y=Ax+b) for paper edge

	// convert to gray scale
	cvtColor(*input->_stripImageSource, g_imPaperEdge_Input_GL, CV_RGB2GRAY);

	// define and clear overlay image
	if (input->GenerateOverlay()) {
		output->_edgeOverlay->create(input->_stripImageSource->rows, input->_stripImageSource->cols, CV_8UC3);
		output->_edgeOverlay->setTo(0);
	}

	int iEstimated_X = input->_approxDistanceFromEdgeX ;
	int iStart_X = max(iEstimated_X - 20, 0);
	int iEnd_X = min(iEstimated_X + 20, input->_stripImageSource->cols - 1);

	// paper edge
	int iCount = 0 ;		// toal number of edges
	int iFails = 0 ;		// number of fail edges
	int iEdges_Len = 0;

	for (iY = 100; iY < g_imPaperEdge_Input_GL.rows - 100; iY += 50) {
		float fMiddle_Paper = Detect_Edge_X(g_imPaperEdge_Input_GL, iEnd_X, iStart_X, iY);

		iCount ++ ;
		if (fMiddle_Paper < 0)
			iFails ++ ;

		g_afPaperEdge_Edges[iEdges_Len++] = fMiddle_Paper;
	}
	Find_Line_Data(g_afPaperEdge_Edges, iEdges_Len, fAx, fBx);

	if (iFails > iCount / 2)
		output->_result = ALG_STATUS_FAILED;

	// draw overlay
	if (input->GenerateOverlay())
		for (iY = 0; iY < g_imPaperEdge_Input_GL.rows; iY++) {
			float fPos_X = fAx * float(iY - 100) / 50 + fBx;
			Draw_Point(*output->_edgeOverlay, fPos_X, static_cast<float>(iY), 0, 255, 0);
		}

	// linear line parameters of the paper
	float fPaper_A = fAx / 50;
	float fPaper_B = fBx - 2;

	output->_exactDistanceFromEdgeX = round((fPaper_A * input->_triangeApproximateY + fPaper_B) * input->Pixel2MM_X() * 1000);
	output->_result = ALG_STATUS_SUCCESS;

	//if (input->GenerateOverlay())
	//	imwrite("e:\\temp\\res1.tif", output->_edgeOverlay);
}



void detect_edge_shutdown()
{
	if (g_afPaperEdge_Edges != NULL)
		delete [] g_afPaperEdge_Edges;
	g_afPaperEdge_Edges = NULL;
}
