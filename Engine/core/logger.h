#pragma once

#include <concepts>
#include <fmt/base.h>
#include <memory>
#include <source_location>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/common.h>
#include <spdlog/details/thread_pool.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <utility>

namespace neon::core
{

	class Logger
	{
	private:
		std::shared_ptr<spdlog::logger> logger;
		std::shared_ptr<spdlog::async_logger> async_logger;
		std::shared_ptr<spdlog::details::thread_pool> thread_pool;

	public:
		Logger(const std::string& log_path);

		enum class LogLevel
		{
			info,
			warn,
			error,
			critical,
			debug,
			trace
		};
		LogLevel currentLogLevel = LogLevel::info;
		enum class LogType
		{
			sync,
			async
		};
		LogType currentLogType = LogType::sync;

		struct Fmt_str_with_loc// 这个结构体用于封装格式化字符串和调用位置
		{
			std::string_view fmt;
			std::source_location loc;

			// 支持从字符串隐式构造，并在此抓取调用位置
			template <typename T>
				requires std::convertible_to<T, std::string_view>
			consteval Fmt_str_with_loc(const T& s, std::source_location l = std::source_location::current())
				: fmt(s), loc(l) {}
		};

		void setLogLevel(LogLevel level) {
			currentLogLevel = level;
		}
		void setLogType(LogType type) {
			currentLogType = type;
		}

		template<typename ...Args>
		void info(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			switch (currentLogType)
			{
			case LogType::sync:
				info_sync(fmt, std::forward<Args>(args)...);
				break;
			case LogType::async:
				info_async(fmt, std::forward<Args>(args)...);
				break;
			default:
				break;
			}
		}

		template<typename ...Args>
		void warn(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			switch (currentLogType)
			{
			case LogType::sync:
				warn_sync(fmt, std::forward<Args>(args)...);
				break;
			case LogType::async:
				warn_async(fmt, std::forward<Args>(args)...);
				break;
			default:
				break;
			}
		}

		template<typename ...Args>
		void error(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			switch (currentLogType)
			{
			case LogType::sync:
				error_sync(fmt, std::forward<Args>(args)...);
				break;
			case LogType::async:
				error_async(fmt, std::forward<Args>(args)...);
				break;
			default:
				break;
			}
		}

		template<typename ...Args>
		void critical(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			switch (currentLogType)
			{
			case LogType::sync:
				critical_sync(fmt, std::forward<Args>(args)...);
				break;
			case LogType::async:
				critical_async(fmt, std::forward<Args>(args)...);
				break;
			default:
				break;
			}
		}

		template<typename ...Args>
		void debug(Fmt_str_with_loc fmt, Args && ...args)
		{
			switch (currentLogType)
			{
			case LogType::sync:
				debug_sync(fmt, std::forward<Args>(args)...);
				break;
			case LogType::async:
				debug_async(fmt, std::forward<Args>(args)...);
				break;
			default:
				break;
			}
		}

		template<typename ...Args>
		void trace(Fmt_str_with_loc fmt, Args && ...args)
		{
			switch (currentLogType)
			{
			case LogType::sync:
				trace_sync(fmt, std::forward<Args>(args)...);
				break;
			case LogType::async:
				trace_async(fmt, std::forward<Args>(args)...);
				break;
			default:
				break;
			}
		}

		// 1. 同步日志记录函数
		template<typename ...Args>
		void info_sync(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			logger->info(fmt, std::forward<Args>(args)...);
		}

		template<typename ...Args>
		void warn_sync(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			logger->warn(fmt, std::forward<Args>(args)...);
		}

		template<typename ...Args>
		void error_sync(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			logger->error(fmt, std::forward<Args>(args)...);
		}

		template<typename ...Args>
		void critical_sync(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			logger->critical(fmt, std::forward<Args>(args)...);
		}

		template<typename ...Args>
		void debug_sync(Fmt_str_with_loc fmt, Args && ...args)
		{
			// 1. 将 std::source_location 转换为 spdlog::source_loc
			spdlog::source_loc loc{
				fmt.loc.file_name(),
				static_cast<int>(fmt.loc.line()),
				fmt.loc.function_name()
			};

			// 2. 将 loc 作为第一个参数传给 spdlog::debug
			logger->log(loc, spdlog::level::debug, fmt::runtime(fmt.fmt), std::forward<Args>(args)...);
		}

		template<typename ...Args>
		void trace_sync(Fmt_str_with_loc fmt, Args && ...args)
		{
			// 1. 将 std::source_location 转换为 spdlog::source_loc
			spdlog::source_loc loc{
				fmt.loc.file_name(),
				static_cast<int>(fmt.loc.line()),
				fmt.loc.function_name()
			};

			// 2. 将 loc 作为第一个参数传给 spdlog::trace
			logger->log(loc, spdlog::level::trace, fmt::runtime(fmt.fmt), std::forward<Args>(args)...);
		}


		// 异步日志记录方法
		template<typename ...Args>
		void info_async(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			async_logger->info(fmt, std::forward<Args>(args)...);
		}

		template<typename ...Args>
		void warn_async(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			async_logger->warn(fmt, std::forward<Args>(args)...);
		}

		template<typename ...Args>
		void error_async(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			async_logger->error(fmt, std::forward<Args>(args)...);
		}

		template<typename ...Args>
		void critical_async(spdlog::format_string_t<Args...> fmt, Args && ...args)
		{
			async_logger->critical(fmt, std::forward<Args>(args)...);
		}

		template<typename ...Args>
		void debug_async(Fmt_str_with_loc fmt, Args && ...args)
		{
			// 1. 将 std::source_location 转换为 spdlog::source_loc
			spdlog::source_loc loc{
				fmt.loc.file_name(),
				static_cast<int>(fmt.loc.line()),
				fmt.loc.function_name()
			};

			// 2. 将 loc 作为第一个参数传给 spdlog::debug
			async_logger->log(loc, spdlog::level::debug, fmt::runtime(fmt.fmt), std::forward<Args>(args)...);
		}

		template<typename ...Args>
		void trace_async(Fmt_str_with_loc fmt, Args && ...args)
		{
			// 1. 将 std::source_location 转换为 spdlog::source_loc
			spdlog::source_loc loc{
				fmt.loc.file_name(),
				static_cast<int>(fmt.loc.line()),
				fmt.loc.function_name()
			};

			// 2. 将 loc 作为第一个参数传给 spdlog::trace
			async_logger->log(loc, spdlog::level::trace, fmt::runtime(fmt.fmt), std::forward<Args>(args)...);
		}
	};




}