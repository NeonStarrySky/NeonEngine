#pragma once

#include "graphics/gl/GLResource.hpp"
#include "graphics/gl/GLResource_base.hpp"

namespace neon::graphics::gl {

	struct TextureDeleter {
		void operator()(GLuint id) const noexcept {
			if (id != 0) glDeleteTextures(1, &id);
		}
	};

	class Texture {
		GLResource<TextureDeleter> handle;

	public:
		// 简化后的纹理信息（移除未使用的字段）
		struct Info {
			GLenum target = GL_TEXTURE_2D;
			GLsizei width = 0;
			GLsizei height = 0;
			GLsizei depth = 1;
			GLenum internalFormat = GL_RGBA8;  // 明确用 sized format
			GLint levels = 1;                   // mipmap 层数
		};

		Texture() = default;

		// 接管现有ID
		explicit Texture(GLuint existingID, const Info& info = Info{})
			: handle(existingID), info_(info) {}

		// 生成新纹理ID（不分配存储，不绑定）
		explicit Texture(const Info& info)
			: info_(info)
		{
			GLuint newID = 0;
			glGenTextures(1, &newID);
			handle.reset(newID);
		}

		GLuint getID() const noexcept { return handle.getID(); }
		bool isValid() const noexcept { return handle.isValid(); }
		const Info& info() const noexcept { return info_; }

		// 绑定到指定纹理单元
		void bind(GLuint textureUnit = 0) const {  // 用 0-31 而不是 GLenum
			assert(isValid());
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(info_.target, getID());
		}

		// 解绑（静态）
		static void unbindTarget(GLenum target) {
			glBindTexture(target, 0);
		}

		void unbind() const {
			glBindTexture(info_.target, 0);
		}

		// 设置纹理参数
		void setParameter(GLenum pname, GLint param) {
			assert(isValid());
			glBindTexture(info_.target, getID());
			glTexParameteri(info_.target, pname, param);
		}

		// ========== 核心改正 ==========

		// 分配不可变存储（现代方式）
		void allocateStorage(GLsizei levels, GLenum internalformat,
			GLsizei width, GLsizei height = 1, GLsizei depth = 1) {
			//assert(isValid() && "Texture invalid!");

			// 先更新信息
			info_.internalFormat = internalformat;
			info_.width = width;
			info_.height = height > 1 ? height : 1;
			info_.depth = depth > 1 ? depth : 1;
			info_.levels = levels;

			// 绑定正确的目标
			glBindTexture(info_.target, getID());

			// 根据目标类型分配
			switch (info_.target) {
			case GL_TEXTURE_1D:
				glTexStorage1D(info_.target, levels, internalformat, width);
				break;
			case GL_TEXTURE_2D:
			case GL_TEXTURE_CUBE_MAP:  // cube map 也是 2D 存储
				glTexStorage2D(info_.target, levels, internalformat, width, height);
				break;
			case GL_TEXTURE_3D:
			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_CUBE_MAP_ARRAY:
				glTexStorage3D(info_.target, levels, internalformat, width, height, depth);
				break;
			default:
				assert(false && "Unsupported texture target");
			}
		}

		// 上传数据到已分配的存储（配合 allocateStorage 使用）
		void uploadData(GLint level,
			GLenum format, GLenum type, const void* data,
			GLint xoffset = 0, GLint yoffset = 0, GLint zoffset = 0,
			GLsizei width = 0, GLsizei height = 0, GLsizei depth = 0) {
			assert(isValid());

			// 默认使用 info 中的尺寸
			if (width == 0) width = info_.width;
			if (height == 0) height = info_.height;
			if (depth == 0) depth = info_.depth;

			glBindTexture(info_.target, getID());

			switch (info_.target) {
			case GL_TEXTURE_1D:
				glTexSubImage1D(info_.target, level, xoffset, width, format, type, data);
				break;
			case GL_TEXTURE_2D:
				glTexSubImage2D(info_.target, level, xoffset, yoffset, width, height, format, type, data);
				break;
			case GL_TEXTURE_3D:
				glTexSubImage3D(info_.target, level, xoffset, yoffset, zoffset,
					width, height, depth, format, type, data);
				break;
				// ... 其他目标
			}
		}

		// 传统方式：一次性分配+上传（学习/兼容用）
		void image(GLint level,
			GLenum internalFormat,
			GLsizei width, GLsizei height, GLsizei depth,
			GLenum format, GLenum type, const void* data) {
			assert(isValid());

			info_.internalFormat = internalFormat;
			info_.width = width;
			info_.height = height;
			info_.depth = depth;

			glBindTexture(info_.target, getID());

			// 使用传统的 glTexImage*（可变存储）
			switch (info_.target) {
			case GL_TEXTURE_1D:
				glTexImage1D(info_.target, level, internalFormat, width, 0, format, type, data);
				break;
			case GL_TEXTURE_2D:
				glTexImage2D(info_.target, level, internalFormat, width, height, 0, format, type, data);
				break;
			case GL_TEXTURE_3D:
				glTexImage3D(info_.target, level, internalFormat, width, height, depth, 0, format, type, data);
				break;
			}
		}

		void generateMipmap() {
			assert(isValid());
			glBindTexture(info_.target, getID());
			glGenerateMipmap(info_.target);
		}

		// 移动语义
		Texture(Texture&&) noexcept = default;
		Texture& operator=(Texture&&) noexcept = default;
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

	private:
		Info info_;
	};

} // namespace