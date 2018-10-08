#pragma once

namespace LandaJune
{
	enum class FrameProviderDataCallbackType
	{
		  CALLBACK_FRAME_DATA
		, CALLBACK_SCANNED_FILES_COUNT
	};

	enum class FrameConsumerDataCallbackType
	{
		CALLBACK_FRAME_HANDLED
		,CALLBACK_FRAME_FAILED
	};
}

