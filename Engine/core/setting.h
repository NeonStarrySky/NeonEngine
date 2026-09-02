#pragma once

#include "graphics/window_info.h"

namespace neon::core
{
	struct Setting
	{
		int FrameRate = 60;
		graphics::WindowInfo windowInfo{
			800, // width
			600, // height
			"Neon Engine", // title
			false, // fullscreen
			0, // monitorIndex
			true // resizable
		};
	};
}
