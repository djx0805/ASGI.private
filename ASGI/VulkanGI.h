#pragma once
#include <vector>
#include "DynamicGI.h"
#include "VulkanResource.h"
#include "VulkanDevice.h"
#include "VulkanCommand.h"

namespace ASGI {
	class VulkanGI : public DynamicGI {
	public:
		bool Init(const char* device_name, ICmdBufferTaskQueue* cmdBufferTaskQueue = nullptr) override;

		ShaderModule* CreateShaderModule(const ShaderModuleCreateInfo& create_info) override;
		ShaderProgram* CreateShaderProgram(ShaderModule* pVertexShader, ShaderModule* pGeomteryShader, ShaderModule* pTessControlShader, ShaderModule* pTessEvaluationShader, ShaderModule* pFragmentShader) override;
		RenderPass* CreateRenderPass(const RenderPassCreateInfo& create_info) override;
		GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& create_info) override;
		Swapchain* CreateSwapchain(const SwapchainCreateInfo& create_info) override;
		FrameBuffer* CreateFrameBuffer(RenderPass* targetRenderPass, uint8_t numAttachment, ImageView** attachments, ClearValue* clearValues, uint32_t width, uint32_t height) override;

		ComputePass* CreateComputePass() override;
		ComputePipeline* CreateComputePipeline() override;

