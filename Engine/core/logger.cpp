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
	//文件sink有问题，以后再改，先用控制台sink
	Logger::Logger(const std::string& log_path)
	{
		//同步日志初始化
		//普通级别时不需要源代码位置，只有在debug和trace级别时才需要源代码位置
		auto consoleSink =
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>();// 创建一个控制台输出的sink，支持彩色输出

		auto fileSink =
			std::make_shared<spdlog::sinks::basic_file_sink_mt>(
				log_path,
				true
			);

		std::vector<spdlog::sink_ptr> sinks =
		{
			consoleSink,
			//fileSink
		};

		logger = make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end());

		logger->set_level(spdlog::level::info);
		logger->set_pattern("[%d %H:%M:%S.%e] [%^%l%$] %v");

		//初始化同步调试模式logger
		auto consoleSink_debug =
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>();// 创建一个控制台输出的sink，支持彩色输出

		auto fileSink_debug =
			std::make_shared<spdlog::sinks::basic_file_sink_mt>(
				log_path,
				true
			);

		std::vector<spdlog::sink_ptr> sinks_debug =
		{
			consoleSink_debug,
			//fileSink_debug
		};
		logger_debug = make_shared<spdlog::logger>("logger_debug", sinks_debug.begin(), sinks_debug.end());

		logger_debug->set_level(spdlog::level::trace);
		logger_debug->set_pattern("[%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [thread:%t] %v");

		//异步日志初始化
		auto async_consoleSink =
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>();// 创建一个控制台输出的sink，支持彩色输出

		auto async_fileSink =
			std::make_shared<spdlog::sinks::basic_file_sink_mt>(
				log_path,
				true
			);

		std::vector<spdlog::sink_ptr> async_sinks =
		{
			async_consoleSink
		};

		//async_consoleSink->set_pattern("[%d %H:%M:%S.%e] [%^%l%$] %v");

		thread_pool = std::make_shared<spdlog::details::thread_pool>(8192, 1);

		async_logger = std::make_shared<spdlog::async_logger>(
			"async_logger",
			async_sinks.begin(),
			async_sinks.end(),
			thread_pool
		);

		async_logger->set_level(spdlog::level::info);
		async_logger->set_pattern("[%d %H:%M:%S.%e] [%^%l%$] %v");

		//初始化异步调试模式logger
		auto async_consoleSink_debug =
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>();// 创建一个控制台输出的sink，支持彩色输出

		auto async_fileSink_debug =
			std::make_shared<spdlog::sinks::basic_file_sink_mt>(
				log_path,
				true
			);

		std::vector<spdlog::sink_ptr> async_sinks_debug =
		{
			async_consoleSink_debug
		};
		async_logger_debug = std::make_shared<spdlog::async_logger>(
			"async_logger_debug",
			async_sinks_debug.begin(),
			async_sinks_debug.end(),
			thread_pool
		);

		async_logger_debug->set_level(spdlog::level::trace);
		async_logger_debug->set_pattern("[%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [thread:%t] %v");
	}

}