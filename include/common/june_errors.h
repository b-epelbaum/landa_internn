#pragma once
#include <iostream>  
#include <map>

namespace LandaJune
{
	/*
	template <typename E>
	constexpr auto to_underlying(E e) noexcept
	{
		return static_cast<std::underlying_type_t<E>>(e);
	}

	template <typename E>
	static int toInt(const E val)
	{
		return to_underlying(val);
	}
	*/

	#define ADD_ERR_DESC_AUTO(x) (*_errorStringMap)[x] = #x;
	#define ADD_ERR_DESC_TXT(x,y) (*_errorStringMap)[x] = y;

	class CORE_ERROR
	{
	public:

		CORE_ERROR () = default;
		~CORE_ERROR() = default;
		CORE_ERROR(const CORE_ERROR & other) = default;
		CORE_ERROR(CORE_ERROR && other) noexcept
			: _error (other._error), _errorString(std::move(other._errorString))
		{}

		CORE_ERROR (const int error, std::string strError = "" ) 
			: _error (error)
			, _errorString(std::move(strError))
		{
			if (error != 0 )
			{
				if (_errorString.empty())
				{
					if (!_errorStringMap )
						initStringMap();
					
					std::string defString;

					auto const iter = _errorStringMap->find(error); 
					if ( iter != _errorStringMap->cend())
						_errorString = iter->second;
				}
			}
		}
	
		CORE_ERROR & operator = (const CORE_ERROR & other)
		= default;

		CORE_ERROR & operator = (CORE_ERROR && other) noexcept
		{
			_error = other._error;
			_errorString = std::move(other._errorString);
			return *this;
		}

		friend std::ostream& operator << (std::ostream& os, const CORE_ERROR& err);
		
		operator int() const
		{
			return _error;
		}

		
		static const int JUNE_NO_ERROR = 0;
		static const int JUNE_CORE_ERROR = -1;

		int			_error = JUNE_NO_ERROR;
		std::string _errorString;

		using STRING_MAP = std::map<int, std::string>;

		struct STRING_MAPDeleterFunctor 
		{  
			void operator()(STRING_MAP* p) 
			{
				// TODO : think about nice design of static map destructor
			}
		};

		using STRING_MAP_PTR = std::unique_ptr<STRING_MAP,  STRING_MAPDeleterFunctor>;

		inline static STRING_MAP_PTR _errorStringMap;

