#pragma once

#include "graphics/window_info.h"

namespace neon::core
{
	struct Setting
	{
		int FrameRate = 60;
		float gravity = 0.8f;				// 引力常数：总质量归一化为 1；静止起步、半径 1 的圆盘约需 0.5~1 才会成团
		float gravitySoftening = 0.05f;		// 引力软化长度：避免两粒子靠得极近时引力发散
		float repulsionRadius = 0.08f;		// 近距斥力半径：只有距离小于它才产生斥力
		float repulsionStrength = 0.01f;	// 近距斥力强度：按 1/r⁴ 增长（实现里另有加速度上限，避免数值爆炸）
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
