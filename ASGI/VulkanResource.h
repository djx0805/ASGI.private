#pragma once
#include <vector>
#include <list>
#include <array>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ASGI.hpp"

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "VulkanMemory.h"
#include "VulkanCommand.h"

namespace spirv_cross {
	class Compiler;
}
namespace ASGI {
	class VKShaderModule : public ShaderModule {
		friend class VulkanGI;
		friend class VKGPUProgram;
	public:
		inline static VKShaderModule* Cast(ShaderModule* pmodule) {
			return (VKShaderModule*)pmodule;
		}
	public:
		inline const char* const GetEntryName() {
			return mEntryName.c_str();
		}
	private:
		std::unique_ptr<spirv_cross::Compiler> mSpirvCompiler;
		VkShaderModule mShaderModule;
		std::string mEntryName;
	};

	class VKGPUProgram : public GPUProgram {
		friend class VulkanGI;
	public:
		static inline VKGPUProgram* Cast(GPUProgram* gpuProgram) {
			return (VKGPUProgram*)gpuProgram;
		}
	public:
		VKGPUProgram(ShaderModule* pVertexShader, ShaderModule* pGeomteryShader, ShaderModule* pTessControlShader, ShaderModule* pTessEvaluationShader, ShaderModule* pFragmentShader) {
			mVertexShader = pVertexShader;
			mGeomteryShader = pGeomteryShader;
			mTessControlShader = pTessControlShader;
			mTessEvaluationShader = pTessEvaluationShader;
			mFragmentShader = pFragmentShader;
	    }

		inline ShaderModule* GetVertexShader() override {
			return mVertexShader;
		}
		inline ShaderModule* GetGeomteryShader() override {
			return mGeomteryShader;
		}
		inline ShaderModule* GetTessControlShader() override {
			return mTessControlShader;
		}
		inline ShaderModule* GetTessEvaluationShader() override {
			return mTessEvaluationShader;
		}
		inline ShaderModule* GetFragmentShader() override {
			return mFragmentShader;
		}
		//
		bool InitDescriptorSet(VkDevice logicDevice);
		//
		const std::vector<VkDescriptorSet>& GetDescriptorSets() {
			return mDescriptorSets;
		}
	private:
		VkDevice mLogicDevice;
		shader_module_ptr mVertexShader;
		shader_module_ptr mGeomteryShader;
		shader_module_ptr mTessControlShader;
		shader_module_ptr mTessEvaluationShader;
		shader_module_ptr mFragmentShader;
		std::vector<VkPushConstantRange> mPushConstantRanges;
		std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
		VkDescriptorPool mDescriptorPool;
		std::vector<VkDescriptorSet> mDescriptorSets;
		std::unordered_map<uint8_t, int> mIndexSet;
	};

	class VKImage2D;
	class VKSwapchain : public Swapchain {
		friend class VulkanGI;
	public:
		inline static VKSwapchain* Cast(Swapchain* pchain) {
			return (VKSwapchain*)pchain;
		}
	public:
		VKSwapchain(VkDevice logicDevice) {
			mLogicDevice = logicDevice;
			//
			VkSemaphoreCreateInfo semaphoreCreateInfo = {};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreCreateInfo.pNext = nullptr;
			semaphoreCreateInfo.flags = 0;
			vkCreateSemaphore(mLogicDevice, &semaphoreCreateInfo, nullptr, &mPresentSemaphore);
		}

		inline Format GetColorFormat() override {
			return (Format)mVkSurfaceFormat.format;
		}

		inline Format GetDepthStencilFormat() override {
			return (Format)mDepthStencilFormat;
		}

		inline uint32_t GetNumAttachment() override {
			return mColorAttachments.size();
		}

		inline uint32_t GetCurrentAttachmentIndex() {
			return mCurrentAttachmentIndex;
		}

		inline uint32_t AcquireNextAttachment() override {
			vkAcquireNextImageKHR(mLogicDevice, mVkSwapchain, UINT64_MAX, mPresentSemaphore, nullptr, &mCurrentAttachmentIndex);
			return mCurrentAttachmentIndex;
		}

