#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "gl/texture.h"
#include "image.h"



namespace neon::graphics::gl
{
	class TextureManager
	{
		std::unordered_map<std::string, std::shared_ptr<gl::Texture>> textureCache_;
		mutable std::mutex cacheMutex_;

	public:
		/// <summary>
		/// 使用stb_image库加载纹理，并创建OpenGL纹理对象。
		/// 如果纹理已加载，则返回缓存的纹理的共享指针。
		/// </summary>
		/// <param name="path">文件路径</param>
		/// <param name="forceReload">是否强制重新加载，忽略缓存</param>
		/// <returns>纹理对象的共享指针。如果加载失败，返回空指针</returns>
		std::shared_ptr<gl::Texture> loadTexture(const std::string& path, bool forceReload = false)
		{
			std::lock_guard<std::mutex> lock(cacheMutex_);

			// 1. 检查缓存
			if (!forceReload)
			{
				auto it = textureCache_.find(path);
				if (it != textureCache_.end())
				{
					return it->second; // 返回缓存的共享指针
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
				return nullptr; // 返回空指针表示加载失败
			}
		}
	};
}