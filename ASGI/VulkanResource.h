#pragma once
#include <vector>
#include "GPUResource.h"
#ifdef _WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#endif //  _WINDOWNS


#include "VulkanSDK\1.1.77.0\Include\vulkan\vulkan.h"

namespace ASGI {
	class VkSwapchain : public Swapchain {
		friend class VulkanGI;
	private:
		VkSurfaceKHR mVkSurface = nullptr;
		VkSurfaceCapabilitiesKHR mVkSurfaceCapabilities;
		VkSurfaceFormatKHR mVkSurfaceFormat;
		VkPresentModeKHR mVkPresentMode;
		VkSwapchainKHR mVkSwapchain = nullptr;
	};
}
