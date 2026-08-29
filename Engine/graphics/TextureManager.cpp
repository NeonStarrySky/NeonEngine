#include "TextureManager.h"

namespace neon::graphics::gl
{
	std::weak_ptr<Texture> TextureManager::loadTexture(const std::string& path, bool forceReload)
	{
		std::lock_guard<std::mutex> lock(cacheMutex_);

		// 不用强制重新加载时，先检查缓存

		if (!forceReload)
		{
			auto it = textureCache_.find(path);
			if (it != textureCache_.end())
			{
				return it->second; // 返回缓存的指针
			}
		}


		try
		{
			// 2. 调用实际的纹理加载函数（在neon::graphics命名空间）
			// 注意：需要使用完全限定名，因为TextureManager在neon::graphics::gl命名空间
			// image.cpp中的loadTexture_stb函数返回gl::Texture对象
			auto texture = std::make_shared<gl::Texture>(
				std::move(neon::graphics::loadTexture_stb(path))
			);

			// 3. 存入缓存
			textureCache_[path] = texture;

			return texture;
		}
		catch (const std::exception& e)
		{

			// 4. 加载失败，记录错误（实际项目中应使用日志系统）
			// 这里可以根据需要添加日志输出
			return std::weak_ptr<Texture>(); // 返回空指针表示加载失败
		}
	}
}
