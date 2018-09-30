#include "VulkanDevice.h"

namespace ASGI {
	bool VKLogicDevice::Init(VkPhysicalDevice physicalDevice) {
		mPhysicalDevice = physicalDevice;
		//
		uint32_t queue_families_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queue_families_count, nullptr);
		if (queue_families_count == 0) {
			return false;
		}
		mQueueFamilys.resize(queue_families_count);
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queue_families_count, &mQueueFamilys[0]);
		//
		int graphics_queue_family_index = -1;
		int compute_queue_family_index = -1;
		for (int i = 0; i < mQueueFamilys.size(); ++i) {
			auto &queueFamily = mQueueFamilys[i];
			if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
				if (graphics_queue_family_index > -1) {
					continue;
				}
				graphics_queue_family_index = i;
			}
			else if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) {
				if (compute_queue_family_index > -1) {
					continue;
				}
				compute_queue_family_index = i;
			}
		}
		//
		if (graphics_queue_family_index == -1) {
			return false;
		}
		//
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		VkDeviceQueueCreateInfo graphics_queue_create_info = {
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			graphics_queue_family_index,
			mQueueFamilys[graphics_queue_family_index].queueCount,
			nullptr
		};
		queueCreateInfos.push_back(graphics_queue_create_info);
		if (compute_queue_family_index > -1) {
			VkDeviceQueueCreateInfo compute_queue_create_info = {
				VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				nullptr,
				0,
				compute_queue_family_index,
				mQueueFamilys[compute_queue_family_index].queueCount,
				nullptr
			};
			queueCreateInfos.push_back(compute_queue_create_info);
		}
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
			queueCreateInfos.size(),
			&queueCreateInfos[0],
			0,
			nullptr,
			static_cast<uint32_t>(desired_extensions.size()),
			desired_extensions.size() > 0 ? &desired_extensions[0] : nullptr,
			&desired_features
		};
		//
		auto result = vkCreateDevice(mPhysicalDevice, &device_create_info, nullptr, &mLogicDevice);
		if ((result != VK_SUCCESS) || (mLogicDevice == VK_NULL_HANDLE)) {
			return false;
		}
		//
		for (int i = 0; i < mQueueFamilys[graphics_queue_family_index].queueCount; ++i) {
			VkQueue queue;
			vkGetDeviceQueue(mLogicDevice, graphics_queue_family_index, i, &queue);
			//
			QueueItm tmp;
			tmp.familyIndex = graphics_queue_family_index;
			tmp.queue = queue;
			tmp.queueFlags = mQueueFamilys[graphics_queue_family_index].queueFlags;
			//
			VkFenceCreateInfo fenceCreateInfo = {};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.pNext = nullptr;
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			if (vkCreateFence(mLogicDevice, &fenceCreateInfo, nullptr, &tmp.signalingFence) != VK_SUCCESS) {
				continue;
			}
			//
			VkSemaphoreCreateInfo semaphoreCreateInfo = {};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreCreateInfo.pNext = nullptr;
			semaphoreCreateInfo.flags = 0;
			if (vkCreateSemaphore(mLogicDevice, &semaphoreCreateInfo, nullptr, &tmp.signalingSemaphore) != VK_SUCCESS) {
				continue;
			}
			//
			VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
			cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolCreateInfo.pNext = nullptr;
			cmdPoolCreateInfo.queueFamilyIndex = graphics_queue_family_index;
			cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			if (vkCreateCommandPool(mLogicDevice, &cmdPoolCreateInfo, nullptr, &tmp.cmdPool) != VK_SUCCESS) {
				continue;
			}
			//
			VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.commandPool = tmp.cmdPool;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAllocInfo.pNext = nullptr;
			cmdBufferAllocInfo.commandBufferCount = 1;
			if (vkAllocateCommandBuffers(mLogicDevice, &cmdBufferAllocInfo, &tmp.primaryCmdBuffer) != VK_SUCCESS) {
				continue;
			}
			//
			mGraphicsQueues[(long long)queue] = tmp;
		}
		//
		if (mGraphicsQueues.size() == 0) {
			return false;
		}
		//
		if (compute_queue_family_index == -1) {
			mComputeQueues = mGraphicsQueues;
			return true;
		}
		//
		for (int i = 0; i < mQueueFamilys[compute_queue_family_index].queueCount; ++i) {
			VkQueue queue;
			vkGetDeviceQueue(mLogicDevice, compute_queue_family_index, i, &queue);
			//
			QueueItm tmp;
			tmp.familyIndex = compute_queue_family_index;
			tmp.queue = queue;
			tmp.queueFlags = mQueueFamilys[compute_queue_family_index].queueFlags;
			//
			VkFenceCreateInfo fenceCreateInfo = {};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.pNext = nullptr;
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			if (vkCreateFence(mLogicDevice, &fenceCreateInfo, nullptr, &tmp.signalingFence) != VK_SUCCESS) {
				continue;
			}
			//
			VkSemaphoreCreateInfo semaphoreCreateInfo = {};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreCreateInfo.pNext = nullptr;
			semaphoreCreateInfo.flags = 0;
			if (vkCreateSemaphore(mLogicDevice, &semaphoreCreateInfo, nullptr, &tmp.signalingSemaphore) != VK_SUCCESS) {
				continue;
			}
			//
			//
			VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
			cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolCreateInfo.pNext = nullptr;
			cmdPoolCreateInfo.queueFamilyIndex = compute_queue_family_index;
			cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			if (vkCreateCommandPool(mLogicDevice, &cmdPoolCreateInfo, nullptr, &tmp.cmdPool) != VK_SUCCESS) {
				continue;
			}
			//
			VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.commandPool = tmp.cmdPool;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAllocInfo.pNext = nullptr;
			cmdBufferAllocInfo.commandBufferCount = 1;
			if (vkAllocateCommandBuffers(mLogicDevice, &cmdBufferAllocInfo, &tmp.primaryCmdBuffer) != VK_SUCCESS) {
				continue;
			}
			//
			mComputeQueues[(long long)queue] = tmp;
		}
		//
		if (mComputeQueues.size() == 0) {
			mComputeQueues = mGraphicsQueues;
		}
		//
		return true;
	}


	VkResult VKLogicDevice::ExcuteCmdsOnIdleGraphicsQueue(VkCommandBuffer* cmdBuffers, bool waiteFinished) {
		auto queue = GetIdleGraphicsQueue();
		if (queue == nullptr) {
			return VK_NOT_READY;
		}
		//
		auto &tmp = mGraphicsQueues[(long long)queue];
		//
		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = cmdBuffers;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &tmp.signalingSemaphore;
		auto res = vkQueueSubmit(queue, 1, &submitInfo, tmp.signalingFence);
		if (res != VK_SUCCESS) {
			return res;
		}
		//
		if (waiteFinished) {
			res = vkQueueWaitIdle(queue);
		}
		//
		return res;
	}

	VkResult VKLogicDevice::ExcuteCmdsOnIdleGraphicsQueue(std::vector<VkCommandBuffer>& cmdBuffers, bool waiteFinished) {
		auto queue = GetIdleGraphicsQueue();
		if (queue == nullptr) {
			return VK_NOT_READY;
		}
		//
		auto &tmp = mGraphicsQueues[(long long)queue];
		//
		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = cmdBuffers.size();
		submitInfo.pCommandBuffers = &cmdBuffers[0];
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &tmp.signalingSemaphore;
		auto res = vkQueueSubmit(queue, 1, &submitInfo, tmp.signalingFence);
		if (res != VK_SUCCESS) {
			return res;
		}
		//
		if (waiteFinished) {
			res = vkQueueWaitIdle(queue);
		}
		//
		return res;
	}

	VkResult VKLogicDevice::ExcuteCmdsOnIdleGraphicsQueue(std::vector<VkCommandBuffer>& cmdBuffers, std::vector<VkQueue>& waiteQueues, bool waiteFinished) {
		auto queue = GetIdleGraphicsQueue();
		if (queue == nullptr) {
			return VK_NOT_READY;
		}
		//
		std::vector<VkSemaphore> waiteSemaphores;
		for (auto &waiteQueue : waiteQueues) {
			auto itr_graphics = mGraphicsQueues.find((long long)waiteQueue);
			if (itr_graphics != mGraphicsQueues.end()) {
				waiteSemaphores.push_back(itr_graphics->second.signalingSemaphore);
				continue;
			}
			//
			auto itr_compute = mComputeQueues.find((long long)waiteQueue);
			if (itr_compute != mComputeQueues.end()) {
				waiteSemaphores.push_back(itr_compute->second.signalingSemaphore);
			}
		}
		//
		auto &tmp = mGraphicsQueues[(long long)queue];
		//
		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = cmdBuffers.size();
		submitInfo.pCommandBuffers = &cmdBuffers[0];
		submitInfo.waitSemaphoreCount = waiteSemaphores.size();
		submitInfo.pWaitSemaphores = submitInfo.waitSemaphoreCount == 0 ? nullptr : &waiteSemaphores[0];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &tmp.signalingSemaphore;
		auto res = vkQueueSubmit(queue, 1, &submitInfo, tmp.signalingFence);
		if (res != VK_SUCCESS) {
			return res;
		}
		//
		if (waiteFinished) {
			res = vkQueueWaitIdle(queue);
		}
		//
		return res;
	}
}