		static void initStringMap()
		{
			_errorStringMap = STRING_MAP_PTR( new STRING_MAP );

			ADD_ERR_DESC_TXT(ERR_FUNC_INVALID_ALGO_PTR, "Algorithm runner parameter is invalid");
			ADD_ERR_DESC_TXT(ERR_FUNC_INVALID_CORE_OBJECT	, "Core object parameter is invalid");
			ADD_ERR_DESC_TXT(ERR_FUNC_INVALID_CORE_CALLBACK	, "Core callback parameter is invalid");
			ADD_ERR_DESC_TXT(ERR_FUNC_INVALID_PROVIDER_PTR	, "Provider object parameter is invalid");
			ADD_ERR_DESC_TXT(ERR_ALGORITHM_EXCEPTION		, "Algorithm has thrown exception");
			ADD_ERR_DESC_TXT(ERR_PROVIDER_EXCEPTION			, "Provider has thrown exception");
			ADD_ERR_DESC_TXT(ERR_FUNC_INVALID_PARAM_PTR		, "Parameters object parameter is invalid");
			ADD_ERR_DESC_TXT(ERR_PROVIDER_UNINITIALIZED		, "Frame provider has not been initialized");
			ADD_ERR_DESC_TXT(ERR_RUNNER_UNINITIALIZED		, "Algorithm runner has not been initialized");
			ADD_ERR_DESC_TXT(ERR_PROVIDER_FAILED_TO_INIT	, "Provider failed to ininitalize");
			ADD_ERR_DESC_TXT(ERR_RUNNER_FAILED_TO_INIT		, "Runner failed to initialize");
			ADD_ERR_DESC_TXT(ERR_PROVIDER_FUNC_INVALID_9	, "");
			ADD_ERR_DESC_TXT(ERR_DATA_SAVE_FAILURE			, "Saving data failed");

			ADD_ERR_DESC_TXT(ERR_PROVIDER_INVALID_SELECTED_PROVIDER, "Selected provider is invalid");
			ADD_ERR_DESC_TXT(ERR_FRAMEGRABBER_LOAD_APPLET_FAILED, "Failed loading Silicon Software applet");
			ADD_ERR_DESC_TXT(ERR_FRAMEGRABBER_LOAD_CONFIG_FAILED, "Failed loading Silicon Software configuration file");
			ADD_ERR_DESC_TXT(ERR_FRAMEGRABBER_CANNOT_GET_PROPERTY, "Cannot get named property");
			ADD_ERR_DESC_TXT(ERR_ALGORITHM_EXCEPTION, "Algorithm function has thrown exception");
			
			ADD_ERR_DESC_AUTO(ERR_FRAMEGRABBER_MEMORY_ALLOCATION_FAILED);
			
			ADD_ERR_DESC_AUTO(ERR_FRAMEGRABBER_IMAGE_ACQUISITION_FAILED); 	
			ADD_ERR_DESC_AUTO(ERR_FRAMEGRABBER_IMAGE_TIMEOUT	);			
			ADD_ERR_DESC_AUTO(ERR_FRAMEGRABBER_IMAGE_INVALID_DATA_POINTER);
			ADD_ERR_DESC_AUTO(ERR_FRAMEGRABBER_IMAGE_SKIPPED);			
			ADD_ERR_DESC_AUTO(ERR_FGSIMULATOR_NO_FILE_FOUND);				
			ADD_ERR_DESC_AUTO(ERR_OFFLINE_READER_SOURCE_FOLDER_INVALID);	
			ADD_ERR_DESC_AUTO(ERR_OFFLINE_READER_SOURCE_FILE_INVALID);		
			ADD_ERR_DESC_AUTO(ERR_OFFLINE_READER_NO_MORE_FILES);			
			ADD_ERR_DESC_AUTO(ERR_OFFLINE_READER_HAS_NO_IMAGES);				
			ADD_ERR_DESC_AUTO(ERR_OFFLINE_READER_REACHED_MAX_COUNT);			
			ADD_ERR_DESC_AUTO(ERR_PROVIDER_INVALID_PARAMETERS);			
			ADD_ERR_DESC_AUTO(ERR_PROVIDER_DLL_CANNOT_BE_LOADED);			
			ADD_ERR_DESC_AUTO(ERR_RUNNER_DLL_CANNOT_BE_LOADED);

			ADD_ERR_DESC_AUTO(ERR_CORE_NOT_INITIALIZED);						
			ADD_ERR_DESC_AUTO(ERR_CORE_NO_PROVIDER_SELECTED);					
			ADD_ERR_DESC_AUTO(ERR_CORE_NO_ALGORITHM_RUNNER_SELECTED);			
			ADD_ERR_DESC_AUTO(ERR_CORE_PROVIDER_IS_BUSY);						
			ADD_ERR_DESC_AUTO(ERR_CORE_PROVIDER_THROWN_RUNTIME_EXCEPTION);		
			ADD_ERR_DESC_AUTO(ERR_CORE_PROVIDER_FAILED_TO_INIT);				
			ADD_ERR_DESC_AUTO(ERR_CORE_CANNOT_CREATE_FOLDER);					
			ADD_ERR_DESC_AUTO(ERR_CORE_CANNOT_ENCODE_TO_BMP);					
			
			ADD_ERR_DESC_AUTO(ERR_FRAME_INVALID_INIT_DATA);					
			ADD_ERR_DESC_AUTO(ERR_FRAME_INVALID_IMAGE_FORMAT);				
			ADD_ERR_DESC_AUTO(ERR_FRAME_INVALID_FRAME_IMAGE_DIMS);				
			ADD_ERR_DESC_AUTO(ERR_FRAME_INVALID_BATCH_PARAMS);					
			ADD_ERR_DESC_AUTO(ERR_FRAME_NO_FRAME_REGIONS);						
			ADD_ERR_DESC_AUTO(ERR_FRAME_DIFFERENT_EXPECTED_SIZE);				
			ADD_ERR_DESC_AUTO(ERR_FRAME_INSUFFICIENT_BUFFER_SIZE);				
			
			ADD_ERR_DESC_AUTO(ALGO_ROI_RECT_IS_EMPTY);							
			ADD_ERR_DESC_AUTO(ALGO_ROI_INVALID_RECT); 							
			ADD_ERR_DESC_AUTO(ALGO_ROI_RECT_EXCEEDS_FRAME_RECT); 				
			ADD_ERR_DESC_AUTO(ALGO_INVALID_SOURCE_IMAGE);						
			ADD_ERR_DESC_AUTO(ALGO_EMPTY_ROI_NAME_TO_SAVE);					
			ADD_ERR_DESC_AUTO(ALGO_INIT_EDGE_FAILED);							
			ADD_ERR_DESC_AUTO(ALGO_INIT_I2S_FAILED);							
			ADD_ERR_DESC_AUTO(ALGO_INIT_C2C_FAILED);							
			ADD_ERR_DESC_AUTO(ALGO_INIT_WAVE_FAILED);							
			ADD_ERR_DESC_AUTO(ALGO_SHUTDOWN_EDGE_FAILED);						
			ADD_ERR_DESC_AUTO(ALGO_SHUTDOWN_I2S_FAILED);						
			ADD_ERR_DESC_AUTO(ALGO_SHUTDOWN_C2C_FAILED);						
			ADD_ERR_DESC_AUTO(ALGO_SHUTDOWN_WAVE_FAILED);						
			ADD_ERR_DESC_AUTO(ALGO_PROCESS_EDGE_FAILED);						
			ADD_ERR_DESC_AUTO(ALGO_PROCESS_I2S_FAILED);						
			ADD_ERR_DESC_AUTO(ALGO_PROCESS_C2C_FAILED);						
			ADD_ERR_DESC_AUTO(ALGO_PROCESS_WAVE_FAILED);

			ADD_ERR_DESC_AUTO(UI_ROITOOL_INVALID_IMAGE);
			ADD_ERR_DESC_AUTO(UI_ROITOOL_IMAGE_TOO_BIG);
		}

		

