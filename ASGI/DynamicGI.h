#pragma once

#include "ASGI.hpp"
#include "Resource.h"
#include "ICmdBufferTaskQueue.h"

namespace ASGI {
	class DynamicGI {
	public:
		virtual ~DynamicGI() {}

		virtual bool Init(const char* device_name = nullptr, ICmdBufferTaskQueue* cmdBufferTaskQueue = nullptr) = 0;
		//graphics pipeline
		virtual ShaderModule* CreateShaderModule(const ShaderModuleCreateInfo& create_info) = 0;
		virtual GPUProgram* CreateGPUProgram(ShaderModule* pVertexShader, ShaderModule* pGeomteryShader, ShaderModule* pTessControlShader, ShaderModule* pTessEvaluationShader, ShaderModule* pFragmentShader) = 0;

		virtual RenderPass* CreateRenderPass(const RenderPassCreateInfo& create_info) = 0;
		virtual GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& create_info) = 0;
		virtual Swapchain* CreateSwapchain(const SwapchainCreateInfo& create_info) = 0;
		virtual FrameBuffer* CreateFrameBuffer(RenderPass* targetRenderPass, uint8_t numAttachment, ImageView** attachments, ClearValue* clearValues, uint32_t width, uint32_t height) = 0;

		virtual ComputePass* CreateComputePass() = 0;
		virtual ComputePipeline* CreateComputePipeline() = 0;
		//buffer resource
		virtual Buffer* CreateBuffer(uint64_t size, BufferUsageFlags usageFlags) = 0;
		virtual BufferUpdateContext* BeginUpdateBuffer() = 0;
		virtual bool EndUpdateBuffer(BufferUpdateContext* pUpdateContext) = 0;
		virtual void UpdateBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext = nullptr) = 0;
		virtual void* MapBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode = MapMode::MAP_MODE_WRITE) = 0;
		virtual void UnMapBuffer(Buffer* pbuffer) = 0;

		virtual void BindUniformBuffer(GPUProgram* pProgram, uint8_t setIndex, uint32_t bindingIndex, Buffer* pbuffer, uint32_t offset, uint32_t size) = 0;
		//texture resource
		virtual Image2D* CreateImage2D(uint32_t sizeX, uint32_t sizeY, Format format, uint32_t numMips, SampleCountFlagBits samples, ImageUsageFlags usageFlags) = 0;
		virtual ImageView* CreateImageView(Image2D* srcImage, uint32_t mipLevel) = 0;
		virtual ImageView* CreateImageView(Image2D* srcImage, uint32_t mipLevel, uint32_t numMipLevels, Format format) = 0;

		virtual ImageUpdateContext* BeginUpdateImage() = 0;
		virtual bool EndUpdateImage(ImageUpdateContext* pUpdateContext) = 0;
		virtual bool UpdateImage2D(Image2D* pimg, uint32_t level, uint32_t offsetX, uint32_t offsetY, uint32_t sizeX, uint32_t sizeY, void* pdata, ImageUpdateContext* pUpdateContext = nullptr) = 0;
		//render command
		virtual ExcuteQueue* AcquireExcuteQueue(QueueType queueType) = 0;
		virtual void WaitQueueExcuteFinished(uint32_t numWaiteQueue, ExcuteQueue** excuteQueues) = 0;
		virtual bool SubmitCommands(ExcuteQueue* excuteQueue, uint32_t numBuffers, CommandBuffer** cmdBuffers, uint32_t numWaiteQueue, ExcuteQueue** waiteQueues, uint32_t numSwapchain, Swapchain** waiteSwapchains, bool waiteFinished = false) = 0;
		virtual void Present(ExcuteQueue* excuteQueue, uint32_t numSwapchain, Swapchain** swapchains, bool waiteFinished = false) = 0;
		//
		virtual CommandBuffer* CreateCmdBuffer() = 0;
		virtual bool BeginRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, FrameBuffer* frameBuffer) = 0;
		virtual void EndSubRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) = 0;
		virtual void EndRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) = 0;
		virtual bool BeginComputePass(CommandBuffer* cmdBuffer, ComputePass* computePass) = 0;
		virtual void EndComputePass(CommandBuffer* cmdBuffer, ComputePass* computePass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) = 0;
		//
		virtual void CmdBindPipeline(CommandBuffer*  commandBuffer, GraphicsPipeline* pipeline) = 0;
		virtual void CmdSetViewport(CommandBuffer*  commandBuffer, uint32_t   firstViewport, uint32_t  viewportCount, Viewport*  pViewports) = 0;
		virtual void CmdSetScissor(CommandBuffer*  commandBuffer, uint32_t  firstScissor, uint32_t   scissorCount, Rect2D*  pScissors) = 0;
		virtual void CmdSetLineWidth(CommandBuffer*  commandBuffer, float   lineWidth) = 0;
		virtual void CmdBindIndexBuffer(CommandBuffer* commandBuffer, Buffer* pBuffer, uint32_t offset, Format indexFormat) = 0;
		virtual void CmdBindVertexBuffer(CommandBuffer* commandBuffer, uint32_t  bindingIndex, Buffer*  pBuffer, uint32_t offset) = 0;
		virtual void CmdDraw(CommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t  instanceCount, uint32_t firstVertex, uint32_t  firstInstance) = 0;
		virtual void CmdDrawIndexed(CommandBuffer* commandBuffer, uint32_t indexCount, uint32_t   instanceCount, uint32_t  firstIndex, int32_t  vertexOffset, uint32_t  firstInstance) = 0;
		/*
		
		virtual void CmdBindPipeline(CommandBuffer*  commandBuffer, ComputePipeline* pipeline) = 0;
		
		virtual void CmdSetDepthBias(CommandBuffer*  commandBuffer, float  depthBiasConstantFactor, float  depthBiasClamp, float    depthBiasSlopeFactor) = 0;
		virtual void CmdSetBlendConstants(CommandBuffer* commandBuffer, const float   blendConstants[4]) = 0;
		virtual void CmdSetDepthBounds(CommandBuffer* commandBuffer, float minDepthBounds, float  maxDepthBounds) = 0;
		virtual void CmdSetStencilCompareMask(CommandBuffer* commandBuffer, StencilFaceFlags  faceMask, uint32_t  compareMask) = 0;
		virtual void CmdSetStencilWriteMask(CommandBuffer*  commandBuffer, StencilFaceFlags  faceMask, uint32_t  writeMask) = 0;
		virtual void CmdSetStencilReference(CommandBuffer* commandBuffer, StencilFaceFlags  faceMask, uint32_t reference) = 0;
		
		virtual void CmdDrawIndirect(CommandBuffer* commandBuffer, Buffer* buffer, uint32_t  offset, uint32_t  drawCount, uint32_t  stride) = 0;
		virtual void CmdDrawIndexedIndirect(CommandBuffer* commandBuffer, Buffer* buffer, uint32_t offset, uint32_t  drawCount, uint32_t  stride) = 0;
		virtual void CmdDispatch(CommandBuffer* commandBuffer, uint32_t  x, uint32_t y, uint32_t  z) = 0;
		virtual void CmdDispatchIndirect(CommandBuffer* commandBuffer, Buffer* buffer, uint32_t offset) = 0;
		virtual void CmdCopyBuffer(CommandBuffer* commandBuffer, Buffer* srcBuffer, Buffer* dstBuffer, uint32_t  srcOffset, uint32_t dstOffset, uint32_t size) = 0;
		virtual void CmdCopyImage2D(CommandBuffer* commandBuffer, Image* srcImage, Image* dstImage, Offset2D srcOffset, Offset2D dstOffset, Extent2D extent) = 0;
		virtual void CmdBlitImage2D(CommandBuffer* commandBuffer, Image* srcImage, Image* dstImage, Offset2D srcOffset, Offset2D dstOffset, Extent2D extent) = 0;
		//CmdCopyBufferToImage
		//CmdCopyImageToBuffer;
		virtual void CmdUpdateBuffer(CommandBuffer* commandBuffer, Buffer* dstBuffer, uint32_t dstOffset, uint32_t dataSize, const void*  pData) = 0;
		virtual void CmdFillBuffer(CommandBuffer* commandBuffer, Buffer* dstBuffer, uint32_t dstOffset, uint32_t size, uint32_t  data) = 0;
		virtual void CmdClearColorImage(CommandBuffer* commandBuffer, Image* image, void*  pColor) = 0;
		virtual void CmdClearDepthStencilImage(CommandBuffer* commandBuffer, Image* image, float depth, uint32_t stencil) = 0;
		//CmdClearAttachments
		//CmdResolveImage
		virtual void CmdPushConstants(CommandBuffer* commandBuffer, GraphicsPipeline* pipeline, uint32_t  offset, uint32_t  size, const void*  pValues) = 0;

		//vkCmdBeginQuery

		//vkCmdEndQuery

		//vkCmdResetQueryPool

		//vkCmdWriteTimestamp

		//vkCmdCopyQueryPoolResults
		*/
	};
}
