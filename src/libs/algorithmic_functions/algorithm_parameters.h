#pragma once

#include "algorithm_basic_types.h"
#include <optional>
#include <vector>

///////////////
/// PPL
#include <ppl.h>
#include <concurrent_vector.h>

#define USE_PPL

#include "common/june_errors.h"
#include "opencv/cv.h"
#include "include/format.h"

#define DECLARE_INPUT_PARAMETER(x,type,initval) public: type _##x = initval; public: type x() const { return _##x; } public: void set##x(const type val) { _##x = val; }

namespace LandaJune {
	namespace Core {
		class FrameRef;
	}
}

namespace LandaJune
{
	namespace Algorithms
	{

		//////////////////////////////////////////////////
		///////////  Algorithm functions parameters 
		//////////////////////////////////////////////////

		//-----------------------------------------------
		//		Init Functions parameters
		//------------------------------------------

		class INIT_PARAMETER
		{
		public:
			ROIRect _roiRect;
		};

		class C2C_ROI_INIT_PARAMETER : INIT_PARAMETER
		{
		public:

			C2C_ROI_INIT_PARAMETER(const INIT_PARAMETER& other) : INIT_PARAMETER(other) {}
			cv::Mat	_templateImage;
		};

		class WAVE_INIT_PARAMETER : INIT_PARAMETER
		{
		public:

			WAVE_INIT_PARAMETER(const INIT_PARAMETER& other) : INIT_PARAMETER(other) {}
			cv::Mat	_templateImage;
		};

		////////////////////////////////////////////////
		////////////    INPUT CLASSES  /////////////////
		////////////////////////////////////////////////

		//------------------------------------------
		//		Abstract Input  class
		//------------------------------------------

		class ABSTRACT_INPUT
		{
			public:
				explicit ABSTRACT_INPUT(const Core::FrameRef * frame) : _frame(frame){}
				virtual ~ABSTRACT_INPUT() = default;

				virtual std::string getElementName() const  = 0;

				// other parameters
				DECLARE_INPUT_PARAMETER (Pixel2MM_X, double, 0.0)
				DECLARE_INPUT_PARAMETER (Pixel2MM_Y, double, 0.0)
				DECLARE_INPUT_PARAMETER(GenerateOverlay, bool, false)
				
				const Core::FrameRef * _frame = nullptr;
				std::weak_ptr<ABSTRACT_INPUT> _parent;
		};

		//------------------------------------------
		//		Frame I2S detection Input  class
		//------------------------------------------

		class PARAMS_I2S_INPUT :   public ABSTRACT_INPUT
		{
			public:
				explicit PARAMS_I2S_INPUT(const Core::FrameRef * frame, const SHEET_SIDE side)
					: ABSTRACT_INPUT(frame)
					, _side(side)
					{}

				std::string getElementName() const  override
				{
					return fmt::format("I2S_{0}", SIDE_NAMES[_side]);
				}

				SHEET_SIDE					_side = LEFT;
				// ref color ?
				ROIRect						_approxTriangeROI {};
				cv::Mat						_triangleImageSource;
		};

		//------------------------------------------
		//		C2C ROI detection Input parameters
		//------------------------------------------

		// Input
		class PARAMS_C2C_ROI_INPUT :  public ABSTRACT_INPUT
		{
			public:
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

				std::string getElementName() const  override
				{
					return fmt::format("C2C_{0}_{1}_[{2},{3}]", SIDE_NAMES[_side], _roiIndex, _ROI.left(), _ROI.top());
				}
		};

		//------------------------------------------
		//		Edge detection Input parameters
		//------------------------------------------

		class PARAMS_PAPEREDGE_INPUT : public ABSTRACT_INPUT
		{
			public:
				PARAMS_PAPEREDGE_INPUT(const Core::FrameRef * frame, const SHEET_SIDE side)
					: ABSTRACT_INPUT(frame )
					, _side(side)
					{}

					std::string getElementName() const  override
					{
						return fmt::format("EDGE_{0}", SIDE_NAMES[_side]);
					}

					SHEET_SIDE					_side = LEFT;
					uint32_t					_approxDistanceFromEdgeX = -1;
					uint32_t					_triangeApproximateY = -1;
					cv::Mat						_stripImageSource;

					std::weak_ptr<ABSTRACT_INPUT> _parent;
		};

		//------------------------------------------
		//		WAVE detection Input parameters
		//------------------------------------------

		// Input
		class PARAMS_WAVE_INPUT :  public ABSTRACT_INPUT
		{
			public:
			explicit PARAMS_WAVE_INPUT(const Core::FrameRef * frame)
				: ABSTRACT_INPUT(frame)
				{}

				std::string getElementName() const  override
				{
					return "WAVE";
				}

			cv::Mat						_waveImageSource;
			// scanner side
			HSV							_circleColor;
			ROIRect						_waveROI;
			int32_t						_circlesCount = 0;
		};

		//------------------------------------------
		//		Frame Strip detection Input parameters
		//------------------------------------------
		class PARAMS_C2C_STRIP_INPUT :  public ABSTRACT_INPUT
		{
			public:
			explicit PARAMS_C2C_STRIP_INPUT(const Core::FrameRef * frame, const SHEET_SIDE side)
				: ABSTRACT_INPUT(frame)
				, _side(side)
				, _paperEdgeInput(frame, side)
				, _i2sInput(frame, side)
			{
			}

			std::string getElementName() const  override
			{
				return fmt::format("Strip_{0}", SIDE_NAMES[_side]);
			}

