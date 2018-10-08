#pragma once
#include <mutex>
#include "global.h"

namespace LandaJune 
{
	namespace Helpers
	{
		class STATLIB_EXPORT RealTimeStats
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

			void reset();
			void increment(const StatName stat, const double delta, const double v = 1);
			std::string to_string(bool bBreakLines = false);

			int count(void) const { return statsNumber; }
			std::string name(StatName id) const;
			StatInfo info(StatName id) const;

			static RealTimeStats* rtStats();

		protected:
			std::mutex _mutex;
			double _current[statsNumber]{};
			double _values[statsNumber]{};
			double _times[statsNumber]{};

			static RealTimeStats* _this;
		};
	}
}

