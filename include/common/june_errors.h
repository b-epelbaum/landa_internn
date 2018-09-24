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

	class CORE_ERROR
	{
	public:

		CORE_ERROR () = default;
		~CORE_ERROR() = default;
		CORE_ERROR(const CORE_ERROR & other) 
			: _error (other._error), _errorString(other._errorString) 
		{}


		CORE_ERROR(CORE_ERROR && other) noexcept
			: _error (std::move(other._error)), _errorString(std::move(other._errorString))
		{}

		CORE_ERROR (const int error, const std::string strError = "" ) 
			: _error (error)
			, _errorString(std::move(strError))
		{
			if (_errorString.empty() )
			{
				auto const iter = _errorStringMap.map().find(error); 
				if ( iter != _errorStringMap.map().end())
					_errorString = iter->second;
			}
		}
	
		CORE_ERROR & operator = (const CORE_ERROR & other)
		{
			_error = other._error;
			_errorString = other._errorString;
			return *this;
		}

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

		template <typename T, typename U>
		class create_map
		{
			private:
				std::map<T, U> m_map;
			public:
				create_map(const T& key, const U& val)
			    {
			        m_map[key] = val;
				}	

			    create_map<T, U>& operator()(const T& key, const U& val)
			    {
			        m_map[key] = val;
			        return *this;
			    }

			    const std::map<T,U>& map() const
			    {
			        return m_map;
			    }
		};


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
		static const int ERR_OFFLINEREADER_SOURCE_FOLDER_INVALID				= 112;
		static const int ERR_OFFLINEREADER_SOURCE_FILE_INVALID					= 113;
		static const int ERR_OFFLINEREADER_NO_MORE_FILES						= 114;
		static const int ERR_SIMULATOR_HAVE_NO_IMAGES							= 115;
		static const int ERR_SIMULATOR_REACHED_MAX_COUNT						= 116;

		static const int ERR_CORE_NOT_INITIALIZED								= 200;
		static const int ERR_CORE_NO_PROVIDER_SELECTED							= 201;
		static const int ERR_CORE_NO_ALGORITHM_RUNNER_SELECTED					= 202;
		static const int ERR_CORE_PROVIDER_IS_BUSY								= 203;
		static const int ERR_CORE_PROVIDER_THROWN_RUNTIME_EXCEPTION				= 204;
		static const int ERR_CORE_PROVIDER_FAILED_TO_INIT						= 205;
		static const int ERR_CORE_ALGO_RUNNER_THROWN_RUNTIME_EXCEPTION			= 206;
		static const int ERR_CORE_CANNOT_CREATE_FOLDER							= 207;
		static const int ERR_CORE_CANNOT_ENCODE_TO_BMP							= 208;

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
		static const int ALGO_INIT_EDGE_FAILED									= 405;
		static const int ALGO_INIT_I2S_FAILED									= 406;
		static const int ALGO_INIT_C2C_FAILED									= 407;
		static const int ALGO_INIT_WAVE_FAILED									= 408;
		static const int ALGO_SHUTDOWN_EDGE_FAILED								= 409;
		static const int ALGO_SHUTDOWN_I2S_FAILED								= 410;
		static const int ALGO_SHUTDOWN_C2C_FAILED								= 411;
		static const int ALGO_SHUTDOWN_WAVE_FAILED								= 412;
		static const int ALGO_PROCESS_EDGE_FAILED								= 409;
		static const int ALGO_PROCESS_I2S_FAILED								= 410;
		static const int ALGO_PROCESS_C2C_FAILED								= 411;
		static const int ALGO_PROCESS_WAVE_FAILED								= 412;

		inline static auto _errorStringMap = create_map<int, std::string>
			(ERR_PROVIDER_INVALID_SELECTED_PROVIDER,				"Selected provider is invalid")
			(ERR_FRAMEGRABBER_LOAD_APPLET_FAILED,					"Failed loading Silicon Software applet")
			(ERR_FRAMEGRABBER_LOAD_CONFIG_FAILED,					"Failed loading Silicon Software configuraion file")
			(ERR_CORE_ALGO_RUNNER_THROWN_RUNTIME_EXCEPTION, 		"Runtime exception has been caught");
	};

	#define RESULT_OK CORE_ERROR::JUNE_NO_ERROR
	#define RESULT_NOT_IMPLEMENTED CORE_ERROR::JUNE_NO_ERROR

	inline std::ostream& operator << (std::ostream& os, const CORE_ERROR& err)
	{  
		os << "Error : " << err._error << " => [" << err._errorString.c_str() << "]";
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
