#pragma once

#ifdef ASGI_EXPORTS
#define ASGI_API __declspec(dllexport)
#else
#define ASGI_API __declspec(dllimport)
#endif

namespace ASGI {
	extern ASGI_API bool GSupportParallelCommandBuffer;


	//
	struct VertexBufferCreateInfo {

	};

	struct IndexBufferCreateInfo {

	};


	struct CommandBufferCreateInfo {

	};

	struct  Texture2DCreateInfo
	{

	};
}