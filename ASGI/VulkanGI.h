#pragma once
#include <vector>
#include "DynamicGI.h"
#include "VulkanResource.h"

namespace ASGI {
	class VulkanGI : public DynamicGI {
	public:
		bool Init(const char* device_name) override;

		ShaderModule* CreateShaderModule(const ShaderModuleCreateInfo& create_info) override;
		RenderPass* CreateRenderPass(const RenderPassCreateInfo& create_info) override;
		GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& create_info) override;
		Swapchain* CreateSwapchain(const SwapchainCreateInfo& create_info) override;

		VertexBuffer* CreateVertexBuffer(uint64_t size, BufferUsageFlags usageFlags) override;
		IndexBuffer* CreateIndexBuffer(uint32_t size, BufferUsageFlags usageFlags) override;
		UniformBuffer* CreateUniformBuffer(uint32_t size, BufferUsageFlags usageFlags) override;
		void BeginUpdateBuffer() override;
		void EndUpdateBuffer() override;
		void UpdateVertexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, bool inBatch = false) override;
		void UpdateIndexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, bool inBatch = false) override;
		void UpdateUniformBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, bool inBatch = false) override;
		void* MapVertexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode = MapMode::MAP_MODE_WRITE) override;
		void UnMapVertexBuffer(VertexBuffer* pbuffer) override;
		void* MapIndexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode = MapMode::MAP_MODE_WRITE) override;
		void UnMapIndexBuffer(VertexBuffer* pbuffer) override;
		void* MapUniformBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode = MapMode::MAP_MODE_WRITE) override;
		void UnMapUniformBuffer(VertexBuffer* pbuffer) override;
		void DestroyBuffer(Buffer* targetBuffer) override;

		Texture2D* CreateTexture2D(uint32_t sizeX, uint32_t sizeY, Format format, uint32_t numMips, SampleCountFlagBits samples, ImageUsageFlags usageFlags) override;

		CommandBuffer* CreateCommandBuffer(const CommandBufferCreateInfo& create_info) override;
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
		int getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);
		bool createBuffer(uint64_t size, VkBufferUsageFlags usageFlags, uint32_t createFlags, VKBuffer* pres);
		bool updateBuffer(VKBuffer* buffer, uint32_t offset, uint32_t size, const char* pdata);
	private:
		std::vector<VkExtensionProperties> mVkInstanceExtensions;
		VkInstance mVkInstance;
		std::vector<VkExtensionProperties> mVkDeviceExtensions;
		VkPhysicalDevice mVkPhysicalDevice;
		VkPhysicalDeviceFeatures mVkDeviceFeatures;
		VkPhysicalDeviceMemoryProperties mVkDeviceMemoryProperties;
		VkPhysicalDeviceProperties mVkDeviceProperties;
		VkDevice mVkLogicDevice;
		VkQueue mVkGraphicsQueue;
		VKSwapchain* mSwapchain = nullptr;
		void* mVkMemoryAllocator = nullptr;
	};
}