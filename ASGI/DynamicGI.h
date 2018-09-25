#pragma once

#include "ASGI.hpp"
#include "GPUResource.h"

namespace ASGI {
	class DynamicGI {
	public:
		virtual ~DynamicGI() {}

		virtual bool Init(const char* device_name = nullptr) = 0;
		//graphics pipeline
		virtual ShaderModule* CreateShaderModule(const ShaderModuleCreateInfo& create_info) = 0;
		virtual RenderPass* CreateRenderPass(const RenderPassCreateInfo& create_info) = 0;
		virtual GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& create_info) = 0;
		virtual Swapchain* CreateSwapchain(const SwapchainCreateInfo& create_info) = 0;
		//buffer resource
		virtual VertexBuffer* CreateVertexBuffer(uint64_t size, BufferUsageFlags usageFlags) = 0;
		virtual IndexBuffer* CreateIndexBuffer(uint32_t size, BufferUsageFlags usageFlags) = 0;
		virtual UniformBuffer* CreateUniformBuffer(uint32_t size, BufferUsageFlags usageFlags) = 0;
		virtual BufferUpdateContext* BeginUpdateBuffer() = 0;
		virtual bool EndUpdateBuffer(BufferUpdateContext* pUpdateContext) = 0;
		virtual void UpdateVertexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext = nullptr) = 0;
		virtual void UpdateIndexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext = nullptr) = 0;
		virtual void UpdateUniformBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext = nullptr) = 0;
		virtual void* MapVertexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode = MapMode::MAP_MODE_WRITE) = 0;
		virtual void UnMapVertexBuffer(VertexBuffer* pbuffer) = 0;
		virtual void* MapIndexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode = MapMode::MAP_MODE_WRITE) = 0;
		virtual void UnMapIndexBuffer(VertexBuffer* pbuffer) = 0;
		virtual void* MapUniformBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode = MapMode::MAP_MODE_WRITE) = 0;
		virtual void UnMapUniformBuffer(VertexBuffer* pbuffer) = 0;
		virtual void DestroyBuffer(Buffer* targetBuffer) = 0;
		//texture resource
		virtual Image2D* CreateImage2D(uint32_t sizeX, uint32_t sizeY, Format format, uint32_t numMips, SampleCountFlagBits samples, ImageUsageFlags usageFlags) = 0;

		virtual ImageUpdateContext* BeginUpdateImage() = 0;
		virtual bool EndUpdateImage(ImageUpdateContext* pUpdateContext) = 0;
		virtual bool UpdateImage2D(Image2D* pimg, uint32_t level, const Rect2D& updateRegion, void* pdata, ImageUpdateContext* pUpdateContext = nullptr) = 0;
		//render command
		virtual CommandBuffer* CreateCommandBuffer(const CommandBufferCreateInfo& create_info) = 0;
		virtual void CmdSetViewport(CommandBuffer& commandBuffer) = 0;
		virtual void CmdSetScissor(CommandBuffer& commandBuffer) = 0;
		virtual void CmdSetLineWidth(CommandBuffer& commandBuffer) = 0;
		virtual void CmdBindGraphicsPipeline(CommandBuffer& commandBuffer) = 0;
		virtual void CmdBindIndexBuffer(CommandBuffer& commandBuffer) = 0;
		virtual void CmdBindVertexBuffer(CommandBuffer& commandBuffer) = 0;
		virtual void CmdFillBuffer(CommandBuffer& commandBuffer) = 0;
		virtual void CmdUpdateBuffer(CommandBuffer& commandBuffer) = 0;
		virtual void CmdClearColorImage(CommandBuffer& commandBuffer) = 0;
		virtual void CmdClearDepthStencilImage(CommandBuffer& commandBuffer) = 0;
		virtual void CmdDraw(CommandBuffer& commandBuffer) = 0;
		virtual void CmdDrawIndexed(CommandBuffer& commandBuffer) = 0;
	};
}
