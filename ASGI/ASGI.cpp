// ASGI.cpp : 定义 DLL 应用程序的导出函数。
//
#include "ASGI.h"
#include "VkGLSL.hpp"
#include "VulkanGI.h"
#include <io.h>
#include  <stdio.h>
#include <thread>
#include <mutex>
#include <unordered_map>

namespace ASGI {
	bool GSupportParallelCommandBuffer = false;
	uint8_t GetFormatSize(Format format) {
		switch (format){
		case Format::FORMAT_R32G32B32A32_SFLOAT:
		case Format::FORMAT_R32G32B32A32_SINT:
		case Format::FORMAT_R32G32B32A32_UINT:
			return 16;
		case Format::FORMAT_R32G32B32_SFLOAT:
		case Format::FORMAT_R32G32B32_SINT:
		case Format::FORMAT_R32G32B32_UINT:
			return 12;
		case Format::FORMAT_R32G32_SFLOAT:
		case Format::FORMAT_R32G32_SINT:
		case Format::FORMAT_R32G32_UINT:
		case Format::FORMAT_R16G16B16A16_SFLOAT:
		case Format::FORMAT_R16G16B16A16_SINT:
		case Format::FORMAT_R16G16B16A16_SNORM:
		case Format::FORMAT_R16G16B16A16_SSCALED:
		case Format::FORMAT_R16G16B16A16_UINT:
		case Format::FORMAT_R16G16B16A16_UNORM:
		case Format::FORMAT_R16G16B16A16_USCALED:
			return 8;
		case Format::FORMAT_R32_SFLOAT:
		case Format::FORMAT_R32_SINT:
		case Format::FORMAT_R32_UINT:
		case Format::FORMAT_R16G16_SFLOAT:
		case Format::FORMAT_R16G16_SINT:
		case Format::FORMAT_R16G16_SNORM:
		case Format::FORMAT_R16G16_SSCALED:
		case Format::FORMAT_R16G16_UINT:
		case Format::FORMAT_R16G16_UNORM:
		case Format::FORMAT_R16G16_USCALED:
		case Format::FORMAT_R8G8B8A8_SINT:
		case Format::FORMAT_R8G8B8A8_SNORM:
		case Format::FORMAT_R8G8B8A8_SRGB:
		case Format::FORMAT_R8G8B8A8_SSCALED:
		case Format::FORMAT_R8G8B8A8_UINT:
		case Format::FORMAT_R8G8B8A8_UNORM:
		case Format::FORMAT_R8G8B8A8_USCALED:
		case Format::FORMAT_B8G8R8A8_SINT:
		case Format::FORMAT_B8G8R8A8_SNORM:
		case Format::FORMAT_B8G8R8A8_SRGB:
		case Format::FORMAT_B8G8R8A8_SSCALED:
		case Format::FORMAT_B8G8R8A8_UINT:
		case Format::FORMAT_B8G8R8A8_UNORM:
		case Format::FORMAT_B8G8R8A8_USCALED:
			return 4;
		case Format::FORMAT_R8G8B8_SINT:
		case Format::FORMAT_R8G8B8_SNORM:
		case Format::FORMAT_R8G8B8_SRGB:
		case Format::FORMAT_R8G8B8_SSCALED:
		case Format::FORMAT_R8G8B8_UINT:
		case Format::FORMAT_R8G8B8_UNORM:
		case Format::FORMAT_R8G8B8_USCALED:
		case Format::FORMAT_B8G8R8_SINT:
		case Format::FORMAT_B8G8R8_SNORM:
		case Format::FORMAT_B8G8R8_SRGB:
		case Format::FORMAT_B8G8R8_SSCALED:
		case Format::FORMAT_B8G8R8_UINT:
		case Format::FORMAT_B8G8R8_UNORM:
		case Format::FORMAT_B8G8R8_USCALED:
			return 3;
		case Format::FORMAT_R16_SFLOAT:
		case Format::FORMAT_R16_SINT:
		case Format::FORMAT_R16_SNORM:
		case Format::FORMAT_R16_SSCALED:
		case Format::FORMAT_R16_UINT:
		case Format::FORMAT_R16_UNORM:
		case Format::FORMAT_R16_USCALED:
		case Format::FORMAT_R8G8_UNORM:
		case Format::FORMAT_R8G8_SNORM:
		case Format::FORMAT_R8G8_USCALED:
		case Format::FORMAT_R8G8_SSCALED:
		case Format::FORMAT_R8G8_UINT:
		case Format::FORMAT_R8G8_SINT:
		case Format::FORMAT_R8G8_SRGB:
			return 2;
		case Format::FORMAT_R8_UNORM:
		case Format::FORMAT_R8_SNORM:
		case Format::FORMAT_R8_USCALED:
		case Format::FORMAT_R8_SSCALED:
		case Format::FORMAT_R8_UINT:
		case Format::FORMAT_R8_SINT:
		case Format::FORMAT_R8_SRGB:
			return 1;
		default :
			return 0;
		}
	}