		////////////////////////////////////////////////////////
		///////////////////// ERRORS ///////////////////////////

		// errors from 20 to 50 are critical 
		static const int ERR_FUNC_INVALID_ALGO_PTR								= 20;
		static const int ERR_FUNC_INVALID_CORE_OBJECT							= 21;
		static const int ERR_FUNC_INVALID_CORE_CALLBACK							= 22;
		static const int ERR_FUNC_INVALID_PROVIDER_PTR							= 23;
		static const int ERR_ALGORITHM_EXCEPTION								= 25;
		static const int ERR_PROVIDER_EXCEPTION									= 26;
		static const int ERR_FUNC_INVALID_PARAM_PTR								= 27;
		static const int ERR_PROVIDER_UNINITIALIZED								= 28;
		static const int ERR_RUNNER_UNINITIALIZED								= 29;
		static const int ERR_PROVIDER_FAILED_TO_INIT							= 30;
		static const int ERR_RUNNER_FAILED_TO_INIT								= 31;
		static const int ERR_PROVIDER_FUNC_INVALID_9							= 32;
		static const int ERR_DATA_SAVE_FAILURE									= 33;

		

		static const int ERR_PROVIDER_INVALID_SELECTED_PROVIDER					= 102;
		static const int ERR_FRAMEGRABBER_LOAD_APPLET_FAILED					= 103;
		static const int ERR_FRAMEGRABBER_LOAD_CONFIG_FAILED					= 104;
		static const int ERR_FRAMEGRABBER_CANNOT_GET_PROPERTY					= 105;
		static const int ERR_FRAMEGRABBER_MEMORY_ALLOCATION_FAILED				= 106;
		static const int ERR_FRAMEGRABBER_IMAGE_ACQUISITION_FAILED				= 107;
		static const int ERR_FRAMEGRABBER_IMAGE_TIMEOUT							= 108;
		static const int ERR_FRAMEGRABBER_IMAGE_INVALID_DATA_POINTER			= 109;
		static const int ERR_FRAMEGRABBER_IMAGE_SKIPPED							= 110;
		static const int ERR_FGSIMULATOR_NO_FILE_FOUND							= 111;
		static const int ERR_OFFLINE_READER_SOURCE_FOLDER_INVALID				= 112;
		static const int ERR_OFFLINE_READER_SOURCE_FILE_INVALID					= 113;
		static const int ERR_OFFLINE_READER_NO_FILES_TO_LOAD						= 114;
		static const int ERR_OFFLINE_READER_NO_MORE_FILES						= 115;
		static const int ERR_OFFLINE_READER_HAS_NO_IMAGES							= 116;
		static const int ERR_OFFLINE_READER_REACHED_MAX_COUNT						= 117;
		static const int ERR_PROVIDER_INVALID_PARAMETERS						= 118;
		static const int ERR_PROVIDER_DLL_CANNOT_BE_LOADED						= 119;
		static const int ERR_RUNNER_DLL_CANNOT_BE_LOADED						= 120;
		static const int ERR_RUNNER_ANALYSIS_FAILED								= 121;
		static const int ERR_PROVIDER_CLEANUP_FAILED							= 122;

