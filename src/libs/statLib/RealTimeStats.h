#pragma once
#include <mutex>
#include "global.h"

namespace LandaJune 
{
	namespace Helpers
	{
		class RealTimeStats
		{
		public:
			
			RealTimeStats();
			virtual ~RealTimeStats();
			

			enum StatName {
				objectsPerSec_acquiredFramesOk = 0,
				objectsPerSec_framesHandledOk,
				objectsPerSec_regionsGeneratedOk,
				objectsPerSec_regionsCopiedOk,
				objectsPerSec_stripsHandledOk,
				objectsPerSec_edgeHandledOk,
				objectsPerSec_I2SHandledOk,
				objectsPerSec_C2CHandledOk,
				objectsPerSec_WaveHandledOk,
				objectsPerSec_savedBitmapsOk,
				objectsPerSec_acquiredFramesFail,
				objectsPerSec_generatedImagesFail,
				objectsPerSec_performedAlgoFail,
				objectsPerSec_performedAlgoResultFail,
				objectsPerSec_createdRegionsFail,
				objectsPerSec_savedBitmapsFail,
				objects_saveQueueLength,
				statsNumber

			};

			struct StatInfo {
				double _total;
				double _current;
				double _average;
				double _raverage;
			};

			STATLIB_EXPORT void reset();
			STATLIB_EXPORT void increment(const StatName stat, const double delta, const double v = 1);
			STATLIB_EXPORT std::string to_string(bool bBreakLines = false);

			STATLIB_EXPORT int count(void) const { return statsNumber; }
			STATLIB_EXPORT std::string name(StatName id) const;
			STATLIB_EXPORT StatInfo info(StatName id) const;

			STATLIB_EXPORT static RealTimeStats* rtStats();

		protected:
			std::mutex _mutex;
			double _current[statsNumber]{};
			double _values[statsNumber]{};
			double _times[statsNumber]{};

			static RealTimeStats* _this;
		};
	}
}

