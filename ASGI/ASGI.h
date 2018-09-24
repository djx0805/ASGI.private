#pragma once
#include "ASGI.hpp"

namespace ASGI {
	ASGI_API bool Init(GIType driver, const char* device_name = nullptr);
	//
	ASGI_API ShaderModule* CreateShaderModule(const char* shaderPath);
	ASGI_API RenderPass* CreateRenderPass(const RenderPassCreateInfo& create_info);
	ASGI_API GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& create_info);
	ASGI_API Swapchain* CreateSwapchain(const SwapchainCreateInfo& create_info);
	//
	ASGI_API VertexBuffer* CreateVertexBuffer(uint64_t size, const char* pdata = nullptr);
	ASGI_API IndexBuffer* CreateIndexBuffer(uint32_t size, const char* pdata = nullptr);
	ASGI_API UniformBuffer* CreateUniformBuffer(uint32_t size, const char* pdata = nullptr);
	ASGI_API void DestroyBuffer(Buffer* targetBuffer);
	//
	ASGI_API Texture2D* CreateTexture2D(uint32_t sizeX, uint32_t sizeY, Format format, uint32_t numMips, SampleCountFlagBits samples, ImageUsageFlags usageFlags);

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
