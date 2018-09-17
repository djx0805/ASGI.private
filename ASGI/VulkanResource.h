#pragma once
#include <vector>
#include <memory>
#include "GPUResource.h"
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif


#include "VulkanSDK\1.1.77.0\Include\vulkan\vulkan.h"
#include "third_lib\SPIRV-Cross\spirv_cross.hpp"

namespace ASGI {
	class VKShaderModule : public ShaderModule {
		friend class VulkanGI;
	private:
		std::unique_ptr<spirv_cross::Compiler> mSpirvCompiler;
		VkShaderModule mShaderModule;
	};

	class VKSwapchain : public Swapchain {
		friend class VulkanGI;
	private:
		VkSurfaceKHR mVkSurface = nullptr;
		VkSurfaceCapabilitiesKHR mVkSurfaceCapabilities;
		VkSurfaceFormatKHR mVkSurfaceFormat;
		VkPresentModeKHR mVkPresentMode;
		VkSwapchainKHR mVkSwapchain = nullptr;
	};
}
