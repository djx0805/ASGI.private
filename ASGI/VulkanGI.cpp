#include "VulkanResource.h"
#include "VulkanGI.h"

#pragma comment(lib, "VulkanSDK\\1.1.77.0\\Lib\\vulkan-1.lib")

namespace ASGI {

	bool VulkanGI::getInstanceLevelExtensions() {
		uint32_t extensions_count = 0;
		VkResult res = VK_SUCCESS;

		res = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);
		if (res != VK_SUCCESS || extensions_count == 0) {
			return false;
		}
		//
		mVkExtensions.resize(extensions_count);
		res = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, &mVkExtensions[0]);
		if (res != VK_SUCCESS || extensions_count == 0) {
			return false;
		}
		//
		return true;
	}

	bool VulkanGI::createVKInstance(std::vector<char const *>& desired_extensions) {
		VkApplicationInfo application_info = {
			VK_STRUCTURE_TYPE_APPLICATION_INFO,
			nullptr,
			"vulkanGI",
			VK_MAKE_VERSION(1, 0, 0),
			"vulkanGI",
			VK_MAKE_VERSION(1, 0, 0),
			VK_MAKE_VERSION(1, 1, 0)
		};
		//
		VkInstanceCreateInfo instance_create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			nullptr,
			0,
			&application_info,
			0,
			nullptr,
			static_cast<uint32_t>(desired_extensions.size()),
			desired_extensions.size() > 0 ? &desired_extensions[0] : nullptr
		};
		//
		VkResult result = vkCreateInstance(&instance_create_info, nullptr, &mVkInstance);
		if ((result != VK_SUCCESS) || (mVkInstance == VK_NULL_HANDLE)) {
			return false;
		}
		//
		return true;
	}

	bool VulkanGI::createLogicDevice(const char* physic_device_name) {
		uint32_t devices_count = 0;
		VkResult result = VK_SUCCESS;
		result = vkEnumeratePhysicalDevices(mVkInstance, &devices_count, nullptr);
		if ((result != VK_SUCCESS) ||
			(devices_count == 0)) {
			return false;
		}
		//
		std::vector<VkPhysicalDevice> available_devices;
		available_devices.resize(devices_count);
		result = vkEnumeratePhysicalDevices(mVkInstance, &devices_count, &available_devices[0]);
		if ((result != VK_SUCCESS) ||
			(devices_count == 0)) {
			return false;
		}
		//
		for (int n = 0; n < available_devices.size(); ++n) {
			vkGetPhysicalDeviceProperties(available_devices[n], &mVkDeviceProperties);
			//
			mVkPhysicalDevice = available_devices[n];
			vkGetPhysicalDeviceFeatures(available_devices[n], &mVkDeviceFeatures);
			//
			if (physic_device_name == nullptr || std::string(mVkDeviceProperties.deviceName) == std::string(physic_device_name)) {
				//
				break;
			}
		}
		//
		uint32_t extensions_count = 0;
		result = vkEnumerateDeviceExtensionProperties(mVkPhysicalDevice, nullptr, &extensions_count, nullptr);
		if ((result != VK_SUCCESS) ||
			(extensions_count == 0)) {
			return false;
		}
		//
		std::vector<VkExtensionProperties> available_extensions;
		available_extensions.resize(extensions_count);
		result = vkEnumerateDeviceExtensionProperties(mVkPhysicalDevice, nullptr, &extensions_count, &available_extensions[0]);
		if ((result != VK_SUCCESS) ||
			(extensions_count == 0)) {
			return false;
		}
		//
		uint32_t queue_families_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(mVkPhysicalDevice, &queue_families_count, nullptr);
		if (queue_families_count == 0) {
			return false;
		}
		//
		std::vector<VkQueueFamilyProperties> queue_families;
		queue_families.resize(queue_families_count);
		vkGetPhysicalDeviceQueueFamilyProperties(mVkPhysicalDevice, &queue_families_count, &queue_families[0]);
		if (queue_families_count == 0) {
			return false;
		}
		//
		VkDeviceQueueCreateInfo queue_create_info = {
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			0,
			1,
			nullptr
		};
		//
		vkGetPhysicalDeviceMemoryProperties(mVkPhysicalDevice, &mVkDeviceMemoryProperties);
		//
		std::vector<char const *> desired_extensions;
		desired_extensions.push_back("VK_KHR_swapchain");
		VkPhysicalDeviceFeatures desired_features;
		memset(&desired_features, 0, sizeof(VkPhysicalDeviceFeatures));
		desired_features.tessellationShader = VK_TRUE;
		desired_features.geometryShader = VK_TRUE;
		VkDeviceCreateInfo device_create_info = {
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			nullptr,
			0,
			1,
			&queue_create_info,
			0,
			nullptr,
			static_cast<uint32_t>(desired_extensions.size()),
			desired_extensions.size() > 0 ? &desired_extensions[0] : nullptr,
			&desired_features
		};
		//
		result = vkCreateDevice(mVkPhysicalDevice, &device_create_info, nullptr, &mVkLogicDevice);
		if ((result != VK_SUCCESS) || (mVkLogicDevice == VK_NULL_HANDLE)) {
			return false;
		}
		//
		vkGetDeviceQueue(mVkLogicDevice, 0, 0, &mVkQueue);
		//
		return true;
	}

	bool VulkanGI::Init(const char* device_name) {
		GSupportParallelCommandBuffer = true;
		//
		if (!getInstanceLevelExtensions()) {
			return false;
		}
		//
		std::vector<char const *> desired_extensions;
#ifdef _DEBUG
		desired_extensions.push_back("VK_EXT_debug_report");
		desired_extensions.push_back("VK_EXT_debug_utils");
#endif // DEBUG
#ifdef _WINDOWS
		desired_extensions.push_back("VK_KHR_surface");
		desired_extensions.push_back("VK_KHR_win32_surface");
#endif // _WINDOWS
		if (!createVKInstance(desired_extensions)) {
			return false;
		}
		//
		if (!createLogicDevice(device_name)) {
			return false;
		}
		//
		return true;
	}

	VertexBuffer* VulkanGI::CreateVertexBuffer(const VertexBufferCreateInfo& create_info) {
		return nullptr;
	}

	IndexBuffer* VulkanGI::CreateIndexBuffer(const IndexBufferCreateInfo& create_info) {
		return nullptr;
	}

	Texture2D* VulkanGI::CreateTexture2D(const Texture2DCreateInfo& create_info) {
		return nullptr;
	}

	CommandBuffer* VulkanGI::CreateCommandBuffer(const CommandBufferCreateInfo& create_info) {
		return nullptr;
	}

	void VulkanGI::CmdSetViewport(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdSetScissor(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdSetLineWidth(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdBindGraphicsPipeline(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdBindIndexBuffer(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdBindVertexBuffer(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdDraw(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdDrawIndexed(CommandBuffer& commandBuffer) {
	}
}