		static const int ERR_CORE_NOT_INITIALIZED								= 200;
		static const int ERR_CORE_NO_PROVIDER_SELECTED							= 201;
		static const int ERR_CORE_NO_ALGORITHM_RUNNER_SELECTED					= 202;
		static const int ERR_CORE_PROVIDER_IS_BUSY								= 230;
		static const int ERR_CORE_PROVIDER_THROWN_RUNTIME_EXCEPTION				= 231;
		static const int ERR_CORE_PROVIDER_FAILED_TO_INIT						= 232;
		static const int ERR_CORE_CANNOT_CREATE_FOLDER							= 251;
		static const int ERR_CORE_CANNOT_ENCODE_TO_BMP							= 270;

		static const int ERR_FRAME_INVALID_INIT_DATA							= 300;
		static const int ERR_FRAME_INVALID_IMAGE_FORMAT							= 301;
		static const int ERR_FRAME_INVALID_FRAME_IMAGE_DIMS						= 302;
		static const int ERR_FRAME_INVALID_BATCH_PARAMS							= 303;
		static const int ERR_FRAME_NO_FRAME_REGIONS								= 304;
		static const int ERR_FRAME_DIFFERENT_EXPECTED_SIZE						= 305;
		static const int ERR_FRAME_INSUFFICIENT_BUFFER_SIZE						= 306;
		
