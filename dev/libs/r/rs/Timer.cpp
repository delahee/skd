#include "stdafx.h"
#include "Timer.hpp"
#include "Sys.hpp"

#include <math.h>
#include "1-time/Time.h"
#include <chrono>

using namespace std;
using namespace rs;
using namespace Pasta;

struct TimerDelay {
	double curMs				= 0.0;
	double durationMs			= 0.0;
	std::function<void(void)>	fun;
};

static eastl::vector<TimerDelay> delays;
static eastl::vector<TimerDelay> tmp;

double					Timer::dt = 0.0000001;
double					Timer::maxDt = -1;
int						Timer::df = 0;
double					Timer::dfr = 0.00001;
double					Timer::now = 0.0;
uint64_t				Timer::frameCount = 0;
int						Timer::lastFrameCount = 0;
Pasta::u64				Timer::lastMarker = 0;

void Timer::enterFrame() {
	if(lastMarker == 0 ) lastMarker = Pasta::Time::getTimeMarker();

	auto nuMarker = Pasta::Time::getTimeMarker();
	auto dmarker = nuMarker - lastMarker;

	dt = Pasta::Time::getTimeNS(dmarker) / 1000000000.0;
	if (dt <= 0.0) dt = 0.0000001;
	if(maxDt>0)
		if (dt >= maxDt) dt = maxDt;
	df = round( dt * rs::Sys::FPS );
	dfr = dt * rs::Sys::FPS;
	now = Pasta::Time::getTimeNS(nuMarker) / 1000000000.0;
	lastFrameCount = frameCount;
	frameCount++;

	lastMarker = nuMarker;
}

void rs::Timer::delay(double ms, std::function<void(void)> fun){
	if (!fun) return;

	TimerDelay td;
	td.curMs = ms;
	td.durationMs = ms;
	td.fun = fun;
	delays.push_back(td);
}

void rs::Timer::delay(std::function<void(void)> fun) {
	if (!fun) return;

	TimerDelay td;
	td.curMs = 0;
	td.durationMs = 0;
	td.fun = fun;
	delays.push_back(td);
}

Pasta::u32 rs::Timer::lastGpuId = 0;

Pasta::u64 rs::Timer::gpuMeasureEndNano(){
	Pasta::GraphicContext::endQuery(Pasta::QueryType::TimeElapsed);
	return Pasta::GraphicContext::getQueryResult(lastGpuId);
}

void rs::Timer::gpuMeasureBegin(){
	lastGpuId = 0;
	Pasta::GraphicContext::createQuery(&lastGpuId);
	Pasta::GraphicContext::beginQuery(Pasta::QueryType::TimeElapsed, lastGpuId);
}

double rs::Timer::gpuMeasureEnd(){
	Pasta::GraphicContext::endQuery(Pasta::QueryType::TimeElapsed);
	return Pasta::GraphicContext::getQueryResult(lastGpuId) * 1e-9;
}

void Timer::exitFrame() {
	//TimerDelay & td : delays

	if (delays.size()) {
		int size = delays.size();

		//in case we have deletions and insertion, be super careful no iterators or offsetting happen at this stage
		for ( int i = 0 ; i<size; ++i) {
			TimerDelay& td = delays[i];
			td.curMs -= dt * 1000.0;
			if (td.curMs <= 0) {
				if (td.fun) 
					td.fun();
			}
		}

		for (int i = 0; i < delays.size(); ) {//just erase offenders
			TimerDelay& td = delays[i];
			if (td.curMs < 0)//this must be <0 in case function with 0 del are inserted during prev loop
				delays.erase(delays.begin() + i);
			else
				i++;
		}
	}

}


time_t rs::Timer::dateNow()
{
	return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

double Timer::getTimeStamp() {
	return Pasta::Time::getTimeNS(Time::getTimeMarker()) / 1000000000.0;
}
