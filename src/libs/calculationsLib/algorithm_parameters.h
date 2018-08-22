#pragma once

#include "algorithm_basic_types.h"
#include "ProcessParameter.h"

#include <optional>
#include "opencv/cv.h"

#define DECLARE_INPUT_PARAMETER(x,type,initval) type _##x = initval; public: type x() const { return _##x; } void set##x(const type val) { _##x = val; }

namespace LandaJune {
	namespace Core {
		class FrameRef;
	}
}

namespace LandaJune
{
	namespace Parameters
	{

		//////////////////////////////////////////////////
		///////////  Algorithm functions parameters 
		//////////////////////////////////////////////////

		//-----------------------------------------------
		//		Init Functions parameters
		//------------------------------------------

		struct INIT_PARAMETER
		{
			ROIRect _roiRect;
		};



		//------------------------------------------
		//		Paper Edge detection parameters
		//------------------------------------------

		struct ABSTRACT_INPUT
		{
			explicit ABSTRACT_INPUT(const Core::FrameRef * frame) : _frame(frame) {}

			const Core::FrameRef * _frame = nullptr;


			// other parameters
			DECLARE_INPUT_PARAMETER (Pixel2MM_X, double, 0.0)
			DECLARE_INPUT_PARAMETER (Pixel2MM_Y, double, 0.0)
			DECLARE_INPUT_PARAMETER(GenerateOverlay, bool, false)
		};


		// Input
		struct PARAMS_PAPEREDGE_INPUT : ABSTRACT_INPUT
		{
			PARAMS_PAPEREDGE_INPUT(const Core::FrameRef * frame, const SHEET_SIDE side)
				: ABSTRACT_INPUT(frame )
				, _side(side)
			{}

			SHEET_SIDE					_side = LEFT;
			uint32_t					_approxDistanceFromEdgeX = -1;
			uint32_t					_triangeApproximateY = -1;
			cv::Mat						_stripImageSource;
		};

		// output
		struct PARAMS_PAPEREDGE_OUTPUT
		{
			OUT_STATUS					_outStatus = ALG_STATUS_FAILED;
			uint32_t					_exactDistanceFromEdgeX = -1;
			cv::Mat						_edgeOverlay;

			///
			std::optional<PARAMS_PAPEREDGE_INPUT>		_input;
		};


		//------------------------------------------
		//		I2S detection parameters
		//------------------------------------------

		// Input
		struct PARAMS_I2S_INPUT :  ABSTRACT_INPUT
		{
			explicit PARAMS_I2S_INPUT(const Core::FrameRef * frame, const SHEET_SIDE side)
				: ABSTRACT_INPUT(frame)
				, _side(side)
			{}

			SHEET_SIDE					_side = LEFT;
			// ref color ?
			ROIRect						_approxTriangeROI {};
			cv::Mat						_triangleImageSource;
		};

		// output
		struct PARAMS_I2S_OUTPUT
		{
			OUT_STATUS					_outStatus = ALG_STATUS_FAILED;
			APOINT						_triangeCorner {};
			cv::Mat						_triangleOverlay;

			//
			std::optional<PARAMS_I2S_INPUT>		_input;
		};


		//------------------------------------------
		//		C2C ROI detection parameters
		//------------------------------------------

		// Input
		struct PARAMS_C2C_ROI_INPUT : ABSTRACT_INPUT
		{
			explicit PARAMS_C2C_ROI_INPUT(const Core::FrameRef * frame, const SHEET_SIDE side, std::vector<HSV> hsvs, const ROIRect& roi, int roiIndex)
				: ABSTRACT_INPUT(frame)
				, _side(side)
				, _colors(std::move(hsvs))
				, _ROI(roi)
				, _roiIndex(roiIndex)
			{}


			SHEET_SIDE				_side = LEFT;
			// scanner side