		static const int ALGO_ROI_RECT_IS_EMPTY									= 400;
		static const int ALGO_ROI_INVALID_RECT 									= 401;
		static const int ALGO_ROI_RECT_EXCEEDS_FRAME_RECT 						= 402;
		static const int ALGO_INVALID_SOURCE_IMAGE								= 403;
		static const int ALGO_EMPTY_ROI_NAME_TO_SAVE							= 404;
		static const int ALGO_C2C_IMAGE_TEMPLATE_INVALID						= 406;
		static const int ALGO_WAVE_IMAGE_TEMPLATE_INVALID						= 407;
		static const int ALGO_INVALID_PARAMETER_TYPE_PASSED						= 408;
		static const int ALGO_SETUP_SHEET_PARAMETERS_FAILED						= 409;
		static const int ALGO_SETUP_STRIP_PARAMETERS_FAILED						= 410;
		static const int ALGO_SETUP_EDGE_PARAMETERS_FAILED						= 411;
		static const int ALGO_SETUP_WAVE_PARAMETERS_FAILED						= 412;
		static const int ALGO_GENERATE_SHEET_REGIONS_FAILED						= 413;
		static const int ALGO_GENERATE_STRIP_REGIONS_FAILED						= 414;
		static const int ALGO_GENERATE_I2S_REGIONS_FAILED						= 415;
		static const int ALGO_GENERATE_C2C_REGIONS_FAILED						= 416;
		static const int ALGO_GENERATE_WAVE_REGIONS_FAILED						= 417;
		static const int ALGO_COPY_REGIONS_FAILED								= 418;
		static const int ALGO_PERFORM_COPY_FUNC_FAILED							= 419;
		static const int ALGO_AVT_SOURCE_PATH_INVALID							= 420;
		static const int ALGO_INIT_I2S_FAILED									= 430;
		static const int ALGO_INIT_EDGE_FAILED									= 431;
		static const int ALGO_INIT_C2C_FAILED									= 432;
		static const int ALGO_INIT_WAVE_FAILED									= 433;
		static const int ALGO_SHUTDOWN_EDGE_FAILED								= 440;
		static const int ALGO_SHUTDOWN_I2S_FAILED								= 441;
		static const int ALGO_SHUTDOWN_C2C_FAILED								= 442;
		static const int ALGO_SHUTDOWN_WAVE_FAILED								= 443;
		static const int ALGO_PROCESS_EDGE_FAILED								= 470;
		static const int ALGO_PROCESS_I2S_FAILED								= 471;
		static const int ALGO_PROCESS_C2C_FAILED								= 472;
		static const int ALGO_PROCESS_WAVE_FAILED								= 473;
		static const int ALGO_PROCESS_SHEET_FAILED								= 474;
		static const int ALGO_PROCESS_STRIP_FAILED								= 475;
		static const int ALGO_PROCESS_UNCAUGHT_EXCEPTION						= 476;
		static const int ALGO_PROCESS_I2S_OUTPUT_FAILED							= 477;
		static const int ALGO_PROCESS_C2C_OUTPUT_FAILED							= 478;
		static const int ALGO_PROCESS_WAVE_OUTPUT_FAILED						= 479;
		static const int ALGO_PROCESS_SHEET_OUTPUT_FAILED						= 480;
		static const int ALGO_PROCESS_STRIP_OUTPUT_FAILED						= 481;
		static const int ALGO_PROCESS_STRIP_OUTPUT_CSV_FAILED					= 482;
		static const int ALGO_PROCESS_WAVE_OUTPUT_CSV_FAILED					= 483;

		static const int UI_ROITOOL_INVALID_IMAGE								= 501;
		static const int UI_ROITOOL_IMAGE_TOO_BIG								= 502;

	};
	#define RESULT_OK CORE_ERROR::JUNE_NO_ERROR
	#define RESULT_NOT_IMPLEMENTED CORE_ERROR::JUNE_NO_ERROR

	inline std::ostream& operator << (std::ostream& os, const CORE_ERROR& err)
	{  
		os << "Error : " << err._error;
		return os;  
	} 
	
	

	enum OUT_STATUS
	{
		ALG_STATUS_SUCCESS,
		ALG_STATUS_FAILED,
		ALG_STATUS_EXCEPTION_THROWN,
		ALG_STATUS_CIRCLE_NOT_FOUND,
		ALG_STATUS_CORRUPT_CIRCLE,
		ALG_STATUS_TOO_MANY_CIRCLES,
		ALG_STATUS_NOT_ENOUGH_CIRCLES,
		ALG_STATUS_NUM
	};
}
