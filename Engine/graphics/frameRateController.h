#pragma once

#include "timer.h"

namespace neon::graphics {
	class FrameRateController {
		Timer timer;
		double frame_rate = 0;
		double frame_time = 0;
		double frame_rate_actual = 0;
	public:
		FrameRateController() : frame_rate(60.0), frame_time(1.0 / 60.0) {};
		FrameRateController(double frame_rate);
		void checkAndWait();
		void setFrameRate(double frame_rate = 60.0);
		void setDeltaTime(double delta_time = 1.0 / 60.0);
		double getActualFrameRate() const { return frame_rate_actual; }
	};
}