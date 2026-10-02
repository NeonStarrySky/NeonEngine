#pragma once

#include<glad/glad.h>

#include "GLResource_base.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace neon::graphics::gl
{

	namespace detail
	{
		// 纹理资源
		struct TexturesDeleter
		{
			void operator()(GLsizei n, const GLuint* ids) const noexcept
			{
				glDeleteTextures(n, ids);
			}
		};

		// 缓冲区资源
		struct BufferDeleter
		{
			void operator()(GLuint id) const noexcept
			{
				glDeleteBuffers(1, &id);
			}
		};

		struct BuffersDeleter
		{
			void operator()(GLsizei n, const GLuint* ids) const noexcept
			{
				glDeleteBuffers(n, ids);
			}
		};

		// 顶点数组对象
		struct VertexArrayDeleter
		{
			void operator()(GLuint id) const noexcept
			{
				glDeleteVertexArrays(1, &id);
			}
		};

		struct VertexArraysDeleter
		{
			void operator()(GLsizei n, const GLuint* ids) const noexcept
			{
				glDeleteVertexArrays(n, ids);
			}
		};

		// 着色器程序
		struct ProgramDeleter
		{
			void operator()(GLuint id) const noexcept
			{
				glDeleteProgram(id);
			}
		};

		struct ProgramsDeleter
		{
			void operator()(GLsizei n, const GLuint* ids) const noexcept
			{
				for (GLsizei i = 0; i < n; ++i)
				{
					glDeleteProgram(ids[i]);
				}
			}
		};

		// 着色器
		struct ShaderDeleter
		{
			void operator()(GLuint id) const noexcept
			{
				glDeleteShader(id);
			}
		};

		struct ShadersDeleter
		{
			void operator()(GLsizei n, const GLuint* ids) const noexcept
			{
				for (GLsizei i = 0; i < n; ++i)
				{
					glDeleteShader(ids[i]);
				}
			}
		};

		// 帧缓冲区
		struct FramebufferDeleter
		{
			void operator()(GLuint id) const noexcept
			{
				glDeleteFramebuffers(1, &id);
			}
		};

		struct FramebuffersDeleter
		{
			void operator()(GLsizei n, const GLuint* ids) const noexcept
			{
				glDeleteFramebuffers(n, ids);
			}
		};

		// 渲染缓冲区
		struct RenderbufferDeleter
		{
			void operator()(GLuint id) const noexcept
			{
				glDeleteRenderbuffers(1, &id);
			}
		};

		struct RenderbuffersDeleter
		{
			void operator()(GLsizei n, const GLuint* ids) const noexcept
			{
				glDeleteRenderbuffers(n, ids);
			}
		};

		// 采样器
		struct SamplerDeleter
		{
			void operator()(GLuint id) const noexcept
			{
				glDeleteSamplers(1, &id);
			}
		};

		struct SamplersDeleter
		{
			void operator()(GLsizei n, const GLuint* ids) const noexcept
			{
				glDeleteSamplers(n, ids);
			}
		};

		// 查询对象
		struct QueryDeleter
		{
			void operator()(GLuint id) const noexcept
			{
				glDeleteQueries(1, &id);
			}
		};

		struct QueriesDeleter
		{
			void operator()(GLsizei n, const GLuint* ids) const noexcept
			{
				glDeleteQueries(n, ids);
			}
		};

		// 变换反馈
		struct TransformFeedbackDeleter
		{
			void operator()(GLuint id) const noexcept
			{
				glDeleteTransformFeedbacks(1, &id);
			}
		};

		struct TransformFeedbacksDeleter
		{
			void operator()(GLsizei n, const GLuint* ids) const noexcept
			{
				glDeleteTransformFeedbacks(n, ids);
			}
		};
	}

	using Textures = GLResources<detail::TexturesDeleter>;



	using Buffer = GLResource<detail::BufferDeleter>;
	using Buffers = GLResources<detail::BuffersDeleter>;

	/// @brief RAII 管理单个 Uniform Buffer Object；多个 UBO 可由多个实例分别持有。
	/// 所有 OpenGL 操作都要求当前线程存在有效的 OpenGL context。
	class UniformBuffer
	{
		Buffer handle;
		GLsizeiptr capacity = 0;

		void requireValid() const
		{
			if (!handle.isValid() || capacity <= 0)
			{
				throw std::logic_error("UniformBuffer is not allocated");
			}
		}

	public:
		UniformBuffer() = default;
		explicit UniformBuffer(GLsizeiptr size, const void* initialData = nullptr, GLenum usage = GL_DYNAMIC_DRAW)
		{
			allocate(size, initialData, usage);
		}

		UniformBuffer(UniformBuffer&& other) noexcept
			: handle(std::move(other.handle)), capacity(std::exchange(other.capacity, 0)) {}

		UniformBuffer& operator=(UniformBuffer&& other) noexcept
		{
			if (this != &other)
			{
				handle = std::move(other.handle);
				capacity = std::exchange(other.capacity, 0);
			}
			return *this;
		}
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;

		bool isValid() const noexcept { return handle.isValid() && capacity > 0; }
		GLuint getID() const noexcept { return handle.getID(); }
		GLsizeiptr getSize() const noexcept { return capacity; }

		void allocate(GLsizeiptr size, const void* initialData = nullptr, GLenum usage = GL_DYNAMIC_DRAW)
		{
			if (size <= 0)
			{
				throw std::invalid_argument("UniformBuffer size must be greater than zero");
			}

			GLuint id = 0;
			glGenBuffers(1, &id);
			if (id == 0)
			{
				throw std::runtime_error("Failed to create UniformBuffer");
			}

			Buffer candidate(id);
			GLint previousBinding = 0;
			glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &previousBinding);
			glBindBuffer(GL_UNIFORM_BUFFER, id);
			glBufferData(GL_UNIFORM_BUFFER, size, initialData, usage);

			GLint64 allocatedSize = 0;
			glGetBufferParameteri64v(GL_UNIFORM_BUFFER, GL_BUFFER_SIZE, &allocatedSize);
			glBindBuffer(GL_UNIFORM_BUFFER, static_cast<GLuint>(previousBinding));

			if (allocatedSize != size)
			{
				throw std::runtime_error("Failed to allocate UniformBuffer storage");
			}

			handle = std::move(candidate);
			capacity = size;
		}

		void bind() const
		{
			requireValid();
			glBindBuffer(GL_UNIFORM_BUFFER, handle.getID());
		}

		void bindBase(GLuint bindingPoint) const
		{
			requireValid();

			GLint maxBindingPoints = 0;
			glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxBindingPoints);
			if (bindingPoint >= static_cast<GLuint>(maxBindingPoints))
			{
				throw std::out_of_range("UniformBuffer binding point is out of range");
			}

			glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, handle.getID());
		}

		void update(GLintptr offset, GLsizeiptr size, const void* data)
		{
			requireValid();
			if (offset < 0 || size < 0 || offset > capacity || size > capacity - offset)
			{
				throw std::out_of_range("UniformBuffer update exceeds allocated storage");
			}
			if (size > 0 && data == nullptr)
			{
				throw std::invalid_argument("UniformBuffer update data must not be null");
			}
			if (size == 0)
			{
				return;
			}

			GLint previousBinding = 0;
			glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &previousBinding);
			glBindBuffer(GL_UNIFORM_BUFFER, handle.getID());
			glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
			glBindBuffer(GL_UNIFORM_BUFFER, static_cast<GLuint>(previousBinding));
		}

		void reset() noexcept
		{
			handle.reset();
			capacity = 0;
		}
	};



	using VertexArray = GLResource<detail::VertexArrayDeleter>;
	using VertexArrays = GLResources<detail::VertexArraysDeleter>;



	class Program {
		GLResource<detail::ProgramDeleter> handle;
		mutable std::unordered_map<std::string, GLint> uniformLocations;
	public:
		Program() = default;
		explicit Program(GLuint id) : handle(id) {};
		void use() const noexcept { glUseProgram(handle.getID()); }
		auto getID() const noexcept { return handle.getID(); }

		/// @brief 获取 uniform 位置；首次查询后会按名称缓存结果（包括 -1）。
		GLint getUniformLocation(std::string_view name) const
		{
			auto [it, inserted] = uniformLocations.try_emplace(std::string(name));
			if (inserted)
			{
				it->second = glGetUniformLocation(handle.getID(), it->first.c_str());
			}
			return it->second;
		}
	};
	//using Programs = GLResources<ProgramsDeleter>;



	class Shader {
		GLResource<detail::ShaderDeleter> handle;
		GLenum type;

	public:
		Shader() : handle(), type(0) {};
		Shader(GLuint id, GLenum type) : handle(id), type(type) {};
		auto getType() const noexcept { return type; }
		auto getID() const noexcept { return handle.getID(); }
	};
	using Shaders = GLResources<detail::ShadersDeleter>;


	using Framebuffer = GLResource<detail::FramebufferDeleter>;
	using Framebuffers = GLResources<detail::FramebuffersDeleter>;


	using Renderbuffer = GLResource<detail::RenderbufferDeleter>;
	using Renderbuffers = GLResources<detail::RenderbuffersDeleter>;


	using Sampler = GLResource<detail::SamplerDeleter>;
	using Samplers = GLResources<detail::SamplersDeleter>;

	// 查询对象
	using Query = GLResource<detail::QueryDeleter>;
	using Queries = GLResources<detail::QueriesDeleter>;


	using TransformFeedback = GLResource<detail::TransformFeedbackDeleter>;
	using TransformFeedbacks = GLResources<detail::TransformFeedbacksDeleter>;

} // namespace neon::graphics::gl
