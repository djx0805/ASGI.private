#pragma once

#include "VulkanSDK\1.1.77.0\Include\vulkan\vulkan.h"

namespace ASGI {
	enum class VKImageLayoutBarrier
	{
		Undefined,
		HostWrite,
		HostRead,
		TransferDest,
		ColorAttachment,
		DepthStencilAttachment,
		TransferSource,
		Present,
		PixelShaderRead,
		PixelDepthStencilRead,
		ComputeGeneralRW,
	};

	inline VkPipelineStageFlags GetImageBarrierFlags(VKImageLayoutBarrier Target, VkAccessFlags& AccessFlags, VkImageLayout& Layout)
	{
		VkPipelineStageFlags StageFlags = (VkPipelineStageFlags)0;
		switch (Target)
		{
		case VKImageLayoutBarrier::Undefined:
			AccessFlags = 0;
			StageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			break;
		case VKImageLayoutBarrier::HostWrite:
			AccessFlags = VK_ACCESS_HOST_WRITE_BIT;
			StageFlags = VK_PIPELINE_STAGE_HOST_BIT;
			Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			break;
		case VKImageLayoutBarrier::HostRead:
			AccessFlags = VK_ACCESS_HOST_READ_BIT;
			StageFlags = VK_PIPELINE_STAGE_HOST_BIT;
			Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			break;
		case VKImageLayoutBarrier::TransferDest:
			AccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
			StageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			Layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			break;

		case VKImageLayoutBarrier::ColorAttachment:
			AccessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			StageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;

		case VKImageLayoutBarrier::DepthStencilAttachment:
			AccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			StageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			break;

		case VKImageLayoutBarrier::TransferSource:
			AccessFlags = VK_ACCESS_TRANSFER_READ_BIT;
			StageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			Layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			break;

		case VKImageLayoutBarrier::Present:
			AccessFlags = 0;
			StageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			Layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			break;

		case VKImageLayoutBarrier::PixelShaderRead:
			AccessFlags = VK_ACCESS_SHADER_READ_BIT;
			StageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;

		case VKImageLayoutBarrier::PixelDepthStencilRead:
			AccessFlags = VK_ACCESS_SHADER_READ_BIT;
			StageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			break;

		case VKImageLayoutBarrier::ComputeGeneralRW:
			AccessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			StageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			Layout = VK_IMAGE_LAYOUT_GENERAL;
			break;

		default:
			break;
		}

		return StageFlags;
	}

	class VKMemory {
	public :
		enum MemoryUsage
		{
			VK_MEMORY_USAGE_UNKNOWN = 0,
			VK_MEMORY_USAGE_GPU_ONLY = 1,
			VK_MEMORY_USAGE_CPU_ONLY = 2,
			VK_MEMORY_USAGE_CPU_TO_GPU = 3,
			VK_MEMORY_USAGE_GPU_TO_CPU = 4
		} ;
	public:
		inline uint32_t GetMemoryTypeIndex() { return mMemoryTypeIndex; }
		inline VkDeviceMemory GetDeviceMemory() { return mDeviceMemory; }
		inline VkDeviceSize GetOffset() { return mOffset; }
		inline VkDeviceSize GetSize() { return mSize; }
	protected:
		VKMemory(uint32_t memTypeIndex, VkDeviceMemory deviceMemory, VkDeviceSize offset, VkDeviceSize size) {
			mMemoryTypeIndex = memTypeIndex;
			mDeviceMemory = deviceMemory;
			mOffset = offset;
			mSize = size;
		}
		virtual ~VKMemory() {}
	protected:
		uint32_t mMemoryTypeIndex;
		VkDeviceMemory mDeviceMemory;
		VkDeviceSize mOffset;
		VkDeviceSize mSize;
	};


	class VKMemoryManager {
	public:
		static VKMemoryManager* Instance();
	public:
		virtual ~VKMemoryManager() {}
		//
		virtual bool Init(VkPhysicalDevice physicalDevice, VkDevice logicDevice) = 0;
		virtual VkResult CreateBuffer(const VkBufferCreateInfo& bufferCreateInfo,  VkBuffer* pBuffer, VKMemory::MemoryUsage memoryUsage, VKMemory*& pMemory) = 0;
		virtual VkResult CreateImage(const VkImageCreateInfo& imageCreateInfo, VkImage* pImage, VKMemory::MemoryUsage memoryUsage, VKMemory*& pMemory) = 0;
		virtual VkResult CreateImageView(VkImageViewCreateInfo& imageViewCreateInfo, VkImageView* pimgView) = 0;
		virtual VkResult MapMemory(VKMemory* pMemory, void** pData) = 0;
		virtual void UnMapMemory(VKMemory* pMemory) = 0;
		virtual void FlushAllocation(VKMemory* pMemory, uint32_t offset, uint32_t size) = 0;
		virtual void DestoryBuffer(VkBuffer buffer, VKMemory*& pMemory) = 0;
		virtual void DestoryImage(VkImage image, VKMemory*& pMemory) = 0;
		virtual void DestoryImageView(VkImageView imageView) = 0;
	};
}
