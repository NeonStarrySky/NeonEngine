#pragma once

#include <string>

namespace neon::graphics
{
	struct WindowInfo
	{
		// 基本窗口信息
		int width = 800;                // 窗口宽度
		int height = 600;               // 窗口高度
		std::string title = "T I T L E";   // 窗口标题

		// 显示和风格
		bool fullscreen = false;        // 是否全屏
		int monitorIndex = 0;           // 多显示器索引，默认主显示器
		bool resizable = true;          // 是否可调整大小
		bool visible = true;            // 是否可见
		bool focused = true;            // 是否获取焦点
		bool decorated = true;          // 是否有边框/标题栏


		int refreshRate = 60;           // 显示器刷新率（主要用于全屏）

		// 缓冲区设置
		int redBits = 8;                // 红色通道
		int greenBits = 8;              // 绿色通道
		int blueBits = 8;               // 蓝色通道
		int alphaBits = 8;              // Alpha通道
		int depthBits = 24;             // 深度缓冲
		int stencilBits = 8;            // 模板缓冲
		int samples = 0;                // MSAA采样数

		// 垂直同步
		bool vsync = true;              // 是否开启VSync

		// 构造函数
		//WindowInfo() = default;

	};
}