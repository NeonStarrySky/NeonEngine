#pragma once

#include <chrono>
#include <iostream>

namespace neon {
	class Timer {
	public:
		void tick() {
			auto now = clock::now();
			delta = duration(now - last);
			last = now;
			total += delta;
		}

		double deltaTime() const { return delta; }
		double totalTime() const { return total; }

		void reset() {
			last = clock::now();
			delta = 0;
			total = 0.0f;
		}

	private:
		using clock = std::chrono::steady_clock;

		// 将时钟持续时间转换为秒
		double duration(clock::duration d) {
			return std::chrono::duration<double>(d).count();
		}

		clock::time_point last = clock::now();
		double delta = 0;
		double total = 0;
	};
}