	DynamicGI* pGI = nullptr;
	class GraphicsContextManager {
	public:
		GraphicsContext* GetCurrentContext(long long threadId) {
			std::lock_guard<std::mutex> lock(mMutexContext);
			//
			std::this_thread::get_id();
			auto itr_thread = mThreadContexts.find(threadId);
			if (itr_thread == mThreadContexts.end() || itr_thread->second.size() == 0) {
				return nullptr;
			}
			auto itr_current = mThreadCurrentContext.find(threadId);
			if (itr_current == mThreadCurrentContext.end()) {
				return itr_thread->second[0];
			}
			else {
				return itr_thread->second[itr_current->second];
			}
		}
		//

	private:
		std::unordered_map<long long, std::vector<graphics_context_ptr> > mThreadContexts;
		std::unordered_map<long long, int> mThreadCurrentContext;
		std::unordered_map<long long, DynamicGI*> mContextGIs;
		//
		std::mutex mMutexContext;
	};

	graphics_context_ptr CreateContext(GIType driver, SwapchainCreateInfo* swapchainInfo, const char* device_name) {
		if (driver == GIType::GI_VULKAN) {
			pGI = new VulkanGI();
			if (pGI && pGI->Init(device_name)) {
				Swapchain* pSwapchain = nullptr;
				if (swapchainInfo != nullptr) {
					pSwapchain = pGI->CreateSwapchain(*swapchainInfo);
					if (pSwapchain == nullptr) {
						delete pGI;
						pGI = nullptr;
						return nullptr;
					}
					//
					VKContext* pres = new VKContext(pSwapchain);
					return pres;
				}
			}
			else {
				return nullptr;
			}
		}
		//
		return nullptr;
	}

	void SetCurrentContext(GraphicsContext* context) {

	}

	bool Init(GIType driver, const char* device_name) {
		if (driver == GIType::GI_VULKAN) {
			pGI = new VulkanGI();
			if (pGI) {
				return pGI->Init(device_name);
			}
			else {
				return false;
			}
		}
		return false;
	}

	shader_module_ptr CreateShaderModule(const char* shaderPath) {
		if (_access(shaderPath, 0) != 0) {
			return nullptr;
		}
		//
		std::string spirvPath = std::string(shaderPath) + ".spv";
		std::string spirvCode;
		FILE* pfile = fopen(spirvPath.c_str(), "rb");
		if (pfile == nullptr) {
#ifdef  _WIN32
			char szFilePath[MAX_PATH + 1] = { 0 };
			GetModuleFileName(NULL, szFilePath, MAX_PATH);
			strrchr(szFilePath, '\\')[1] = '\0';
			std::string glslangPath = std::string(szFilePath) + "glslangValidator.exe";
			//
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(&pi, sizeof(pi));
			std::string commandLine = glslangPath + " -V " + std::string(shaderPath) + " -o " + spirvPath;
			if (CreateProcess(NULL, (LPSTR)commandLine.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
				WaitForSingleObject(pi.hProcess, INFINITE);
				CloseHandle(pi.hProcess);
			}
#endif //  _WIN32
			//
			pfile = fopen(spirvPath.c_str(), "rb");
			if (pfile == nullptr) {
				return nullptr;
			}
		}
		//
		fclose(pfile);
		//
		ShaderModuleCreateInfo create_info_spirv;
		create_info_spirv.path = spirvPath.c_str();
		return pGI->CreateShaderModule(create_info_spirv);
	}

	shader_program_ptr CreateShaderProgram(ShaderModule* pVertexShader, ShaderModule* pGeomteryShader, ShaderModule* pTessControlShader, ShaderModule* pTessEvaluationShader, ShaderModule* pFragmentShader) {
		return pGI->CreateShaderProgram(pVertexShader, pGeomteryShader, pTessControlShader, pTessEvaluationShader, pFragmentShader);
	}

	render_pass_ptr CreateRenderPass(const RenderPassCreateInfo& create_info) {
		return pGI->CreateRenderPass(create_info);
	}

	graphics_pipeline_ptr CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& create_info) {
		return pGI->CreateGraphicsPipeline(create_info);
	}