			std::vector<HSV>		_colors;
			ROIRect					_ROI;
			cv::Mat					_ROIImageSource;
			uint32_t				_roiIndex = -1;
		};


		// Output
		struct PARAMS_C2C_ROI_OUTPUT
		{
			OUT_STATUS					_outStatus = ALG_STATUS_FAILED;
			std::vector<OUT_STATUS>		_colorStatuses;
			std::vector<APOINT>			_colorCenters;
			std::vector<cv::Mat>		_colorOverlays;

			///
			std::optional<PARAMS_C2C_ROI_INPUT>		_input;
		};


		//------------------------------------------
		//		WAVE detection parameters
		//------------------------------------------

		// Input
		struct PARAMS_WAVE_INPUT : ABSTRACT_INPUT
		{
			explicit PARAMS_WAVE_INPUT(const Core::FrameRef * frame)
				: ABSTRACT_INPUT(frame)
			{}

			// image mat
			// scanner side
			std::vector<HSV>			_colors;
			ROIRect						_setROI;
		};


		// Output
		struct PARAMS_WAVE_OUTPUT
		{
			OUT_STATUS								_result = ALG_STATUS_FAILED;
			std::vector<std::vector<OUT_STATUS>>	_colorDetectionResults;
			std::vector<std::vector<APOINT>>		_colorCenters;
			std::vector<cv::Mat>					_colorOverlays;
			///
			std::optional<PARAMS_WAVE_INPUT>		_input;
		};


		//------------------------------------------
		//		C2C strip detection parameters
		//------------------------------------------

		// Input
		struct PARAMS_C2C_STRIP_INPUT : ABSTRACT_INPUT
		{

			explicit PARAMS_C2C_STRIP_INPUT(const Core::FrameRef * frame, const SHEET_SIDE side)
				: ABSTRACT_INPUT(frame)
				, _side(side)
				, _paperEdgeInput(frame, side)
				, _i2sInput(frame, side)
			{
				for (auto& roiInput : _c2cROIInputs)
				{
					roiInput._side = side;
					roiInput._frame = frame;
				}
			}
			SHEET_SIDE							_side = LEFT;
			PARAMS_PAPEREDGE_INPUT				_paperEdgeInput;
			PARAMS_I2S_INPUT					_i2sInput;
			std::vector<PARAMS_C2C_ROI_INPUT>	_c2cROIInputs;
		};


		// Output
		struct PARAMS_C2C_STRIP_OUTPUT
		{
			OUT_STATUS							_result = ALG_STATUS_FAILED;
			PARAMS_PAPEREDGE_OUTPUT				_paperEdgeOutput;
			PARAMS_I2S_OUTPUT					_i2sOutput;
			std::vector<PARAMS_C2C_ROI_OUTPUT>	_c2cROIOutputs;
			///
			std::optional<PARAMS_C2C_STRIP_INPUT>		_input;
		};



		//------------------------------------------
		//	C2C per sheet detection parameters
		//------------------------------------------

		// Input

		struct PARAMS_C2C_SHEET_INPUT : ABSTRACT_INPUT
		{
			explicit PARAMS_C2C_SHEET_INPUT(const Core::FrameRef * frame)
				: ABSTRACT_INPUT(frame)
				, _stripInputParamLeft(frame, LEFT)
				, _stripInputParamRight(frame, RIGHT)
			{
				
			}

			PARAMS_C2C_STRIP_INPUT _stripInputParamLeft;
			PARAMS_C2C_STRIP_INPUT _stripInputParamRight;
		};


		// Output
		struct PARAMS_C2C_SHEET_OUTPUT
		{
			OUT_STATUS _result = ALG_STATUS_FAILED;
			PARAMS_C2C_STRIP_OUTPUT _stripOutputParameterLeft;
			PARAMS_C2C_STRIP_OUTPUT _stripOutputParameterRight;
			///
			std::optional<PARAMS_C2C_SHEET_INPUT>		_input;
		};
	}
}
