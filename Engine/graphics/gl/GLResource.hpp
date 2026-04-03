#pragma once

#include <glad.h>

#include "GLResource_base.hpp"

namespace neon::graphics::gl
{
	// 纹理资源
	struct TextureDeleter
	{
		void operator()(GLuint id) const noexcept
		{
			glDeleteTextures(1, &id);
		}
	};

	struct TexturesDeleter
	{
		void operator()(GLsizei n, const GLuint* ids) const noexcept
		{
			glDeleteTextures(n, ids);
		}
	};

	using Texture = GLResource<TextureDeleter>;
	using Textures = GLResources<TexturesDeleter>;

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

	using Buffer = GLResource<BufferDeleter>;
	using Buffers = GLResources<BuffersDeleter>;

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

	using VertexArray = GLResource<VertexArrayDeleter>;
	using VertexArrays = GLResources<VertexArraysDeleter>;

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

	class Program {
		GLResource<ProgramDeleter> handle;
	public:
		explicit Program() {};
		explicit Program(GLuint id) : handle(id) {};
		void use() const noexcept { glUseProgram(handle.getID()); }
		auto getID() const noexcept { return handle.getID(); }
	};
	//using Programs = GLResources<ProgramsDeleter>;

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

	class Shader {
		GLResource<ShaderDeleter> handle;
		GLenum type;
	public:
		Shader() {};
		Shader(GLuint id, GLenum type) : handle(id), type(type) {};
		auto getType() const noexcept { return type; }
		auto getID() const noexcept { return handle.getID(); }
	};
	using Shaders = GLResources<ShadersDeleter>;

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

	using Framebuffer = GLResource<FramebufferDeleter>;
	using Framebuffers = GLResources<FramebuffersDeleter>;

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

	using Renderbuffer = GLResource<RenderbufferDeleter>;
	using Renderbuffers = GLResources<RenderbuffersDeleter>;

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

	using Sampler = GLResource<SamplerDeleter>;
	using Samplers = GLResources<SamplersDeleter>;

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

	using Query = GLResource<QueryDeleter>;
	using Queries = GLResources<QueriesDeleter>;

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

	using TransformFeedback = GLResource<TransformFeedbackDeleter>;
	using TransformFeedbacks = GLResources<TransformFeedbacksDeleter>;

	// 同步对象 (GLsync 不是 GLuint，需要特殊处理)
	// 注意：Sync 对象不是 GLuint 类型，不能使用上面的模板

} // namespace neon::graphics::gl