#pragma once

#include "ASGI.hpp"
#include "Resource.h"

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
		virtual Buffer* CreateBuffer(uint64_t size, BufferUsageFlags usageFlags) = 0;
		virtual BufferUpdateContext* BeginUpdateBuffer() = 0;
		virtual bool EndUpdateBuffer(BufferUpdateContext* pUpdateContext) = 0;
		virtual void UpdateBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext = nullptr) = 0;
		virtual void* MapBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode = MapMode::MAP_MODE_WRITE) = 0;
		virtual void UnMapBuffer(Buffer* pbuffer) = 0;
		//texture resource
		virtual Image2D* CreateImage2D(uint32_t sizeX, uint32_t sizeY, Format format, uint32_t numMips, SampleCountFlagBits samples, ImageUsageFlags usageFlags) = 0;
		virtual ImageView* CreateImageView(Image2D* srcImage, uint32_t mipLevel) = 0;
		virtual ImageView* CreateImageView(Image2D* srcImage, uint32_t mipLevel, uint32_t numMipLevels, Format format) = 0;

		virtual ImageUpdateContext* BeginUpdateImage() = 0;
		virtual bool EndUpdateImage(ImageUpdateContext* pUpdateContext) = 0;
		virtual bool UpdateImage2D(Image2D* pimg, uint32_t level, uint32_t offsetX, uint32_t offsetY, uint32_t sizeX, uint32_t sizeY, void* pdata, ImageUpdateContext* pUpdateContext = nullptr) = 0;
		//render command
		virtual ExcuteQueue* AcquireExcuteQueue() = 0;
		virtual void WaitQueueExcuteFinished(uint32_t numWaiteQueue, ExcuteQueue* excuteQueues) = 0;
		virtual CommandBuffer* AcquireCommandBuffer(const CommandBufferCreateInfo& create_info) = 0;
		virtual bool ExcuteCommands(ExcuteQueue* excuteQueue, uint32_t numBuffers, CommandBuffer* cmdBuffers, uint32_t numWaiteQueue, ExcuteQueue* waiteQueues) = 0;
		virtual bool BeginCommandBuffer(CommandBuffer* cmdBuffer) = 0;
		virtual bool EndCommandBuffer(CommandBuffer* cmdBuffer) = 0;
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
