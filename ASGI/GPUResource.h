#pragma once

namespace ASGI {
	class GPUResource {
	public:
		virtual ~GPUResource() {}
	protected:
		GPUResource(){}
	};


	class Buffer : public GPUResource {
	public:
			virtual ~Buffer() {};
	};

	class VertexBuffer : public Buffer {
	public:

	};

	class IndexBuffer : public Buffer {

	};

	class UniformBuffer : public Buffer {

	};

	class Texture : public GPUResource {

	};

	class Texture2D : public Texture {

	};

	class CommandBuffer : public GPUResource {

	};

	class ShaderModule : public GPUResource {
	public:
		virtual ~ShaderModule() {}
	};

	class GraphicsPipeline : public GPUResource {

	};

	class Swapchain : public GPUResource {

	};
}
