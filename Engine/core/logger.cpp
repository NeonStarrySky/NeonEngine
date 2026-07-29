#include "logger.h"

#include <memory>
#include <source_location>
#include <spdlog/async_logger.h>
#include <spdlog/common.h>
#include <spdlog/details/thread_pool.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/spdlog-inl.h>
#include <string>
#include <vector>

namespace neon::core
{

	Logger::Logger(const std::string& log_path = "log.txt")
	{
		auto consoleSink =
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>();// 创建一个控制台输出的sink，支持彩色输出

		auto fileSink =
			std::make_shared<spdlog::sinks::basic_file_sink_mt>(
				log_path,
				true
			);

		std::vector<spdlog::sink_ptr> sinks =
		{
			consoleSink
		};

		logger = make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end());

		logger->set_level(spdlog::level::trace);
		logger->set_pattern("[%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [thread:%t] %v");

		auto consoleSink_async =
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>();// 创建一个控制台输出的sink，支持彩色输出

		auto fileSink_async =
			std::make_shared<spdlog::sinks::basic_file_sink_mt>(
				log_path,
				true
			);

		std::vector<spdlog::sink_ptr> sinks_async =
		{
			consoleSink_async,
			fileSink_async
		};

		thread_pool = std::make_shared<spdlog::details::thread_pool>(8192, 1);

		async_logger = std::make_shared<spdlog::async_logger>(
			"neon_logger",
			sinks.begin(),
			sinks.end(),
			thread_pool
		);

		async_logger->set_level(spdlog::level::trace);
		async_logger->set_pattern("[%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [thread:%t] %v");
	}







}