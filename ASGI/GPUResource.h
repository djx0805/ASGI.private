#pragma once

namespace ASGI {
	class GPUResource {
	public:
		virtual ~GPUResource() {}
	};

	class VertexBuffer : public GPUResource {
	public:

	};

	class IndexBuffer : public GPUResource {

	};

	class Texture : public GPUResource {

	};

	class Texture2D : public Texture {

	};

	class CommandBuffer : public GPUResource {

	};
}
