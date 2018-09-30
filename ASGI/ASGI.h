#pragma once
#include "ASGI.hpp"

namespace ASGI {
	ASGI_API bool Init(GIType driver, const char* device_name = nullptr);
	//
	ASGI_API shader_module_ptr CreateShaderModule(const char* shaderPath);
	ASGI_API render_pass_ptr CreateRenderPass(const RenderPassCreateInfo& create_info);
	ASGI_API graphics_pipeline_ptr CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& create_info);
	ASGI_API swapchain_ptr CreateSwapchain(const SwapchainCreateInfo& create_info);
	//
	ASGI_API buffer_ptr CreateBuffer(uint64_t size, BufferUsageFlags usageFlags);
	ASGI_API buffer_update_contex_ptr BeginUpdateBuffer();
	ASGI_API bool EndUpdateBuffer(BufferUpdateContext* pUpdateContext);
	ASGI_API void UpdateBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext = nullptr);
	ASGI_API void* MapBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode = MapMode::MAP_MODE_WRITE);
	ASGI_API void UnMapBuffer(Buffer* pbuffer);
	//
	//ASGI_API Texture2D* CreateTexture2D(uint32_t sizeX, uint32_t sizeY, Format format, uint32_t numMips, SampleCountFlagBits samples, ImageUsageFlags usageFlags);

	ASGI_API command_buffer_ptr AcquireCommandBuffer(const CommandBufferCreateInfo& create_info);
	ASGI_API void CmdSetViewport(CommandBuffer& commandBuffer);
	ASGI_API void CmdSetScissor(CommandBuffer& commandBuffer);
	ASGI_API void CmdSetLineWidth(CommandBuffer& commandBuffer);
	ASGI_API void CmdBindGraphicsPipeline(CommandBuffer& commandBuffer);
	ASGI_API void CmdBindIndexBuffer(CommandBuffer& commandBuffer);
	ASGI_API void CmdBindVertexBuffer(CommandBuffer& commandBuffer);
	ASGI_API void CmdDraw(CommandBuffer& commandBuffer);
	ASGI_API void CmdDrawIndexed(CommandBuffer& commandBuffer);
}
