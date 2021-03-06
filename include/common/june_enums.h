#pragma once

namespace LandaJune
{
	enum class CoreCallbackType
	{
		  CALLBACK_PROVIDER_SCANNED_FILES_COUNT
		, CALLBACK_PROVIDER_FRAME_GENERATED_OK
		, CALLBACK_PROVIDER_FRAME_SKIPPED
		, CALLBACK_PROVIDER_FINISHED
		, CALLBACK_PROVIDER_FRAME_IMAGE_DATA
		, CALLBACK_PROVIDER_EXCEPTION
		
		, CALLBACK_RUNNER_FRAME_HANDLED_OK
		, CALLBACK_RUNNER_FRAME_SKIPPED
		, CALLBACK_RUNNER_DETECTION_OK
		, CALLBACK_RUNNER_DETECTION_FAILED
		, CALLBACK_RUNNER_EXCEPTION

		, CALLBACK_IMAGE_SAVER_ERROR
	};
}

