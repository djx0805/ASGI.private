#pragma once
#include <vector>
#include "DynamicGI.h"
#include "VulkanResource.h"
#include "VulkanDevice.h"

namespace ASGI {
	class VulkanGI : public DynamicGI {
	public:
		bool Init(const char* device_name) override;

		ShaderModule* CreateShaderModule(const ShaderModuleCreateInfo& create_info) override;
		RenderPass* CreateRenderPass(const RenderPassCreateInfo& create_info) override;
		GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& create_info) override;
		Swapchain* CreateSwapchain(const SwapchainCreateInfo& create_info) override;

		Buffer* CreateBuffer(uint64_t size, BufferUsageFlags usageFlags) override;
		BufferUpdateContext* BeginUpdateBuffer() override;
		bool EndUpdateBuffer(BufferUpdateContext* pUpdateContext) override;
		void UpdateBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext = nullptr) override;
		void* MapBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode = MapMode::MAP_MODE_WRITE) override;
		void UnMapBuffer(Buffer* pbuffer) override;

		Image2D* CreateImage2D(uint32_t sizeX, uint32_t sizeY, Format format, uint32_t numMips, SampleCountFlagBits samples, ImageUsageFlags usageFlags) override;
		ImageView* CreateImageView(Image2D* srcImage, uint32_t mipLevel) override;
		ImageView* CreateImageView(Image2D* srcImage, uint32_t mipLevel, uint32_t numMipLevels, Format format) override;

		ImageUpdateContext* BeginUpdateImage() override;
		bool EndUpdateImage(ImageUpdateContext* pUpdateContext) override;
		bool UpdateImage2D(Image2D* pimg, uint32_t level, uint32_t offsetX, uint32_t offsetY, uint32_t sizeX, uint32_t sizeY, void* pdata, ImageUpdateContext* pUpdateContext = nullptr) override;

		ExcuteQueue* AcquireExcuteQueue() override;
		void WaitQueueExcuteFinished(uint32_t numWaiteQueue, ExcuteQueue* excuteQueues) override;
		CommandBuffer* AcquireCommandBuffer(const CommandBufferCreateInfo& create_info) override;
		bool ExcuteCommands(ExcuteQueue* excuteQueue, uint32_t numBuffers, CommandBuffer* cmdBuffers, uint32_t numWaiteQueue, ExcuteQueue* waiteQueues) override;
		bool BeginCommandBuffer(CommandBuffer* cmdBuffer) override;
		bool EndCommandBuffer(CommandBuffer* cmdBuffer) override;
		void CmdSetViewport(CommandBuffer& commandBuffer) override;
		void CmdSetScissor(CommandBuffer& commandBuffer) override;
		void CmdSetLineWidth(CommandBuffer& commandBuffer) override;
		void CmdBindGraphicsPipeline(CommandBuffer& commandBuffer) override;
		void CmdBindIndexBuffer(CommandBuffer& commandBuffer) override;
		void CmdBindVertexBuffer(CommandBuffer& commandBuffer) override;
		void CmdFillBuffer(CommandBuffer& commandBuffer) override;
		void CmdUpdateBuffer(CommandBuffer& commandBuffer) override;
		void CmdClearColorImage(CommandBuffer& commandBuffer) override;
		void CmdClearDepthStencilImage(CommandBuffer& commandBuffer) override;
		void CmdDraw(CommandBuffer& commandBuffer) override;
		void CmdDrawIndexed(CommandBuffer& commandBuffer) override;
	private:
		bool getInstanceLevelExtensions();
		bool createVKInstance(std::vector<char const *>& desired_extensions);
		bool createLogicDevice(const char* physic_device_name);
		bool createBuffer(uint64_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VKBuffer* pres);
		bool updateBuffer(VKBuffer* buffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext);
		bool BeginSingleTimeCommands();
		bool EndSingleTimeCommands();
	private:
		std::vector<VkExtensionProperties> mVkInstanceExtensions;
		VkInstance mVkInstance;
		std::vector<VkExtensionProperties> mVkDeviceExtensions;
		VkPhysicalDevice mVkPhysicalDevice;
		VkPhysicalDeviceFeatures mVkDeviceFeatures;
		VkPhysicalDeviceMemoryProperties mVkDeviceMemoryProperties;
		VkPhysicalDeviceProperties mVkDeviceProperties;
		VKLogicDevice mLogicDevice;
		VKSwapchain* mSwapchain = nullptr;
		VkCommandBuffer mVkUploadCmdBuffer;
	};
}