		Buffer* CreateBuffer(uint64_t size, BufferUsageFlags usageFlags) override;
		BufferUpdateContext* BeginUpdateBuffer() override;
		bool EndUpdateBuffer(BufferUpdateContext* pUpdateContext) override;
		void UpdateBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext = nullptr) override;
		void* MapBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode = MapMode::MAP_MODE_WRITE) override;
		void UnMapBuffer(Buffer* pbuffer) override;
		void BindUniformBuffer(ShaderProgram* pProgram, uint8_t setIndex, uint32_t bindingIndex, Buffer* pbuffer, uint32_t offset, uint32_t size) override;

		Image2D* CreateImage2D(uint32_t sizeX, uint32_t sizeY, Format format, uint32_t numMips, SampleCountFlagBits samples, ImageUsageFlags usageFlags) override;
		ImageView* CreateImageView(Image2D* srcImage, uint32_t mipLevel) override;
		ImageView* CreateImageView(Image2D* srcImage, uint32_t mipLevel, uint32_t numMipLevels, Format format) override;

		ImageUpdateContext* BeginUpdateImage() override;
		bool EndUpdateImage(ImageUpdateContext* pUpdateContext) override;
		bool UpdateImage2D(Image2D* pimg, uint32_t level, uint32_t offsetX, uint32_t offsetY, uint32_t sizeX, uint32_t sizeY, void* pdata, ImageUpdateContext* pUpdateContext = nullptr) override;

		Sampler* CreateSampler(float minLod = 0.0f, float maxLod = 0.0f, float  mipLodBias = 0.0f,
			Filter magFilter = Filter::FILTER_LINEAR, Filter minFilter = Filter::FILTER_LINEAR,
			SamplerMipmapMode mipmapMode = SamplerMipmapMode::SAMPLER_MIPMAP_MODE_LINEAR,
			SamplerAddressMode addressModeU = SamplerAddressMode::SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			SamplerAddressMode addressModeV = SamplerAddressMode::SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			SamplerAddressMode addressModeW = SamplerAddressMode::SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			float  maxAnisotropy = 1.0f, CompareOp compareOp = CompareOp::COMPARE_OP_NEVER,
			BorderColor borderColor = BorderColor::BORDER_COLOR_INT_OPAQUE_WHITE) override;

		void BindTexture(ShaderProgram* pProgram, uint8_t setIndex, uint32_t bindingIndex, ImageView* pImgView, Sampler* pSampler) override;

		ExcuteQueue* AcquireExcuteQueue(QueueType queueType) override;
		void WaitQueueExcuteFinished(uint32_t numWaiteQueue, ExcuteQueue** excuteQueues) override;
		bool SubmitCommands(ExcuteQueue* excuteQueue, uint32_t numBuffers, CommandBuffer** cmdBuffers, uint32_t numWaiteQueue, ExcuteQueue** waiteQueues, uint32_t numSwapchain, Swapchain** waiteSwapchains, bool waiteFinished = false) override;
		void Present(ExcuteQueue* excuteQueue, uint32_t numSwapchain, Swapchain** swapchains, bool waiteFinished = false) override;

		CommandBuffer* CreateCmdBuffer() override;
		void BeginCmdBuffer(CommandBuffer* cmdBuffer) override {
			VKCommandBuffer::Cast(cmdBuffer)->Clear();
			VKCommandBuffer::Cast(cmdBuffer)->PushCommand(new VKCmdBeginCmdBuffer());
		}
		void EndCmdBuffer(CommandBuffer* cmdBuffer) override {
			VKCommandBuffer::Cast(cmdBuffer)->PushCommand(new VKCmdEndCmdBuffer());
		}
		bool BeginRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, FrameBuffer* frameBuffer) override;
		void EndSubRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) override;
		void EndRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) override;
		bool BeginComputePass(CommandBuffer* cmdBuffer, ComputePass* computePass) override;
		void EndComputePass(CommandBuffer* cmdBuffer, ComputePass* computePass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) override;
		//
		inline void VulkanGI::CmdBindPipeline(CommandBuffer*  cmdBuffer, GraphicsPipeline* pipeline) override {
			VKCommandBuffer::Cast(cmdBuffer)->PushCommand(new VKCmdBindPipeline(pipeline));
		}

		inline void VulkanGI::CmdSetViewport(CommandBuffer*  cmdBuffer, uint32_t   firstViewport, uint32_t  viewportCount, Viewport*  pViewports) override {
			VKCommandBuffer::Cast(cmdBuffer)->PushCommand(new VKCmdSetViewport(firstViewport, viewportCount, pViewports));
		}

		inline void VulkanGI::CmdSetScissor(CommandBuffer*  cmdBuffer, uint32_t  firstScissor, uint32_t   scissorCount, Rect2D*  pScissors)  override {
			VKCommandBuffer::Cast(cmdBuffer)->PushCommand(new VKCmdSetScissor(firstScissor, scissorCount, pScissors));
		}

		inline void VulkanGI::CmdSetLineWidth(CommandBuffer*  cmdBuffer, float   lineWidth)  override {
			VKCommandBuffer::Cast(cmdBuffer)->PushCommand(new VKCmdSetLineWidth(lineWidth));
		}

		inline void VulkanGI::CmdBindIndexBuffer(CommandBuffer* cmdBuffer, Buffer* pBuffer, uint32_t offset, Format indexFormat)  override {
			VKCommandBuffer::Cast(cmdBuffer)->PushCommand(new VKCmdBindIndexBuffer(pBuffer, offset, indexFormat));
		}

		inline void VulkanGI::CmdBindVertexBuffer(CommandBuffer* cmdBuffer, uint32_t  bindingIndex, Buffer*  pBuffer, uint32_t offset) override {
			VKCommandBuffer::Cast(cmdBuffer)->PushCommand(new VKCmdBindVertexBuffer(bindingIndex, pBuffer, offset));
		}

		inline void VulkanGI::CmdDraw(CommandBuffer* cmdBuffer, uint32_t vertexCount, uint32_t  instanceCount, uint32_t firstVertex, uint32_t  firstInstance) override {
			VKCommandBuffer::Cast(cmdBuffer)->PushCommand(new VKCmdDraw(vertexCount, instanceCount, firstVertex, firstInstance));
		}

		inline void VulkanGI::CmdDrawIndexed(CommandBuffer* cmdBuffer, uint32_t indexCount, uint32_t   instanceCount, uint32_t  firstIndex, int32_t  vertexOffset, uint32_t  firstInstance)  override {
			VKCommandBuffer::Cast(cmdBuffer)->PushCommand(new VKCmdDrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance));
		}
	private:
		bool getInstanceLevelExtensions();
		bool createVKInstance(std::vector<char const *>& desired_extensions);
		bool createLogicDevice(const char* physic_device_name);
		bool createBuffer(uint64_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VKBuffer* pres);
		bool updateBuffer(VKBuffer* buffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext);
		bool BeginSingleTimeCommands();
		bool EndSingleTimeCommands();
	private:
		ICmdBufferTaskQueue* mCmdBufferTaskQueue;
		std::vector<VkExtensionProperties> mVkInstanceExtensions;
		VkInstance mVkInstance;
		std::vector<VkExtensionProperties> mVkDeviceExtensions;
		VkPhysicalDevice mVkPhysicalDevice;
		VkPhysicalDeviceFeatures mVkDeviceFeatures;
		VkPhysicalDeviceMemoryProperties mVkDeviceMemoryProperties;
		VkPhysicalDeviceProperties mVkDeviceProperties;
		VKLogicDevice mLogicDevice;
		VKSwapchain* mSwapchain = nullptr;
		VKCmdBufferManager* mCmdBufferManger;
	};
}