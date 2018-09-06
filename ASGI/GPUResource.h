#pragma once

namespace ASGI {
	class GPUResource {
	public:
		virtual ~GPUResource() {}
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

	class Shader : public GPUResource {
	public:
		virtual ~Shader() {}
	};

	class VertexShader : public Shader {

	};

	class TessControlShader : public Shader {

	};

	class TessEvaluationShader : public Shader {
		
	};

	class GeometryShader : public Shader {

	};

	class FragmentShader : public Shader {

	};

	class GraphicsPipeline : public GPUResource {

	};

	class Swapchain : public GPUResource {

	};
}
