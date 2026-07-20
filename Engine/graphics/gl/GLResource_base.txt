#pragma once
#include <glad.h>

#include <cassert>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace neon::graphics::gl
{
	namespace GLResource_ {
		template<typename D>
		concept GLResourceDeleter =
			std::is_nothrow_invocable_v<D, const GLuint>;

		template<typename D>
		concept GLResourcesDeleter =
			std::is_nothrow_invocable_v<D, GLsizei, const GLuint*>;
	}

	/// @brief 一个通用的 OpenGL 资源管理类，使用 RAII 原则来管理 OpenGL 资源的生命周期
	template<GLResource_::GLResourcesDeleter Deleter>
	class GLResources
	{
		GLsizei safe_gl_size(size_t n)
		{
			assert(n <= std::numeric_limits<GLsizei>::max());
			return static_cast<GLsizei>(n);
		}
		std::vector<GLuint> ids;
	public:

		using HandleType = typename GLuint;

		explicit GLResources(std::vector<GLuint> ids = std::vector<GLuint>) : ids(std::move(ids)) {}

		bool isValid(size_t index) const { return 0 != ids.at(index); }

		GLuint getID(size_t index) const noexcept { return ids.at(index); }
		// 提供一个获取 ID 的指针的方法，方便与 OpenGL 函数交互，请不要直接修改这个指针指向的值，除非你知道自己在做什么
		GLuint* getIDPtr(size_t index) const noexcept { return &ids.at(index); }

		size_t size() { return ids.size(); }
		//移动语义
		void reset(std::vector<GLuint> ids = {}) { this->ids = std::move(ids); }

		std::vector<GLuint> release() noexcept { return std::exchange(ids, {}); }

		GLResources(GLResources&& other) noexcept
		{
			this->ids = std::move(other.ids);
		}
		GLResources& operator=(GLResources&& other) noexcept
		{
			//清理自己的资源并且置零other
			if (this != &other)
			{
				this->reset(other.ids);
			}
			return *this;
		}

		//拷贝语义没必要
		GLResources(const GLResources&) = delete;
		GLResources& operator=(const GLResources&) = delete;

		~GLResources()
		{
			Deleter{}(
				safe_gl_size(ids.size()),
				ids.data()
				);
		}
	};

	template<GLResource_::GLResourceDeleter Deleter>
	class GLResource {
		GLuint id;

	public:
		using HandleType = GLuint;

		explicit GLResource(GLuint id = 0) noexcept : id(id) {}

		bool isValid() const noexcept { return 0 != id; }

		GLuint getID() const noexcept { return id; }
		// 提供一个获取 ID 的指针的方法，方便与 OpenGL 函数交互，请不要直接修改这个指针指向的值，除非你知道自己在做什么
		GLuint* getIDPtr() noexcept { return &id; }
		const GLuint* getIDPtr() const noexcept { return &id; }

		// 移动语义
		void reset(GLuint newId = 0) noexcept {
			if (id != 0) {
				Deleter{}(id);
			}
			id = newId;
		}

		GLuint release() noexcept { return std::exchange(id, 0); }

		GLResource(GLResource&& other) noexcept : id(other.release()) {}

		GLResource& operator=(GLResource&& other) noexcept {
			if (this != &other) {
				reset(other.release());
			}
			return *this;
		}

		// 拷贝语义删除
		GLResource(const GLResource&) = delete;
		GLResource& operator=(const GLResource&) = delete;

		~GLResource() {
			if (id != 0) {
				Deleter{}(id);
			}
		}
	};
}