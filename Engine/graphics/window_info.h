#pragma once

#include <string>

namespace neon::graphics
{
	struct WindowInfo
	{
		// »ů±ľ´°żÚĐĹĎ˘
		int width = 800;                // ´°żÚżí¶Č
		int height = 600;               // ´°żÚ¸ß¶Č
		std::string title = "T I T L E";   // ´°żÚ±ęĚâ

		// ĎÔĘľşÍ·ç¸ń
		bool fullscreen = false;        // ĘÇ·ńČ«ĆÁ
		int monitorIndex = 0;           // ¶ŕĎÔĘľĆ÷Ë÷ŇýŁ¬Ä¬ČĎÖ÷ĎÔĘľĆ÷
		bool resizable = true;          // ĘÇ·ńżÉµ÷Őű´óĐˇ
		bool visible = true;            // ĘÇ·ńżÉĽű
		bool focused = true;            // ĘÇ·ń»ńČˇ˝ąµă
		bool decorated = true;          // ĘÇ·ńÓĐ±ßżň/±ęĚâŔ¸


		int refreshRate = 60;           // ĎÔĘľĆ÷Ë˘ĐÂÂĘŁ¨Ö÷ŇŞÓĂÓÚČ«ĆÁŁ©

		// »şłĺÇřÉčÖĂ
		int redBits = 8;                // şěÉ«Í¨µŔ
		int greenBits = 8;              // ÂĚÉ«Í¨µŔ
		int blueBits = 8;               // Ŕ¶É«Í¨µŔ
		int alphaBits = 8;              // AlphaÍ¨µŔ
		int depthBits = 24;             // Éî¶Č»şłĺ
		int stencilBits = 8;            // ÄŁ°ĺ»şłĺ
		int samples = 0;                // MSAA˛ÉŃůĘý

		// ´ąÖ±Í¬˛˝
		bool vsync = true;              // ĘÇ·ńżŞĆôVSync

		// ąąÔěşŻĘý
		//WindowInfo() = default;

	};
}