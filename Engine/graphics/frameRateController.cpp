#include "core/logger.h"
#include "frameRateController.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace neon::graphics {

#ifdef _WIN32
	namespace {
		class TimerResolution {
		public:
			TimerResolution()
			{
				enabled = timeBeginPeriod(1) == TIMERR_NOERROR;
			}

			~TimerResolution()
			{
				if (enabled) {
					timeEndPeriod(1);
				}
			}

		private:
			bool enabled = false;
		};

		const TimerResolution timer_resolution;
	}
#endif

	FrameRateController::FrameRateController(double frame_rate)
	{
		setFrameRate(frame_rate);
	}

	void FrameRateController::checkAndWait()
	{
		timer.tick();
		while (frame_time - timer.totalTime() > 0) {
			const double remaining_time = frame_time - timer.totalTime();
			if (remaining_time > 0.001) {
				std::this_thread::sleep_for(
					std::chrono::duration<double>(remaining_time - 0.001));
			} else {
				std::this_thread::yield();
			}
			timer.tick();
		}
		frame_rate_actual = 1.0 / timer.totalTime();

		timer.reset();
	}

	void FrameRateController::setFrameRate(double frame_rate)
	{
		if (frame_rate <= 0) {

			//spdlog::info("Frame rate must be positive. Given: {}", frame_rate);

			return;
		}

		this->frame_rate = frame_rate;
		this->frame_time = 1.0f / frame_rate;
	}

	void FrameRateController::setDeltaTime(double frame_time)
	{
		if (frame_time <= 0) {
			char msg[] = "void neon::FrameRateController::setDeltaTime(float frame_time):\n>>>invalid_argument!\n\n";

#ifdef _DEBUG
			std::cerr << msg;
#endif

			throw std::invalid_argument(msg);
		}

		this->frame_time = frame_time;
		this->frame_rate = 1.0f / frame_time;
	}
}
