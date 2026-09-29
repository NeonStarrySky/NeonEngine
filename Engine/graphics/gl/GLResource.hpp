#pragma once

#include<glad/glad.h>

#include "GLResource_base.hpp"

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



	using VertexArray = GLResource<detail::VertexArrayDeleter>;
	using VertexArrays = GLResources<detail::VertexArraysDeleter>;



	class Program {
		GLResource<detail::ProgramDeleter> handle;
	public:
		Program() = default;
		explicit Program(GLuint id) : handle(id) {};
		void use() const noexcept { glUseProgram(handle.getID()); }
		auto getID() const noexcept { return handle.getID(); }
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