#pragma once
#include <unordered_map>
#include "VulkanSDK\1.1.77.0\Include\vulkan\vulkan.h"
#include "Resource.h"

namespace ASGI {
	class VKExcuteQueue : public ExcuteQueue {
		friend class VulkanGI;
		friend class VKLogicDevice;
	public:
		static inline VKExcuteQueue* Cast(ExcuteQueue* excuteQueue) {
			return (VKExcuteQueue*)excuteQueue;
		}
	public:
		VKExcuteQueue(GraphicsContext* pcontext) : ExcuteQueue(pcontext) {}
		//
		inline QueueType GetType() override {
			return mType;
		}

		inline  VkQueue GetVKQueue() {
			return mQueue;
		}
	private:
		VkQueue mQueue;
		QueueType mType;
		uint8_t familyIndex;
		VkQueueFlags queueFlags;
		VkSemaphore signalingSemaphore;
		VkFence signalingFence;
	};
	//
	class VKLogicDevice {
	public:
		bool Init(VkPhysicalDevice physicalDevice);

		inline VkDevice GetDevice() {
			return mLogicDevice;
		}

		inline uint32_t GetGraphicsQueueFamilyIndex() {
			return mGraphicsQueueFamilyIndex;
		}

		inline uint32_t GetComputeQueueFamilyIndex() {
			return mComputeQueueFamilyIndex;
		}

		inline VKExcuteQueue* GetIdleGraphicsQueue() {
			for (auto &itr : mGraphicsQueues) {
				if (vkGetFenceStatus(mLogicDevice, itr.second->signalingFence) == VK_SUCCESS) {
					if (vkResetFences(mLogicDevice, 1, &itr.second->signalingFence) == VK_SUCCESS) {
						return itr.second;
					}
				}
			}
			//
			return nullptr; 
		}

		inline VKExcuteQueue* GetIdleComputeQueue() {
			for (auto &itr : mComputeQueues) {
				if (vkGetFenceStatus(mLogicDevice, itr.second->signalingFence) == VK_SUCCESS) {
					if (vkResetFences(mLogicDevice, 1, &itr.second->signalingFence) == VK_SUCCESS) {
						return itr.second;
					}
				}
			}
			//
			return nullptr;
		}

		inline void WaiteQueueFinished(uint32_t numWaiteQueue, ExcuteQueue** excuteQueues) {
			std::vector<VkFence> fences(numWaiteQueue);
			for (int i = 0; i < numWaiteQueue; ++i) {
				fences[i] = mGraphicsQueues[(long long)VKExcuteQueue::Cast(excuteQueues[i])->mQueue]->signalingFence;
			}
			//
			vkWaitForFences(mLogicDevice, fences.size(), fences.data(), true, UINT64_MAX);
		}

		VkResult ExcuteCmdOnIdleGraphicsQueue(VkCommandBuffer* cmdBuffer, bool waiteFinished = true);
		VkResult ExcuteCommands(ExcuteQueue* excuteQueue, uint32_t numBuffers, CommandBuffer** cmdBuffers, uint32_t numWaiteQueue, ExcuteQueue** waiteQueues, uint32_t numSwapchain, Swapchain** waiteSwapchains, bool waiteFinished = false);
	private:
		VkPhysicalDevice mPhysicalDevice;
		VkDevice mLogicDevice;
		std::vector<VkQueueFamilyProperties> mQueueFamilys;
		std::unordered_map<long long, VKExcuteQueue*> mGraphicsQueues;
		std::unordered_map<long long, VKExcuteQueue*> mComputeQueues;
		uint32_t mGraphicsQueueFamilyIndex;
		uint32_t mComputeQueueFamilyIndex;
	};
}
