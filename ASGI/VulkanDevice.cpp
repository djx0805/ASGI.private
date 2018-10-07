#include "VulkanDevice.h"
#include "VulkanResource.h"

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
		std::vector<float> priorities0(mQueueFamilys[graphics_queue_family_index].queueCount, 1.0);
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		VkDeviceQueueCreateInfo graphics_queue_create_info = {
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			graphics_queue_family_index,
			mQueueFamilys[graphics_queue_family_index].queueCount,
			priorities0.data()
		};
		queueCreateInfos.push_back(graphics_queue_create_info);
		if (compute_queue_family_index > -1) {
			std::vector<float> priorities1(mQueueFamilys[compute_queue_family_index].queueCount, 1.0);
			VkDeviceQueueCreateInfo compute_queue_create_info = {
				VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				nullptr,
				0,
				compute_queue_family_index,
				mQueueFamilys[compute_queue_family_index].queueCount,
				priorities1.data()
			};
			queueCreateInfos.push_back(compute_queue_create_info);
		}
		//
		std::vector<char*> enabledDeviceLayers;
		enabledDeviceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
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
			enabledDeviceLayers.size(),
			enabledDeviceLayers.data(),
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
			VKExcuteQueue* tmp = new VKExcuteQueue();
			tmp->familyIndex = graphics_queue_family_index;
			tmp->mQueue = queue;
			tmp->queueFlags = mQueueFamilys[graphics_queue_family_index].queueFlags;
			tmp->mType = QueueType::QUEUE_TYPE_GRAPHICS;
			//
			VkFenceCreateInfo fenceCreateInfo = {};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.pNext = nullptr;
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			if (vkCreateFence(mLogicDevice, &fenceCreateInfo, nullptr, &tmp->signalingFence) != VK_SUCCESS) {
				continue;
			}
			//
			VkSemaphoreCreateInfo semaphoreCreateInfo = {};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreCreateInfo.pNext = nullptr;
			semaphoreCreateInfo.flags = 0;
			if (vkCreateSemaphore(mLogicDevice, &semaphoreCreateInfo, nullptr, &tmp->signalingSemaphore) != VK_SUCCESS) {
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
			VKExcuteQueue* tmp = new VKExcuteQueue();
			tmp->familyIndex = compute_queue_family_index;
			tmp->mQueue = queue;
			tmp->queueFlags = mQueueFamilys[compute_queue_family_index].queueFlags;
			//
			VkFenceCreateInfo fenceCreateInfo = {};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.pNext = nullptr;
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			if (vkCreateFence(mLogicDevice, &fenceCreateInfo, nullptr, &tmp->signalingFence) != VK_SUCCESS) {
				continue;
			}
			//
			VkSemaphoreCreateInfo semaphoreCreateInfo = {};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreCreateInfo.pNext = nullptr;
			semaphoreCreateInfo.flags = 0;
			if (vkCreateSemaphore(mLogicDevice, &semaphoreCreateInfo, nullptr, &tmp->signalingSemaphore) != VK_SUCCESS) {
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
		mGraphicsQueueFamilyIndex = graphics_queue_family_index;
		mComputeQueueFamilyIndex = compute_queue_family_index;
		//
		return true;
	}

	VkResult VKLogicDevice::ExcuteCmdOnIdleGraphicsQueue(VkCommandBuffer* cmdBuffer, bool waiteFinished) {
		auto tmp = GetIdleGraphicsQueue();
		if (tmp == nullptr) {
			return VK_NOT_READY;
		}
		//
		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = cmdBuffer;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &tmp->signalingSemaphore;
		auto res = vkQueueSubmit(tmp->mQueue, 1, &submitInfo, tmp->signalingFence);
		if (res != VK_SUCCESS) {
			return res;
		}
		//
		if (waiteFinished) {
			res = vkQueueWaitIdle(tmp->mQueue);
		}
		//
		return res;
	}

	VkResult VKLogicDevice::ExcuteCommands(ExcuteQueue* excuteQueue, uint32_t numBuffers, CommandBuffer** cmdBuffers, uint32_t numWaiteQueue, ExcuteQueue** waiteQueues, uint32_t numSwapchain, Swapchain** waiteSwapchains, bool waiteFinished) {
		std::vector<VkSemaphore> waiteSemaphores(numWaiteQueue + numSwapchain);
		std::vector<VkPipelineStageFlags> waiteStages(numWaiteQueue + numSwapchain);
		std::vector<VkCommandBuffer> vkCmdBuffers(numBuffers);
		//
		for (int i = 0; i < numWaiteQueue; ++i) {
			waiteSemaphores[i] = VKExcuteQueue::Cast(waiteQueues[i])->signalingSemaphore;
			waiteStages[i] = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		}
		for (int i = 0; i < numSwapchain; ++i) {
			waiteSemaphores[numWaiteQueue + i] = VKSwapchain::Cast(waiteSwapchains[i])->GetPresentSemaphore();
			waiteStages[numWaiteQueue + i] = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		for (int i = 0; i < numBuffers; ++i) {
			vkCmdBuffers[i] = VKCommandBuffer::Cast(cmdBuffers[i])->GetBindingCmdBuffer();
		}
		//
		auto tmp = VKExcuteQueue::Cast(excuteQueue);
		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr};
		submitInfo.commandBufferCount = numBuffers;
		submitInfo.pCommandBuffers = vkCmdBuffers.data();
		submitInfo.waitSemaphoreCount = waiteSemaphores.size();
		submitInfo.pWaitSemaphores = submitInfo.waitSemaphoreCount > 0 ? waiteSemaphores.data() : nullptr;
		submitInfo.pWaitDstStageMask = submitInfo.waitSemaphoreCount > 0 ? waiteStages.data() : nullptr;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &tmp->signalingSemaphore;
		auto res = vkQueueSubmit(tmp->mQueue, 1, &submitInfo, tmp->signalingFence);
		if (res != VK_SUCCESS) {
			return res;
		}
		//
		if (waiteFinished) {
			res = vkQueueWaitIdle(tmp->mQueue);
		}
		//
		return res;
	}
}