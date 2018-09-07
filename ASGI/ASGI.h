#pragma once
#include "ASGI.hpp"
#include "GPUResource.h"


namespace ASGI {
	enum GIType {
		GI_VULKAN,
		GI_OPENGL
	};

	ASGI_API bool Init(GIType driver, const char* device_name = nullptr);
	ASGI_API Swapchain* CreateSwapchain(const SwapchainCreateInfo& create_info);
	ASGI_API VertexBuffer* CreateVertexBuffer(const VertexBufferCreateInfo& create_info);
	ASGI_API IndexBuffer* CreateIndexBuffer(const IndexBufferCreateInfo& create_info);
	ASGI_API Texture2D* CreateTexture2D(const Texture2DCreateInfo& create_info);

	ASGI_API CommandBuffer* CreateCommandBuffer(const CommandBufferCreateInfo& create_info);
	ASGI_API void CmdSetViewport(CommandBuffer& commandBuffer);
	ASGI_API void CmdSetScissor(CommandBuffer& commandBuffer);
	ASGI_API void CmdSetLineWidth(CommandBuffer& commandBuffer);
	ASGI_API void CmdBindGraphicsPipeline(CommandBuffer& commandBuffer);
	ASGI_API void CmdBindIndexBuffer(CommandBuffer& commandBuffer);
	ASGI_API void CmdBindVertexBuffer(CommandBuffer& commandBuffer);
	ASGI_API void CmdDraw(CommandBuffer& commandBuffer);
	ASGI_API void CmdDrawIndexed(CommandBuffer& commandBuffer);
}
