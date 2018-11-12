#pragma once

#include "algorithm_basic_types.h"
#include <optional>
#include <utility>
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

		template<typename TO, typename FROM>
		std::unique_ptr<TO> static_unique_pointer_cast (std::unique_ptr<FROM>&& old)
		{
			return std::unique_ptr<TO>{static_cast<TO*>(old.release())};
		}

		//////////////////////////////////////////////////
		///////////  Algorithm functions parameters 
		//////////////////////////////////////////////////

		//-----------------------------------------------
		//		Init Functions parameters
		//------------------------------------------

		class INIT_PARAMETER
		{
			public:
				INIT_PARAMETER () = default;
				INIT_PARAMETER (const ROIRect& rc) : _roiRect (rc) {}
				explicit INIT_PARAMETER (const INIT_PARAMETER& other) = delete;
				explicit INIT_PARAMETER (INIT_PARAMETER&& other) = delete;
				const INIT_PARAMETER & operator = (const INIT_PARAMETER& other) = delete;
				INIT_PARAMETER & operator = (INIT_PARAMETER &&) = delete;
				~INIT_PARAMETER () = default;

				ROIRect _roiRect;
		};

		class C2C_ROI_INIT_PARAMETER : public INIT_PARAMETER
		{
			public:
				C2C_ROI_INIT_PARAMETER () = default;
				explicit C2C_ROI_INIT_PARAMETER (const C2C_ROI_INIT_PARAMETER& other) = delete;
				explicit C2C_ROI_INIT_PARAMETER (C2C_ROI_INIT_PARAMETER&& other) = delete;
				const C2C_ROI_INIT_PARAMETER & operator = (const C2C_ROI_INIT_PARAMETER& other) = delete;
				C2C_ROI_INIT_PARAMETER & operator = (C2C_ROI_INIT_PARAMETER &&) = delete;
				~C2C_ROI_INIT_PARAMETER () = default;

				cv::Mat	_templateImage;
		};


		class WAVE_INIT_PARAMETER : public INIT_PARAMETER
		{
			public:
				WAVE_INIT_PARAMETER () = default;
				explicit WAVE_INIT_PARAMETER (const WAVE_INIT_PARAMETER& other) = delete;
				explicit WAVE_INIT_PARAMETER (WAVE_INIT_PARAMETER&& other) = delete;
				const WAVE_INIT_PARAMETER & operator = (const WAVE_INIT_PARAMETER& other) = delete;
				WAVE_INIT_PARAMETER & operator = (WAVE_INIT_PARAMETER &&) = delete;
				~WAVE_INIT_PARAMETER () = default;
	
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
				explicit ABSTRACT_INPUT (const ABSTRACT_INPUT& other) = delete;
				explicit ABSTRACT_INPUT (ABSTRACT_INPUT&& other) = delete;
				const ABSTRACT_INPUT & operator = (const ABSTRACT_INPUT& other) = delete;
				ABSTRACT_INPUT & operator = (ABSTRACT_INPUT &&) = delete;
				virtual ~ABSTRACT_INPUT () = default;

				virtual std::string getElementName() const  = 0;

				// other parameters
				DECLARE_INPUT_PARAMETER (Pixel2MM_X, double, 0.0)
				DECLARE_INPUT_PARAMETER (Pixel2MM_Y, double, 0.0)
				DECLARE_INPUT_PARAMETER(GenerateOverlay, bool, false)
				
				const Core::FrameRef * _frame = nullptr;
		};

		using ABSTRACT_INPUT_PTR = std::shared_ptr<ABSTRACT_INPUT>;

		//------------------------------------------
		//		Frame I2S detection Input  class
		//------------------------------------------

		class PARAMS_I2S_INPUT :   public ABSTRACT_INPUT
		{
			public:
				explicit PARAMS_I2S_INPUT(const Core::FrameRef * frame, const SHEET_SIDE side)
					: ABSTRACT_INPUT(frame)
					, _side(side)
					, _triangleImageSource (std::make_shared<cv::Mat>())
					{}
			
				explicit PARAMS_I2S_INPUT (const PARAMS_I2S_INPUT& other) = delete;
				explicit PARAMS_I2S_INPUT (PARAMS_I2S_INPUT&& other) = delete;
				const PARAMS_I2S_INPUT & operator = (const PARAMS_I2S_INPUT& other) = delete;
				PARAMS_I2S_INPUT & operator = (PARAMS_I2S_INPUT &&) = delete;
				virtual ~PARAMS_I2S_INPUT () = default;

				std::string getElementName() const  override
				{
					return fmt::format("i2s_{0}_[{1},{2}]", SIDE_NAMES[_side], _approxTriangeROI.left(), _approxTriangeROI.top());
				}

				SHEET_SIDE					_side = LEFT;
				ROIRect						_approxTriangeROI {};
				std::shared_ptr<cv::Mat>	_triangleImageSource;
				HSV	_triangleColor {};
		};
		using PARAMS_I2S_INPUT_PTR = std::shared_ptr<PARAMS_I2S_INPUT>;

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
					, _ROIImageSource (std::make_shared<cv::Mat>())
					, _roiIndex(roiIndex)
				{
				}

				explicit PARAMS_C2C_ROI_INPUT (const PARAMS_C2C_ROI_INPUT& other) = delete;
				explicit PARAMS_C2C_ROI_INPUT (PARAMS_C2C_ROI_INPUT&& other) = delete;
				const PARAMS_C2C_ROI_INPUT & operator = (const PARAMS_C2C_ROI_INPUT& other) = delete;
				PARAMS_C2C_ROI_INPUT & operator = (PARAMS_C2C_ROI_INPUT &&) = delete;
				virtual ~PARAMS_C2C_ROI_INPUT () = default;

				SHEET_SIDE						_side = LEFT;
				std::vector<HSV>				_colors;
				ROIRect							_ROI;
				std::shared_ptr<cv::Mat>		_ROIImageSource;
				uint32_t						_roiIndex = -1;

				std::string getElementName() const  override
				{
					return fmt::format("c2c_{0}_{1}_[{2},{3}]", SIDE_NAMES[_side], _roiIndex, _ROI.left(), _ROI.top());
				}
		};

		using PARAMS_C2C_ROI_INPUT_PTR = std::shared_ptr<PARAMS_C2C_ROI_INPUT>;

		//------------------------------------------
		//		Edge detection Input parameters
		//------------------------------------------

		class PARAMS_PAPEREDGE_INPUT : public ABSTRACT_INPUT
		{
			public:
				PARAMS_PAPEREDGE_INPUT(const Core::FrameRef * frame, const SHEET_SIDE side)
					: ABSTRACT_INPUT(frame )
					, _side(side)
					, _stripImageSource (std::make_shared<cv::Mat>())
				{
				}

				explicit PARAMS_PAPEREDGE_INPUT (const PARAMS_PAPEREDGE_INPUT& other) = delete;
				explicit PARAMS_PAPEREDGE_INPUT (PARAMS_PAPEREDGE_INPUT&& other) = delete;
				const PARAMS_PAPEREDGE_INPUT & operator = (const PARAMS_PAPEREDGE_INPUT& other) = delete;
				PARAMS_PAPEREDGE_INPUT & operator = (PARAMS_PAPEREDGE_INPUT &&) = delete;
				virtual ~PARAMS_PAPEREDGE_INPUT () = default;

				std::string getElementName() const  override
				{
					return fmt::format("edge_{0}", SIDE_NAMES[_side]);
				}

				SHEET_SIDE						_side = LEFT;
				uint32_t						_approxDistanceFromEdgeX = -1;
				uint32_t						_triangeApproximateY = -1;
				std::shared_ptr<cv::Mat>		_stripImageSource;
		};
		using PARAMS_PAPEREDGE_INPUT_PTR = std::shared_ptr<PARAMS_PAPEREDGE_INPUT>;

		//------------------------------------------
		//		WAVE detection Input parameters
		//------------------------------------------

		// Input
		class PARAMS_WAVE_INPUT :  public ABSTRACT_INPUT
		{
			public:
				explicit PARAMS_WAVE_INPUT(const Core::FrameRef * frame)
					: ABSTRACT_INPUT(frame)
					, _waveImageSource (std::make_shared<cv::Mat>())
				{}
				explicit PARAMS_WAVE_INPUT (const PARAMS_WAVE_INPUT& other) = delete;
				explicit PARAMS_WAVE_INPUT (PARAMS_WAVE_INPUT&& other) = delete;
				const PARAMS_WAVE_INPUT & operator = (const PARAMS_WAVE_INPUT& other) = delete;
				PARAMS_WAVE_INPUT & operator = (PARAMS_WAVE_INPUT &&) = delete;
				virtual ~PARAMS_WAVE_INPUT () = default;

				std::string getElementName() const  override
				{
					return "wave";
				}

			std::shared_ptr<cv::Mat>	_waveImageSource;
			// scanner side
			HSV							_circleColor;
			ROIRect						_waveROI;
			int32_t						_circlesCount = 0;
		};
		using PARAMS_WAVE_INPUT_PTR = std::shared_ptr<PARAMS_WAVE_INPUT>;

		//------------------------------------------
		//		Frame Strip detection Input parameters
		//------------------------------------------
		class PARAMS_C2C_STRIP_INPUT :  public ABSTRACT_INPUT
		{
			public:
				explicit PARAMS_C2C_STRIP_INPUT(const Core::FrameRef * frame, const SHEET_SIDE side)
					: ABSTRACT_INPUT(frame)
					, _side(side)
					, _paperEdgeInput(std::make_shared<PARAMS_PAPEREDGE_INPUT>(frame, side))
					, _i2sInput(std::make_shared<PARAMS_I2S_INPUT>(frame, side))
					{
					}
				
				explicit PARAMS_C2C_STRIP_INPUT (const PARAMS_C2C_STRIP_INPUT& other) = delete;
				explicit PARAMS_C2C_STRIP_INPUT (PARAMS_C2C_STRIP_INPUT&& other) = delete;
				const PARAMS_C2C_STRIP_INPUT & operator = (const PARAMS_C2C_STRIP_INPUT& other) = delete;
				PARAMS_C2C_STRIP_INPUT & operator = (PARAMS_C2C_STRIP_INPUT &&) = delete;
				virtual ~PARAMS_C2C_STRIP_INPUT () = default;

			std::string getElementName() const  override
			{
				return fmt::format("strip_{0}", SIDE_NAMES[_side]);
			}

			SHEET_SIDE								_side = LEFT;
			PARAMS_PAPEREDGE_INPUT_PTR				_paperEdgeInput;
			PARAMS_I2S_INPUT_PTR					_i2sInput;
			std::vector<PARAMS_C2C_ROI_INPUT_PTR>	_c2cROIInputs;
		};
		using PARAMS_C2C_STRIP_INPUT_PTR = std::shared_ptr<PARAMS_C2C_STRIP_INPUT>;

		//------------------------------------------
		//	Sheet detection Input parameters
		//------------------------------------------
		class PARAMS_C2C_SHEET_INPUT :  public ABSTRACT_INPUT
		{
			public:
				explicit PARAMS_C2C_SHEET_INPUT(const Core::FrameRef * frame)
					: ABSTRACT_INPUT(frame)
					, _stripInputParamLeft(std::make_shared<PARAMS_C2C_STRIP_INPUT>(frame, LEFT))
					, _stripInputParamRight(std::make_shared<PARAMS_C2C_STRIP_INPUT>(frame, RIGHT))
					, _waveTriangleInput(std::make_shared<PARAMS_I2S_INPUT>(frame, WAVE))
					, _waveInputs(0, std::make_shared<PARAMS_WAVE_INPUT>(frame))
					{}

				explicit PARAMS_C2C_SHEET_INPUT (const PARAMS_C2C_SHEET_INPUT& other) = delete;
				explicit PARAMS_C2C_SHEET_INPUT (PARAMS_C2C_SHEET_INPUT&& other) = delete;
				const PARAMS_C2C_SHEET_INPUT & operator = (const PARAMS_C2C_SHEET_INPUT& other) = delete;
				PARAMS_C2C_SHEET_INPUT & operator = (PARAMS_C2C_SHEET_INPUT &&) = delete;
				virtual ~PARAMS_C2C_SHEET_INPUT () = default;

				std::string getElementName() const  override
				{
					return "frame";
				}

				PARAMS_C2C_STRIP_INPUT_PTR							_stripInputParamLeft;
				PARAMS_C2C_STRIP_INPUT_PTR							_stripInputParamRight;
				PARAMS_I2S_INPUT_PTR								_waveTriangleInput;
				std::vector<std::shared_ptr<PARAMS_WAVE_INPUT>>		_waveInputs;
		};
		using PARAMS_C2C_SHEET_INPUT_PTR = std::shared_ptr<PARAMS_C2C_SHEET_INPUT>;


		//////////////////////////////////////////////////////////////////////////
		///////////////////////////    OUTPUT CLASSES  ///////////////////////////
		//////////////////////////////////////////////////////////////////////////

		//------------------------------------------
		//		Abstract Output  class
		//------------------------------------------
		class ABSTRACT_OUTPUT
		{
			public:
				
				explicit ABSTRACT_OUTPUT(std::shared_ptr<ABSTRACT_INPUT> input) {}
				explicit ABSTRACT_OUTPUT (const ABSTRACT_OUTPUT& other) = delete;
				explicit ABSTRACT_OUTPUT (ABSTRACT_OUTPUT&& other) = delete;
				const ABSTRACT_OUTPUT & operator = (const ABSTRACT_OUTPUT& other) = delete;
				ABSTRACT_OUTPUT & operator = (ABSTRACT_OUTPUT &&) = delete;
				virtual ~ABSTRACT_OUTPUT() = default;

				virtual std::string getElementName() = 0;
				virtual std::shared_ptr<cv::Mat> overlay() const  = 0;

				OUT_STATUS	_result = ALG_STATUS_FAILED;
		};
		using ABSTRACT_OUTPUT_PTR = std::shared_ptr<ABSTRACT_OUTPUT>;

		//------------------------------------------
		//		Frame I2S detection Output  class
		//------------------------------------------
		class PARAMS_I2S_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:
				
				explicit PARAMS_I2S_OUTPUT(std::shared_ptr<PARAMS_I2S_INPUT> input)
					: ABSTRACT_OUTPUT(input)
					, _triangleOverlay (std::make_shared<cv::Mat>())
					, _input (input)
				{}
				explicit PARAMS_I2S_OUTPUT (const PARAMS_I2S_OUTPUT& other) = delete;
				explicit PARAMS_I2S_OUTPUT (PARAMS_I2S_OUTPUT&& other) = delete;
				const PARAMS_I2S_OUTPUT & operator = (const PARAMS_I2S_OUTPUT& other) = delete;
				PARAMS_I2S_OUTPUT & operator = (PARAMS_I2S_OUTPUT &&) = delete;
				virtual ~PARAMS_I2S_OUTPUT() = default;

				APOINT							_triangeCorner {};
				std::shared_ptr<cv::Mat>		_triangleOverlay;

				std::shared_ptr<PARAMS_I2S_INPUT>	_input;

				std::string getElementName() override
				{
					return fmt::format("i2s_{0}_[{1},{2}]_overlay", SIDE_NAMES[_input->_side], _input->_approxTriangeROI.left(), _input->_approxTriangeROI.top());
				}
			
				std::shared_ptr<cv::Mat> overlay()  const override
				{
					return _triangleOverlay;
				}
		};
		using PARAMS_I2S_OUTPUT_PTR = std::shared_ptr<PARAMS_I2S_OUTPUT>;
		
		//------------------------------------------
		//		Frame EDGE detection Output  class
		//------------------------------------------
		class PARAMS_PAPEREDGE_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:
				explicit PARAMS_PAPEREDGE_OUTPUT(std::shared_ptr<PARAMS_PAPEREDGE_INPUT> input)
					: ABSTRACT_OUTPUT(input)
					, _edgeOverlay (std::make_shared<cv::Mat>())
					, _input(input)
				{}
				explicit PARAMS_PAPEREDGE_OUTPUT (const PARAMS_PAPEREDGE_OUTPUT& other) = delete;
				explicit PARAMS_PAPEREDGE_OUTPUT (PARAMS_PAPEREDGE_OUTPUT&& other) = delete;
				const PARAMS_PAPEREDGE_OUTPUT & operator = (const PARAMS_PAPEREDGE_OUTPUT& other) = delete;
				PARAMS_PAPEREDGE_OUTPUT & operator = (PARAMS_PAPEREDGE_OUTPUT &&) = delete;
				virtual ~PARAMS_PAPEREDGE_OUTPUT() = default;
				
				uint32_t								_exactDistanceFromEdgeX = -1;
				std::shared_ptr<cv::Mat>				_edgeOverlay;
				std::shared_ptr<PARAMS_PAPEREDGE_INPUT>	_input;
			
				std::string getElementName() override
				{
					return fmt::format("edge_{0}_overlay", SIDE_NAMES[_input->_side]);
				}
				
				std::shared_ptr<cv::Mat> overlay() const override
				{
					return _edgeOverlay;
				}
		};
		using PARAMS_PAPEREDGE_OUTPUT_PTR = std::shared_ptr<PARAMS_PAPEREDGE_OUTPUT>;

		//------------------------------------------
		//		C2C detection Output  class
		//------------------------------------------
		class PARAMS_C2C_ROI_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:
				explicit PARAMS_C2C_ROI_OUTPUT(std::shared_ptr<PARAMS_C2C_ROI_INPUT> input) 
					: ABSTRACT_OUTPUT(input)
					, _colorOverlay (std::make_shared<cv::Mat>())
					, _input(input)
				{}
				explicit PARAMS_C2C_ROI_OUTPUT (const PARAMS_C2C_ROI_OUTPUT& other) = delete;
				explicit PARAMS_C2C_ROI_OUTPUT (PARAMS_C2C_ROI_OUTPUT&& other) = delete;
				const PARAMS_C2C_ROI_OUTPUT & operator = (const PARAMS_C2C_ROI_OUTPUT& other) = delete;
				PARAMS_C2C_ROI_OUTPUT & operator = (PARAMS_C2C_ROI_OUTPUT &&) = delete;
				virtual ~PARAMS_C2C_ROI_OUTPUT() = default;

				std::vector<OUT_STATUS>		_colorStatuses;
				std::vector<APOINT>			_colorCenters;
				std::shared_ptr<cv::Mat>	_colorOverlay;
				PARAMS_C2C_ROI_INPUT_PTR	_input;

			///
				std::string getElementName() override
				{
					const auto castInput = std::static_pointer_cast<PARAMS_C2C_ROI_INPUT>(_input);
					return fmt::format("c2c_{0}_{1}_[{2},{3}]_overlay", SIDE_NAMES[castInput->_side], castInput->_roiIndex, castInput->_ROI.left(), castInput->_ROI.top());
				}

				std::shared_ptr<cv::Mat> overlay() const  override
				{
					return _colorOverlay;
				}
		};
		using PARAMS_C2C_ROI_OUTPUT_PTR = std::shared_ptr<PARAMS_C2C_ROI_OUTPUT>;

		//------------------------------------------
		//		WAVE detection Output  class
		//------------------------------------------
		class PARAMS_WAVE_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:
				explicit PARAMS_WAVE_OUTPUT(PARAMS_WAVE_INPUT_PTR input) 
					: ABSTRACT_OUTPUT(input)
					, _colorOverlay (std::make_shared<cv::Mat>())
					, _input(input)
				{}
				explicit PARAMS_WAVE_OUTPUT (const PARAMS_WAVE_OUTPUT& other) = delete;
				explicit PARAMS_WAVE_OUTPUT (PARAMS_WAVE_OUTPUT&& other) = delete;
				const PARAMS_WAVE_OUTPUT & operator = (const PARAMS_WAVE_OUTPUT& other) = delete;
				PARAMS_WAVE_OUTPUT & operator = (PARAMS_WAVE_OUTPUT &&) = delete;
				virtual ~PARAMS_WAVE_OUTPUT() = default;

				std::vector<OUT_STATUS>					_colorDetectionResults;
				std::vector<APOINT>						_colorCenters;
				std::shared_ptr<cv::Mat>				_colorOverlay;
				PARAMS_WAVE_INPUT_PTR					_input;
				
				std::string getElementName() override
				{
					return fmt::format("wave_[{0}]", std::static_pointer_cast<PARAMS_WAVE_INPUT>(_input)->_circleColor._colorName);
				}

				std::shared_ptr<cv::Mat> overlay()  const override
				{
					return _colorOverlay;
				}

		};
		using PARAMS_WAVE_OUTPUT_PTR = std::shared_ptr<PARAMS_WAVE_OUTPUT>;

		//------------------------------------------
		//		Strip detection Output  class
		//------------------------------------------
		class PARAMS_C2C_STRIP_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:
				explicit PARAMS_C2C_STRIP_OUTPUT(PARAMS_C2C_STRIP_INPUT_PTR input) 
					: ABSTRACT_OUTPUT(input)
					, _paperEdgeOutput(std::make_shared<PARAMS_PAPEREDGE_OUTPUT>(input->_paperEdgeInput))
					, _i2sOutput(std::make_shared<PARAMS_I2S_OUTPUT>(input->_i2sInput))
					, _input(input)
				{}

				explicit PARAMS_C2C_STRIP_OUTPUT (const PARAMS_C2C_STRIP_OUTPUT& other) = delete;
				explicit PARAMS_C2C_STRIP_OUTPUT (PARAMS_C2C_STRIP_OUTPUT&& other) = delete;
				const PARAMS_C2C_STRIP_OUTPUT & operator = (const PARAMS_C2C_STRIP_OUTPUT& other) = delete;
				PARAMS_C2C_STRIP_OUTPUT & operator = (PARAMS_C2C_STRIP_OUTPUT &&) = delete;
				virtual ~PARAMS_C2C_STRIP_OUTPUT() = default;

				PARAMS_PAPEREDGE_OUTPUT_PTR											_paperEdgeOutput;
				PARAMS_I2S_OUTPUT_PTR												_i2sOutput;
				Concurrency::concurrent_vector<PARAMS_C2C_ROI_OUTPUT_PTR>			_c2cROIOutputs;
				PARAMS_C2C_STRIP_INPUT_PTR											_input;
				
								
				std::string getElementName() override
				{
					return fmt::format("strip_{0}_overlay", SIDE_NAMES[std::static_pointer_cast<PARAMS_C2C_STRIP_INPUT>(_input)->_side]);
				}
				
				std::shared_ptr<cv::Mat> overlay()  const override
				{
					return nullptr;
				}
		};
		using PARAMS_C2C_STRIP_OUTPUT_PTR = std::shared_ptr<PARAMS_C2C_STRIP_OUTPUT>;


		//------------------------------------------
		//		Sheet detection Output  class
		//------------------------------------------
		class PARAMS_C2C_SHEET_OUTPUT : public ABSTRACT_OUTPUT
		{
			public:
			explicit PARAMS_C2C_SHEET_OUTPUT(PARAMS_C2C_SHEET_INPUT_PTR input) 
					: ABSTRACT_OUTPUT(input)
					, _stripOutputParameterLeft(std::make_shared<PARAMS_C2C_STRIP_OUTPUT>(input->_stripInputParamLeft))
					, _stripOutputParameterRight(std::make_shared<PARAMS_C2C_STRIP_OUTPUT>(input->_stripInputParamRight))
					, _input(input)
				{}

				explicit PARAMS_C2C_SHEET_OUTPUT (const PARAMS_C2C_SHEET_OUTPUT& other) = delete;
				explicit PARAMS_C2C_SHEET_OUTPUT (PARAMS_C2C_SHEET_OUTPUT&& other) = delete;
				const PARAMS_C2C_SHEET_OUTPUT & operator = (const PARAMS_C2C_SHEET_OUTPUT& other) = delete;
				PARAMS_C2C_SHEET_OUTPUT & operator = (PARAMS_C2C_SHEET_OUTPUT &&) = delete;
				virtual ~PARAMS_C2C_SHEET_OUTPUT() = default;

				PARAMS_C2C_STRIP_OUTPUT_PTR										_stripOutputParameterLeft;
				PARAMS_C2C_STRIP_OUTPUT_PTR										_stripOutputParameterRight;
				PARAMS_I2S_OUTPUT_PTR											_waveTriangleOutput;
				Concurrency::concurrent_vector<PARAMS_WAVE_OUTPUT_PTR>			_waveOutputs;
				PARAMS_C2C_SHEET_INPUT_PTR										_input;
				///

				std::string getElementName() override
				{
					return "frame_overlay";
				}

				std::shared_ptr<cv::Mat> overlay()  const override
				{
					return nullptr;
				}
		};
		using PARAMS_C2C_SHEET_OUTPUT_PTR = std::shared_ptr<PARAMS_C2C_SHEET_OUTPUT>;
	}
}
