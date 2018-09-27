#include "VulkanMemory.h"

#define VMA_IMPLEMENTATION
#include "third_lib\VulkanMemoryAllocator\src\vk_mem_alloc.h"

namespace ASGI {
	class VKMemoryVma : public VKMemory {
		friend class VKMemoryMangerVma;
	public:
		VKMemoryVma(uint32_t memTypeIndex, VkDeviceMemory deviceMemory, VkDeviceSize offset, VkDeviceSize size) : 
			VKMemory(memTypeIndex, deviceMemory, offset, size){}
		~VKMemoryVma() {}
	private:
		VmaAllocation mAllocation = VK_NULL_HANDLE;
	};

	class VKMemoryMangerVma : public VKMemoryManager {
	public:
		bool Init(VkPhysicalDevice physicalDevice, VkDevice logicDevice) override {
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = physicalDevice;
			allocatorInfo.device = logicDevice;
			if (vmaCreateAllocator(&allocatorInfo, &mAllocator) != VK_SUCCESS) {
				return false;
			}
			//
			return true;
		}

		VkResult CreateBuffer(const VkBufferCreateInfo& bufferCreateInfo, VkBuffer* pBuffer, VKMemory::MemoryUsage memoryUsage, VKMemory*& pMemory) override {
			VmaAllocationCreateInfo allocCreateInfo = {};
			allocCreateInfo.usage = (VmaMemoryUsage)memoryUsage;

			VmaAllocation bufferAlloc = VK_NULL_HANDLE;
			auto res = vmaCreateBuffer(mAllocator, &bufferCreateInfo, &allocCreateInfo, pBuffer, &bufferAlloc, nullptr);
			if (res != VK_SUCCESS) {
				pMemory = nullptr;
				return res;
			}
			//
			pMemory = new  VKMemoryVma(bufferAlloc->GetMemoryTypeIndex(), bufferAlloc->GetMemory(), bufferAlloc->GetOffset(), bufferAlloc->GetSize());
			((VKMemoryVma*)pMemory)->mAllocation = bufferAlloc;
			//
			return res;
		}

		VkResult CreateImage(const VkImageCreateInfo& imageCreateInfo, VkImage* pImage, VKMemory::MemoryUsage memoryUsage, VKMemory*& pMemory) override {
			VmaAllocationCreateInfo allocCreateInfo = {};
			allocCreateInfo.usage = (VmaMemoryUsage)memoryUsage;

			VmaAllocation imageAlloc = VK_NULL_HANDLE;
			auto res = vmaCreateImage(mAllocator, &imageCreateInfo, &allocCreateInfo, pImage, &imageAlloc, nullptr);
			if (res != VK_SUCCESS) {
				pMemory = nullptr;
				return res;
			}
			//
			pMemory = new  VKMemoryVma(imageAlloc->GetMemoryTypeIndex(), imageAlloc->GetMemory(), imageAlloc->GetOffset(), imageAlloc->GetSize());
			((VKMemoryVma*)pMemory)->mAllocation = imageAlloc;
			//
			return res;
		}

		VkResult CreateImageView(VkImageViewCreateInfo& imageViewCreateInfo, VkImageView* pimgView) {
			return vkCreateImageView(mAllocator->m_hDevice, &imageViewCreateInfo, nullptr, pimgView);
		}

		VkResult MapMemory(VKMemory* pMemory, void** pData) override {
			return vmaMapMemory(mAllocator, ((VKMemoryVma*)pMemory)->mAllocation, pData);
		}

		void UnMapMemory(VKMemory* pMemory) override {
			 vmaUnmapMemory(mAllocator, ((VKMemoryVma*)pMemory)->mAllocation);
		}

		void FlushAllocation(VKMemory* pMemory, uint32_t offset, uint32_t size) {
			vmaFlushAllocation(mAllocator, ((VKMemoryVma*)pMemory)->mAllocation, offset, size);
		}

		void DestoryBuffer(VkBuffer buffer, VKMemory*& pMemory) override {
			 vmaDestroyBuffer(mAllocator, buffer, ((VKMemoryVma*)pMemory)->mAllocation);
			 delete (VKMemoryVma*)pMemory;
			 pMemory = nullptr;
		}

		void DestoryImage(VkImage image, VKMemory*& pMemory) override {
			vmaDestroyImage(mAllocator, image, ((VKMemoryVma*)pMemory)->mAllocation);
			delete (VKMemoryVma*)pMemory;
			pMemory = nullptr;
		}

		void DestoryImageView(VkImageView imageView) override {
			vkDestroyImageView(mAllocator->m_hDevice, imageView, nullptr);
		}
	private:
		VmaAllocator mAllocator;
	};

	static VKMemoryMangerVma* pVKMemoryManger = new  VKMemoryMangerVma();
	VKMemoryManager* VKMemoryManager::Instance() {
		return pVKMemoryManger;
	}
}
