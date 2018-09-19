#include "stdafx.h"
#include "BackgroundThreadPool.h"


/**
* \brief imageGenerationThread the only thread which deals with image acquisition ( emulation/generation/capture ), e.g. Frame Producer
* \return
*/
LandaJune::Threading::BackgroundThread& LandaJune::Threading::frameProducerThread()
{
	static BackgroundThread __th("Frame Producer Thread", 100);
	return __th;
}

/**
* \brief imageProcessingThread the only thread which deals with image processing, including in-memory region copying and algorithmic image processing, e.g. Frame Consumer
* \return
*/
LandaJune::Threading::BackgroundThread& LandaJune::Threading::frameConsumerThread()
{
	static BackgroundThread __th("Frame Consumer Thread", 200);
	return __th;
}