		inline Image2D* GetColorAttachment(uint32_t index) override {
			if (index >= mColorAttachments.size()) {
				return nullptr;
			}
			return (Image2D*)mColorAttachments[index];
		}

		inline Image2D* GetDepthStencilAttachment(uint32_t index) override {
			if (index >= mDepthStencilAttachments.size()) {
				return nullptr;
			}
			return (Image2D*)mDepthStencilAttachments[index];
		}

		inline Extent2D GetExtent() override {
			return mExtent;
		}

		inline VkSemaphore& GetPresentSemaphore() {
			return mPresentSemaphore;
		}
	private:
		VkDevice mLogicDevice;
		VkSurfaceKHR mVkSurface = nullptr;
		VkSurfaceCapabilitiesKHR mVkSurfaceCapabilities;
		VkSurfaceFormatKHR mVkSurfaceFormat;
		VkFormat mDepthStencilFormat;
		VkPresentModeKHR mVkPresentMode;
		VkSwapchainKHR mVkSwapchain = nullptr;
		VkSemaphore mPresentSemaphore;
		std::vector<VKImage2D*> mColorAttachments;
		std::vector<VKImage2D*> mDepthStencilAttachments;
		Extent2D mExtent;
		uint32_t mCurrentAttachmentIndex;
	};

	class VKRenderPass : public RenderPass {
		friend class VulkanGI;
	public:
		inline static VKRenderPass* Cast(RenderPass* ppass) {
			return (VKRenderPass*)ppass;
		}
	public:
		inline VkRenderPass GetRenderPass() {
			return mVkRenderPass;
		}
	private:
		VkRenderPass mVkRenderPass;
	};

	class VKGraphicsPipeline : public GraphicsPipeline {
		friend class VulkanGI;
	public:
		inline static VKGraphicsPipeline* Cast(GraphicsPipeline* ppipeline) {
			return (VKGraphicsPipeline*)ppipeline;
		}
	public:
		inline GPUProgram* GetGPUProgram() override {
			return mGPUProgram;
		}
		//
		inline VkPipeline GetVKPipleline() {
			return mVkPipeLine;
		}

		inline VkPipelineLayout GetPipelineLayout() {
			return mVkPipelineLayout;
		}
	private:
		VkPipelineLayout mVkPipelineLayout;
		VkPipeline mVkPipeLine;
		gpu_program_ptr mGPUProgram;
	};

	class VKComputePipeline : public ComputePipeline {
		friend class VulkanGI;
	public:
		inline static VKComputePipeline* Cast(ComputePipeline* ppipeline) {
			return (VKComputePipeline*)ppipeline;
		}
	public:
		inline GPUProgram* GetGPUProgram() override {
			return mGPUProgram;
		}
		//
		inline VkPipeline GetVKPiipeline() {
			return mVkPipeline;
		}

		inline VkPipelineLayout GetPipelineLayout() {
			return mVkPipelineLayout;
		}
	private:
		VkPipelineLayout mVkPipelineLayout;
		VkPipeline mVkPipeline;
		gpu_program_ptr mGPUProgram;
	};

	class VKBuffer : public Buffer{
		friend class VulkanGI;
	public:
		inline static VKBuffer* Cast(Buffer* pbuf) {
			return (VKBuffer*)pbuf;
		}
	public:
		~VKBuffer() {
			VKMemoryManager::Instance()->DestoryBuffer(mVkBuffer, mMemory);
		}

		inline VkBuffer GetVKBuffer() {
			return mVkBuffer;
		}
	protected:
		VKBuffer(BufferUsageFlags usageFlags, uint64_t size) : Buffer(usageFlags, size) {
			mMemory = nullptr;
		}
	protected:
		VkBuffer mVkBuffer;
		VKMemory* mMemory;
	};

