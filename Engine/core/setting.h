#pragma once

#include "graphics/window_info.h"

namespace neon::core
{
	struct Setting
	{
		int FrameRate = 60;
		float gravity = 0.8f;			// 引力常数：总质量归一化为 1；半径 1、初速 1 的圆盘需 ≈0.5~1 才会成团
		float gravitySoftening = 0.05f;	// 软化长度：避免两粒子靠得极近时引力发散
		graphics::WindowInfo windowInfo{
			800, // width
			600, // height
			"Neon Engine", // title
			false, // fullscreen
			0, // monitorIndex
			true // resizable
		};
		int pch = 1080;
		int pcw = 1920;
	};
}
