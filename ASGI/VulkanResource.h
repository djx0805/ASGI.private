#pragma once
#include <vector>
#include <list>
#include <array>
#include <memory>
#include "ASGI.hpp"
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "VulkanSDK\1.1.77.0\Include\vulkan\vulkan.h"



namespace spirv_cross {
	class Compiler;
}
namespace ASGI {
	class VKShaderModule : public ShaderModule {
		friend class VulkanGI;
	private:
		std::unique_ptr<spirv_cross::Compiler> mSpirvCompiler;
		VkShaderModule mShaderModule;
	};

	class VKSwapchain : public Swapchain {
		friend class VulkanGI;
	public:
		Format GetSurfaceFormat() {
			return (Format)mVkSurfaceFormat.format;
		}
	private:
		VkSurfaceKHR mVkSurface = nullptr;
		VkSurfaceCapabilitiesKHR mVkSurfaceCapabilities;
		VkSurfaceFormatKHR mVkSurfaceFormat;
		VkPresentModeKHR mVkPresentMode;
		VkSwapchainKHR mVkSwapchain = nullptr;
	};

	class VKRenderPass : public RenderPass {
		friend class VulkanGI;
	private:
		VkRenderPass mVkRenderPass;
	};

	class VKGraphicsPipeline : public GraphicsPipeline {
		friend class VulkanGI;
	private:
		VkPipelineLayout mVkPipelineLayout;
		VkPipeline mVkPipeLine;
	};

	class VKBuffer {
		friend class VulkanGI;
	private:
		VkBuffer mVkBuffer;
		BufferUsageFlags mUsageFlag;
		void* mAllocation;
	};

	class VKVertexBuffer : public VKBuffer, public VertexBuffer {
		friend class VulkanGI;
	private:
		
	};

	class VKIndexBuffer : public VKBuffer, public IndexBuffer {
		friend class VulkanGI;
	private:
	
	};

	class VKUniformBuffer : public VKBuffer, public UniformBuffer {
		friend class VulkanGI;
	private:
		
	};

	class VKBufferUpdateContext : public BufferUpdateContext {
		friend class VulkanGI;
	public:
		struct UpdateItem {
			VKBuffer* dstBuffer;
			uint32_t offset;
			uint32_t size;
			void* pdata;
		};
	private:
		std::list<UpdateItem> updates;
	};
}
