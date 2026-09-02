#pragma once

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "core/logger.h"
#include "gl/texture.h"
#include "image.h"



namespace neon::graphics::gl
{
	class TextureManager
	{
		using Texture = gl::Texture;// 方便切换

		core::Logger& logger;

		std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache_;
		mutable std::mutex cacheMutex_;

	public:

		TextureManager(core::Logger& logger) : logger(logger) {}

		/// <summary>
		/// 使用stb_image库加载纹理，并创建OpenGL纹理对象。
		/// 如果纹理已加载，则返回缓存的纹理的共享指针。
		/// </summary>
		/// <param name="path">文件路径</param>
		/// <param name="forceReload">是否强制重新加载，忽略缓存</param>
		/// <returns>纹理对象的共享指针。如果加载失败，返回空指针</returns>
		std::weak_ptr<gl::Texture> loadTexture(const std::filesystem::path& path, bool forceReload = false);
	};
}