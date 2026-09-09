#include "texture_manager.h"

namespace neon::graphics::gl
{
	// 检查是否为合法的图片文件
	void checkImageFile(const std::filesystem::path& p) {

		namespace fs = std::filesystem;

		// 1. 检查文件是否存在且是一个常规文件（排除文件夹等）
		if (!fs::exists(p) || !fs::is_regular_file(p)) {
			throw std::runtime_error("File does not exist or is not a regular file: " + p.string());
		}

		// 2. 获取文件后缀并转为小写（防止大写后缀如 .JPG 识别失败）
		std::string ext = p.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		// 3. 匹配常见的图片后缀
		if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
			ext == ".bmp" || ext == ".gif" || ext == ".webp") {
			// 合法的图片文件
		}
		else {
			throw std::runtime_error("Unsupported image file format: " + ext);
		}
	}

	std::weak_ptr<Texture> TextureManager::loadTexture(const std::filesystem::path& path_std, bool forceReload)
	{
		std::string path = path_std.string();
		//logger.debug("Star to load texture \"{}\"", path);

		//std::lock_guard<std::mutex> lock(cacheMutex_);

		// 不用强制重新加载时，先检查缓存

		if (!forceReload)
		{
			auto it = textureCache_.find(path);
			if (it != textureCache_.end())
			{
				return it->second; // 返回缓存的指针
			}
		}

		//file op

		checkImageFile(path);

		// 2. 调用实际的纹理加载函数（在neon::graphics命名空间）
		// 注意：需要使用完全限定名，因为TextureManager在neon::graphics::gl命名空间
		// image.cpp中的loadTexture_stb函数返回gl::Texture对象
		logger.debug("Star to load texture \"{}\"", path);
		auto texture = std::make_shared<gl::Texture>(
			std::move(neon::graphics::loadTexture_stb(path))
		);

		// 3. 存入缓存
		textureCache_[path] = texture;

		return texture;

	}
}
