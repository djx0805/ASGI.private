#pragma once
#include <vector>
#include "DynamicGI.h"
#include "VulkanResource.h"

namespace ASGI {
	class VulkanGI : public DynamicGI {
	public:
		bool Init(const char* device_name) override;
		VertexBuffer* CreateVertexBuffer(const VertexBufferCreateInfo& create_info) override;
		IndexBuffer* CreateIndexBuffer(const IndexBufferCreateInfo& create_info) override;
		Texture2D* CreateTexture2D(const Texture2DCreateInfo& create_info) override;

		CommandBuffer* CreateCommandBuffer(const CommandBufferCreateInfo& create_info) override;
		void CmdSetViewport(CommandBuffer& commandBuffer) override;
		void CmdSetScissor(CommandBuffer& commandBuffer) override;
		void CmdSetLineWidth(CommandBuffer& commandBuffer) override;
		void CmdBindGraphicsPipeline(CommandBuffer& commandBuffer) override;
		void CmdBindIndexBuffer(CommandBuffer& commandBuffer) override;
		void CmdBindVertexBuffer(CommandBuffer& commandBuffer) override;
		void CmdDraw(CommandBuffer& commandBuffer) override;
		void CmdDrawIndexed(CommandBuffer& commandBuffer) override;
	private:
		bool getInstanceLevelExtensions();
		bool createVKInstance(std::vector<char const *>& desired_extensions);
		bool createLogicDevice(const char* physic_device_name);
	private:
		std::vector<VkExtensionProperties> mVkExtensions;
		VkInstance mVkInstance;
		VkPhysicalDevice mVkPhysicalDevice;
		VkPhysicalDeviceFeatures mVkDeviceFeatures;
		VkPhysicalDeviceMemoryProperties mVkDeviceMemoryProperties;
		VkPhysicalDeviceProperties mVkDeviceProperties;
		VkDevice mVkLogicDevice;
		VkQueue mVkQueue;
	};
}