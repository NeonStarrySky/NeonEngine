#include "core/logger.h"
#include "frameRateController.h"

#include <iostream>
#include <stdexcept>
#include <thread>

namespace neon::graphics {

	FrameRateController::FrameRateController(double frame_rate)
	{
		setFrameRate(frame_rate);
	}

	void FrameRateController::checkAndWait()
	{
		timer.tick();
		while (frame_time - timer.totalTime() > 0) {
			std::this_thread::yield();
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
