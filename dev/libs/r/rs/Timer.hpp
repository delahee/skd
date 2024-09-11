#pragma once

#include "platform.h"
#include "1-Time/Time.h"

namespace rs {
	class Timer {

	public:
		static int				lastFrameCount;
		static Pasta::u64		lastMarker;

		static double			now;
		static double			dt;
		static double			maxDt;
		static int				df;
		static double			dfr;
		static uint64_t			frameCount;
		
	public:

		static time_t			dateNow();
		static inline double	stamp() { return getTimeStamp(); };
		static double			getTimeStamp();
		static void				exitFrame();
		static void				enterFrame();
		static int				ellapsedFrame() { return lastFrameCount - frameCount; }

		static void				delay(double ms, std::function<void(void)> fun);
		static void				delay(std::function<void(void)> fun);

		static void				gpuMeasureBegin();
		static double			gpuMeasureEnd();//returns in seconds
		static Pasta::u64		gpuMeasureEndNano();

	private:
		static Pasta::u32		lastGpuId;
		
	};
}