			SHEET_SIDE							_side = LEFT;
			PARAMS_PAPEREDGE_INPUT				_paperEdgeInput;
			PARAMS_I2S_INPUT					_i2sInput;
			std::vector<PARAMS_C2C_ROI_INPUT>	_c2cROIInputs;
		};

		//------------------------------------------
		//	Sheet detection Input parameters
		//------------------------------------------
		class PARAMS_C2C_SHEET_INPUT :  public ABSTRACT_INPUT
		{
			public:
				explicit PARAMS_C2C_SHEET_INPUT(const Core::FrameRef * frame)
					: ABSTRACT_INPUT(frame)
					, _stripInputParamLeft(frame, LEFT)
					, _stripInputParamRight(frame, RIGHT)
					, _waveInputs(0, PARAMS_WAVE_INPUT{frame})
					{}

				std::string getElementName() const  override
				{
					return "Frame";
				}

				PARAMS_C2C_STRIP_INPUT				_stripInputParamLeft;
				PARAMS_C2C_STRIP_INPUT				_stripInputParamRight;
				std::vector<PARAMS_WAVE_INPUT>	    _waveInputs;
		};



		//////////////////////////////////////////////////////////////////////////
		///////////////////////////    OUTPUT CLASSES  ///////////////////////////
		//////////////////////////////////////////////////////////////////////////

		//------------------------------------------
		//		Abstract Output  class
		//------------------------------------------
		class ABSTRACT_OUTPUT
		{
			public:
				virtual ~ABSTRACT_OUTPUT() = default;
				ABSTRACT_OUTPUT() = default;

				virtual std::string getElementName() = 0;
				virtual std::optional<cv::Mat> overlay() const  = 0;

				OUT_STATUS	_result = ALG_STATUS_FAILED;
		};

		//------------------------------------------
		//		Frame I2S detection Output  class
		//------------------------------------------
		class PARAMS_I2S_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:
				APOINT						_triangeCorner {};
				cv::Mat						_triangleOverlay;

				//
				std::optional<PARAMS_I2S_INPUT>		_input;

				std::string getElementName() override
				{
					return fmt::format("I2S_{0}_overlay", SIDE_NAMES[_input->_side]);
				}
			
				std::optional<cv::Mat> overlay()  const override
				{
					return _triangleOverlay;
				}
		};
		
		//------------------------------------------
		//		Frame EDGE detection Output  class
		//------------------------------------------
		class PARAMS_PAPEREDGE_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:
				uint32_t					_exactDistanceFromEdgeX = -1;
				cv::Mat						_edgeOverlay;
			
				std::optional<PARAMS_PAPEREDGE_INPUT>		_input;
				std::string getElementName() override
				{
					return fmt::format("Edge_{0}_overlay", SIDE_NAMES[_input->_side]);
				}
				
				std::optional<cv::Mat> overlay() const override
				{
					return _edgeOverlay;
				}
		};

		//------------------------------------------
		//		C2C detection Output  class
		//------------------------------------------
		class PARAMS_C2C_ROI_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:

			std::vector<OUT_STATUS>		_colorStatuses;
			std::vector<APOINT>			_colorCenters;
			cv::Mat						_colorOverlay;

			///
			std::optional<PARAMS_C2C_ROI_INPUT>		_input;
			std::string getElementName() override
			{
				return fmt::format("C2C_{0}_{1}_[{2},{3}]_overlay", SIDE_NAMES[_input->_side], _input->_roiIndex, _input->_ROI.left(), _input->_ROI.top());
			}

			std::optional<cv::Mat> overlay() const  override
			{
				return _colorOverlay;
			}
		};

		//------------------------------------------
		//		WAVE detection Output  class
		//------------------------------------------
		class PARAMS_WAVE_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:
				std::vector<OUT_STATUS>					_colorDetectionResults;
				std::vector<APOINT>						_colorCenters;
				cv::Mat									_colorOverlay;
				///
				std::optional<PARAMS_WAVE_INPUT>		_input;
				std::string getElementName() override
				{
					return fmt::format("Wave_[{0}]", _input->_circleColor._colorName);
				}

				std::optional<cv::Mat> overlay()  const override
				{
					return _colorOverlay;
				}
		};

		//------------------------------------------
		//		Strip detection Output  class
		//------------------------------------------
		class PARAMS_C2C_STRIP_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:
				PARAMS_PAPEREDGE_OUTPUT									_paperEdgeOutput;
				PARAMS_I2S_OUTPUT										_i2sOutput;
				Concurrency::concurrent_vector<PARAMS_C2C_ROI_OUTPUT>	_c2cROIOutputs;
				
			///
				std::optional<PARAMS_C2C_STRIP_INPUT>					_input;
				
				std::string getElementName() override
				{
					return fmt::format("Strip_{0}_overlay", SIDE_NAMES[_input->_side]);
				}
				
				std::optional<cv::Mat> overlay()  const override
				{
					return std::nullopt;
				}
		};


		//------------------------------------------
		//		Sheet detection Output  class
		//------------------------------------------
		class PARAMS_C2C_SHEET_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:
				PARAMS_C2C_STRIP_OUTPUT										_stripOutputParameterLeft;
				PARAMS_C2C_STRIP_OUTPUT										_stripOutputParameterRight;
				Concurrency::concurrent_vector<PARAMS_WAVE_OUTPUT>			_waveOutputs;
				///
				std::optional<PARAMS_C2C_SHEET_INPUT>		_input;
				std::string getElementName() override
				{
					return "Frame_overlay";
				}
				std::optional<cv::Mat> overlay()  const override
				{
					return std::nullopt;
				}
		};
	}
}
