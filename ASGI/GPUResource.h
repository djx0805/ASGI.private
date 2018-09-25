#pragma once
#include "Definitions.hpp"

namespace ASGI {
	class GPUResource {
	public:
		virtual ~GPUResource() {}
	protected:
		GPUResource(){}
	};

	class VertexBuffer;
	class IndexBuffer;
	class UniformBuffer;
	class Buffer : public GPUResource {
	public:
			virtual ~Buffer() {};
			//
			virtual VertexBuffer* asVertexBuffer() { return nullptr; }
			virtual IndexBuffer* asIndexBuffer() { return nullptr; }
			virtual UniformBuffer* asUniformBuffer() { return nullptr; }
	};

	class VertexBuffer : public Buffer {
	public:
		virtual VertexBuffer* asVertexBuffer() { return this; }
	};

	class IndexBuffer : public Buffer {
	public:
		virtual IndexBuffer* asIndexBuffer() { return this; }
	};

	class UniformBuffer : public Buffer {
	public:
		virtual UniformBuffer* asUniformBuffer() { return this; }
	};

	

	class Image : public GPUResource {

	};

	class Image2D : public Image {

	};

	class CommandBuffer : public GPUResource {

	};

	class ShaderModule : public GPUResource {
	public:
		virtual ~ShaderModule() {}
	};

	class RenderPass : public GPUResource {

	};

	class GraphicsPipeline : public GPUResource {

	};

	class Swapchain : public GPUResource {
	public:
		virtual Format GetSurfaceFormat() = 0;
	};
}
