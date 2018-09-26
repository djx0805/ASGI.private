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

	class VKImage2D;
	class VKImageView;
	class VKImage {
		friend class VulkanGI;
	public:
		~VKImage() {
			mOrgiView->unref();
		}
		VKImage2D* asVKImage2D() { return nullptr; }
	protected:
		VKImage(){}
	protected:
	    VkImage mVkImage;
		ImageUsageFlags mUsageFlag;
		void* mAllocation;
		VKImageView* mOrgiView;
		VkImageCreateInfo mImageInfo;
	};

	class VKImageView : public ImageView {
		friend class VulkanGI;
	public:
		VKImageView(Image* pimg, VkImageView pview, VkImageViewCreateInfo viewInfo) {
			mSrcImage = pimg;
			mImageView = pview;
			mViewInfo = viewInfo;
		}
		Image* GetSrcImage() override {
			if (mSrcImage->asImage2D() != nullptr) {
				return (Image*)mSrcImage->asImage2D();
			}
		}
	private:
		image_ptr mSrcImage;
		VkImageView mImageView;
		VkImageViewCreateInfo mViewInfo
	};

	class VKImage2D : public VKImage, public Image2D {
		friend class VulkanGI;
	public:
		VKImage2D(Format format, uint32_t sizeX, uint32_t sizeY, uint32_t numMip) : Image2D(format, sizeX, sizeY, numMip) {}
		//
		ImageView* GetOrigView() override {
			return mOrgiView;
		}
	};

	class VKImageUpdateContext : public ImageUpdateContext {
		friend class VulkanGI;
	private:

	};
}