	class VKBufferUpdateContext : public BufferUpdateContext {
		friend class VulkanGI;
	public:
		inline static VKBufferUpdateContext* Cast(BufferUpdateContext* pcontext) {
			return (VKBufferUpdateContext*)pcontext;
		}
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

	
	class VKImageView;
	class VKImage {
		friend class VulkanGI;
	public:
		~VKImage() {
			VKMemoryManager::Instance()->DestoryImage(mVkImage, mMemory);
		}
		VKImage2D* asVKImage2D() { return nullptr; }
	protected:
		VKImage(){
			mMemory = nullptr;
		}
	protected:
	    VkImage mVkImage;
		ImageUsageFlags mUsageFlag;
		VKMemory* mMemory;
		VKImageView* mOrgiView;
	};

	class VKImageView : public ImageView {
		friend class VulkanGI;
	public:
		inline static VKImageView* Cast(ImageView* pview) {
			return (VKImageView*)pview;
		}
	public:
		VKImageView(Image* pimg, VkImageView pview, VkImageViewCreateInfo viewInfo) {
			mSrcImage = pimg;
			mImageView = pview;
			mViewInfo = viewInfo;
		}

		~VKImageView() {
			VKMemoryManager::Instance()->DestoryImageView(mImageView);
		}

		Image* GetSrcImage() override {
			if (mSrcImage->asImage2D() != nullptr) {
				return (Image*)mSrcImage->asImage2D();
			}
		}
	private:
		image_ptr mSrcImage;
		VkImageView mImageView;
		VkImageViewCreateInfo mViewInfo;
	};

	class VKImage2D : public VKImage, public Image2D {
		friend class VulkanGI;
	public:
		inline static VKImage2D* Cast(Image2D* pimg) {
			return (VKImage2D*)pimg;
		}
	public:
		VKImage2D(Format format, uint32_t sizeX, uint32_t sizeY, uint32_t numMip) : Image2D(format, sizeX, sizeY, numMip) {
			mLayoutBarrier.resize(numMip, VKImageLayoutBarrier::Undefined);
		}
		//
		ImageView* GetOrigView() override {
			return mOrgiView;
		}
	private:
		std::vector<VKImageLayoutBarrier> mLayoutBarrier;
	};

	class VKImageUpdateContext : public ImageUpdateContext {
		friend class VulkanGI;
	public:
		inline static VKImageUpdateContext* Cast(ImageUpdateContext* pcontext) {
			return (VKImageUpdateContext*)pcontext;
		}
	private:

	};

	class VKFrameBuffer : public FrameBuffer {
		friend class VulkanGI;
	public:
		inline static VKFrameBuffer* Cast(FrameBuffer* frameBuffer) {
			return (VKFrameBuffer*)frameBuffer;
		}
	public:
		inline Extent2D GetExtent() override {
			return mExtent;
		}

		inline uint32_t GetNumAttachment() override {
			return mAttachments.size();
		}

		inline ImageView* GetAttachment(uint32_t index) override {
			return mAttachments[index];
		}

		inline ClearValue GetClearValue(uint32_t index) override {
			return mClearValue[index];
		}

		inline void SetClearValue(uint32_t index, ClearValue clearValue) override{
			mClearValue[index] = clearValue;
		}
	public:
		VKFrameBuffer() {}
		VKFrameBuffer(VkFramebuffer frameBuffer, Extent2D extent) {
			mFrameBuffer = frameBuffer;
			mExtent = extent;
		}

		inline uint32_t GetWidth() {
			return mExtent.width;
		}

		inline uint32_t GetHeight() {
			return mExtent.height;
		}

		inline VkFramebuffer GetFrameBuffer() {
			return mFrameBuffer;
		}

		inline void AddAttachment(ImageView* attachment, const ClearValue& clearValue) {
			mAttachments.push_back(attachment);
			mClearValue.push_back(clearValue);
		}
	private:
		VkFramebuffer mFrameBuffer;
		std::vector<image_view_ptr> mAttachments;
		std::vector<ClearValue> mClearValue;
		Extent2D mExtent;
	};
}