	swapchain_ptr CreateSwapchain(const SwapchainCreateInfo& create_info) {
		return pGI->CreateSwapchain(create_info);
	}

	frame_buffer_ptr CreateFrameBuffer(RenderPass* targetRenderPass, uint8_t numAttachment, ImageView** attachments, ClearValue* clearValues, uint32_t width, uint32_t height) {
		return pGI->CreateFrameBuffer(targetRenderPass, numAttachment, attachments, clearValues, width, height);
	}

	buffer_ptr CreateBuffer(uint64_t size, BufferUsageFlags usageFlags) {
		return pGI->CreateBuffer(size, usageFlags);
	}

	buffer_update_contex_ptr BeginUpdateBuffer() {
		return pGI->BeginUpdateBuffer();
	}

	bool EndUpdateBuffer(BufferUpdateContext* pUpdateContext) {
		return pGI->EndUpdateBuffer(pUpdateContext);
	}

	void UpdateBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext) {
		pGI->UpdateBuffer(pbuffer, offset, size, pdata, pUpdateContext);
	}

	void* MapBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode) {
		return nullptr;
	}

	void UnMapBuffer(Buffer* pbuffer) {
	}

	void BindUniformBuffer(ShaderProgram* pProgram, uint8_t setIndex, uint32_t bindingIndex, Buffer* pbuffer, uint32_t offset, uint32_t size) {
		pGI->BindUniformBuffer(pProgram, setIndex, bindingIndex, pbuffer, offset, size);
	}

	image_2d_ptr CreateImage2D(uint32_t sizeX, uint32_t sizeY, Format format, uint32_t numMips, SampleCountFlagBits samples, ImageUsageFlags usageFlags) {
		return pGI->CreateImage2D(sizeX, sizeY, format, numMips, samples, usageFlags);
	}

	image_view_ptr CreateImageView(Image2D* srcImage, uint32_t mipLevel) {
		return pGI->CreateImageView(srcImage, mipLevel);
	}

	image_view_ptr CreateImageView(Image2D* srcImage, uint32_t mipLevel, uint32_t numMipLevels, Format format) {
		return pGI->CreateImageView(srcImage, mipLevel, numMipLevels, format);
	}

	sampler_ptr CreateSampler(float minLod, float maxLod, float  mipLodBias,
		Filter magFilter, Filter minFilter,
		SamplerMipmapMode mipmapMode,
		SamplerAddressMode addressModeU,
		SamplerAddressMode addressModeV,
		SamplerAddressMode addressModeW,
		float  maxAnisotropy, CompareOp compareOp,
		BorderColor borderColor) {
		return pGI->CreateSampler(minLod, maxLod, mipLodBias, magFilter, minFilter, mipmapMode, addressModeU, addressModeV, addressModeW, maxAnisotropy, compareOp, borderColor);
	}

	void BindTexture(ShaderProgram* pProgram, uint8_t setIndex, uint32_t bindingIndex, ImageView* pImgView, Sampler* pSampler) {
		pGI->BindTexture(pProgram, setIndex, bindingIndex, pImgView, pSampler);
	}

	image_update_context_ptr BeginUpdateImage() {
		return pGI->BeginUpdateImage();
	}

	bool EndUpdateImage(ImageUpdateContext* pUpdateContext) {
		return pGI->EndUpdateImage(pUpdateContext);
	}

	bool UpdateImage2D(Image2D* pimg, uint32_t level, uint32_t offsetX, uint32_t offsetY, uint32_t sizeX, uint32_t sizeY, void* pdata, ImageUpdateContext* pUpdateContext) {
		return pGI->UpdateImage2D(pimg, level, offsetX, offsetY, sizeX, sizeY, pdata, pUpdateContext);
	}

	ExcuteQueue* AcquireExcuteQueue(QueueType queueType) {
		return pGI->AcquireExcuteQueue(queueType);
	}

	void WaitQueueExcuteFinished(uint32_t numWaiteQueue, ExcuteQueue** excuteQueues) {
		pGI->WaitQueueExcuteFinished(numWaiteQueue, excuteQueues);
	}

	bool SubmitCommands(ExcuteQueue* excuteQueue, uint32_t numBuffers, CommandBuffer** cmdBuffers, uint32_t numWaiteQueue, ExcuteQueue** waiteQueues, uint32_t numSwapchain, Swapchain** waiteSwapchains, bool waiteFinished) {
		return pGI->SubmitCommands(excuteQueue, numBuffers, cmdBuffers, numWaiteQueue, waiteQueues, numSwapchain, waiteSwapchains, waiteFinished);
	}

	void Present(ExcuteQueue* excuteQueue, uint32_t numSwapchain, Swapchain** swapchains, bool waiteFinished) {
		pGI->Present(excuteQueue, numSwapchain, swapchains, waiteFinished);
	}

	command_buffer_ptr CreateCmdBuffer(){
		return pGI->CreateCmdBuffer();
	}


	bool BeginRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, FrameBuffer* frameBuffer) {
		return pGI->BeginRenderPass(cmdBuffer, renderPass, frameBuffer);
	}

	void EndSubRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) {
		pGI->EndSubRenderPass(cmdBuffer, renderPass, numSecondCmdBuffer, secondCmdBuffers);
	}

	void EndRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) {
		pGI->EndRenderPass(cmdBuffer, renderPass, numSecondCmdBuffer, secondCmdBuffers);
	}

	bool BeginComputePass(CommandBuffer* cmdBuffer, ComputePass* computePass) {
		return pGI->BeginComputePass(cmdBuffer, computePass);
	}
	void EndComputePass(CommandBuffer* cmdBuffer, ComputePass* computePass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) {
		pGI->EndComputePass(cmdBuffer, computePass, numSecondCmdBuffer, secondCmdBuffers);
	}

	void CmdBindPipeline(CommandBuffer*  cmdBuffer, GraphicsPipeline* pipeline) {
		pGI->CmdBindPipeline(cmdBuffer, pipeline);
	}

	void CmdSetViewport(CommandBuffer*  cmdBuffer, uint32_t   firstViewport, uint32_t  viewportCount, Viewport*  pViewports) {
		pGI->CmdSetViewport(cmdBuffer, firstViewport, viewportCount, pViewports);
	}

	void CmdSetScissor(CommandBuffer*  cmdBuffer, uint32_t  firstScissor, uint32_t   scissorCount, Rect2D*  pScissors) {
		pGI->CmdSetScissor(cmdBuffer, firstScissor, scissorCount, pScissors);
	}

	void CmdSetLineWidth(CommandBuffer*  cmdBuffer, float   lineWidth) {
		pGI->CmdSetLineWidth(cmdBuffer, lineWidth);
	}

	void CmdBindIndexBuffer(CommandBuffer* cmdBuffer, Buffer* pBuffer, uint32_t offset, Format indexFormat) {
		pGI->CmdBindIndexBuffer(cmdBuffer, pBuffer, offset, indexFormat);
	}

	void CmdBindVertexBuffer(CommandBuffer* cmdBuffer, uint32_t  bindingIndex, Buffer*  pBuffer, uint32_t offset) {
		pGI->CmdBindVertexBuffer(cmdBuffer, bindingIndex, pBuffer, offset);
	}

	void CmdDraw(CommandBuffer* cmdBuffer, uint32_t vertexCount, uint32_t  instanceCount, uint32_t firstVertex, uint32_t  firstInstance) {
		pGI->CmdDraw(cmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CmdDrawIndexed(CommandBuffer* cmdBuffer, uint32_t indexCount, uint32_t   instanceCount, uint32_t  firstIndex, int32_t  vertexOffset, uint32_t  firstInstance) {
		pGI->CmdDrawIndexed(cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
}
