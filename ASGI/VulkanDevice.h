#pragma once
#include <unordered_map>
#include "VulkanSDK\1.1.77.0\Include\vulkan\vulkan.h"

namespace ASGI {
	class VKLogicDevice {
		struct QueueItm {
			uint8_t familyIndex;
			VkQueueFlags queueFlags;
			VkQueue queue;
			VkSemaphore signalingSemaphore;
			VkFence signalingFence;
			VkCommandPool cmdPool;
			VkCommandBuffer primaryCmdBuffer;
		};
	public:
		bool Init(VkPhysicalDevice physicalDevice);

		VkDevice GetDevice() {
			return mLogicDevice;
		}

		inline VkQueue GetIdleGraphicsQueue() {
			for (auto &itr : mGraphicsQueues) {
				if (vkGetFenceStatus(mLogicDevice, itr.second.signalingFence) == VK_SUCCESS) {
					if (vkResetFences(mLogicDevice, 1, &itr.second.signalingFence) == VK_SUCCESS) {
						return itr.second.queue;
					}
				}
			}
			//
			return nullptr; 
		}

		inline VkQueue GetIdleComputeQueue() {
			for (auto &itr : mComputeQueues) {
				if (vkGetFenceStatus(mLogicDevice, itr.second.signalingFence) == VK_SUCCESS) {
					if (vkResetFences(mLogicDevice, 1, &itr.second.signalingFence) == VK_SUCCESS) {
						return itr.second.queue;
					}
				}
			}
			//
			return nullptr;
		}

		VkResult ExcuteCmdsOnIdleGraphicsQueue(VkCommandBuffer* cmdBuffers, bool waiteFinished = false);
		VkResult ExcuteCmdsOnIdleGraphicsQueue(std::vector<VkCommandBuffer>& cmdBuffers, bool waiteFinished = false);
		VkResult ExcuteCmdsOnIdleGraphicsQueue(std::vector<VkCommandBuffer>& cmdBuffers, std::vector<VkQueue>& waiteQueues, bool waiteFinished = false);
	private:
		VkPhysicalDevice mPhysicalDevice;
		VkDevice mLogicDevice;
		std::vector<VkQueueFamilyProperties> mQueueFamilys;
		std::unordered_map<long long, QueueItm> mGraphicsQueues;
		std::unordered_map<long long, QueueItm> mComputeQueues;
	};
}
