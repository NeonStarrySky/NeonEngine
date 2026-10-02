#pragma once
#include <glad/glad.h>

#include <algorithm>
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

		explicit GLResources(std::vector<GLuint> initialIds = {})
		{
			reset(std::move(initialIds));
		}

		bool isValid(size_t index) const { return 0 != ids.at(index); }

		GLuint getID(size_t index) const { return ids.at(index); }
		// 提供一个获取 ID 的指针的方法，方便与 OpenGL 函数交互，请不要直接修改这个指针指向的值，除非你知道自己在做什么
		GLuint* getIDPtr(size_t index) const { return &ids.at(index); }

		size_t size() { return ids.size(); }
		// 释放当前持有的 OpenGL 资源后接管新的 ID。
		void reset(std::vector<GLuint> newIds = {})
		{
			// 一组 ID 代表一组所有权。去重，避免同一个 ID 在析构时被重复释放。
			std::vector<GLuint> uniqueNewIds;
			uniqueNewIds.reserve(newIds.size());
			for (const GLuint id : newIds)
			{
				if (id != 0 && std::find(uniqueNewIds.begin(), uniqueNewIds.end(), id) == uniqueNewIds.end())
				{
					uniqueNewIds.push_back(id);
				}
			}

			// 只删除不再由本对象持有的 ID。重叠部分继续由 reset 后的对象持有。
			std::vector<GLuint> releasedIds;
			releasedIds.reserve(ids.size());
			for (const GLuint id : ids)
			{
				if (id != 0 &&
					std::find(uniqueNewIds.begin(), uniqueNewIds.end(), id) == uniqueNewIds.end() &&
					std::find(releasedIds.begin(), releasedIds.end(), id) == releasedIds.end())
				{
					releasedIds.push_back(id);
				}
			}

			if (!releasedIds.empty())
			{
				Deleter{}(safe_gl_size(releasedIds.size()), releasedIds.data());
			}
			ids = std::move(uniqueNewIds);
		}

		std::vector<GLuint> release() noexcept { return std::exchange(ids, {}); }

		GLResources(GLResources&& other) noexcept : ids(other.release()) {}
		GLResources& operator=(GLResources&& other) noexcept
		{
			//清理自己的资源并且置零other
			if (this != &other)
			{
				if (!ids.empty())
				{
					Deleter{}(safe_gl_size(ids.size()), ids.data());
				}
				ids = other.release();
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
			// reset(getID()) 只是保留当前所有权，不应先删除再继续保存同一个 ID。
			if (id == newId) {
				return;
			}
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
