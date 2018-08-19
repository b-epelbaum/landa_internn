#pragma once
#include <mutex>
#include "jutils_global.h"

namespace LandaJune 
{
	namespace Helpers
	{
		class JUTILS_EXPORT RealTimeStats
		{
		public:
			RealTimeStats();
			virtual ~RealTimeStats();
			

			enum StatName {
				objectsPerSec_acquiredFramesOk = 0,
				objectsPerSec_performedAlgoOk,
				objectsPerSec_performedAlgoResultOk,
				objectsPerSec_createdRegionsOk,
				objectsPerSec_savedBitmapsOk,
				objectsPerSec_acquiredFramesFail,
				objectsPerSec_generatedImagesFail,
				objectsPerSec_performedAlgoFail,
				objectsPerSec_performedAlgoResultFail,
				objectsPerSec_createdRegionsFail,
				objectsPerSec_savedBitmapsFail,
				statsNumber

			};

			void reset();
			void increment(const StatName stat, const double delta, const double v = 1);
			std::string to_string();

			static std::shared_ptr<RealTimeStats> rtStats()
			{
				return _this ? _this : _this = std::make_shared<RealTimeStats>();
			}

		protected:
			std::mutex _mutex;
			double _values[statsNumber]{};
			double _times[statsNumber]{};

			static std::shared_ptr<RealTimeStats> _this;
		};
	}
}

