#pragma once
#include <iostream>
#include "GraphicWindow.h"
#include "..\ASGI\ASGI.h"

#include "gli\gli\gli.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "..\glm\glm.hpp"
#include "..\glm\gtc\matrix_transform.hpp"

#define FREEIMAGE_LIB
#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "mng.lib")
#pragma comment(lib, "openexr.lib")
#pragma comment(lib, "openjpeg.lib")
#pragma comment(lib, "png.lib")
#pragma comment(lib, "rawlite.lib")
#pragma comment(lib, "tiff.lib")
#pragma comment(lib, "zlib.lib")
#include "FreeImage\FreeImage.h"
#include "FreeImage\Utilities.h"
#pragma comment(lib, "freeimage.lib")


class GITest : public GraphicWindow {
public:
	bool PrepareRender() override {

		ASGI::SwapchainCreateInfo swapchain_create_info = {
			GetHWND(),
			800,
			600,
			false,
			ASGI::Format::FORMAT_B8G8R8A8_UNORM,
			ASGI::Format::FORMAT_D24_UNORM_S8_UINT,
			false
		};

		pGraphicsContext = ASGI::CreateContext(ASGI::GIType::GI_VULKAN, &swapchain_create_info, "GeForce GTX 850M");
		if (pGraphicsContext == nullptr) {
			return false;
		}
		pSwapchain = pGraphicsContext->GetSwapchain();
		//
		//load shader
		auto pVSModule = ASGI::CreateShaderModule("Z:\\ASGI\\debug\\test.vert");
		auto pFGModule = ASGI::CreateShaderModule("Z:\\ASGI\\debug\\test.frag");
		//
		pGPUProgram = ASGI::CreateShaderProgram(pVSModule, nullptr, nullptr, nullptr, pFGModule);
		//create render pass
		std::vector<ASGI::AttachmentDescription> attachments(2);
		// Color attachment
		attachments[0].format = pSwapchain->GetColorFormat();
		attachments[0].samples = ASGI::SampleCountFlagBits::SAMPLE_COUNT_1_BIT;									// We don't use multi sampling in this example
		attachments[0].loadOp = ASGI::AttachmentLoadOp::ATTACHMENT_LOAD_OP_CLEAR;							// Clear this attachment at the start of the render pass
		attachments[0].storeOp = ASGI::AttachmentStoreOp::ATTACHMENT_STORE_OP_STORE;							// Keep it's contents after the render pass is finished (for displaying it)
		attachments[0].stencilLoadOp = ASGI::AttachmentLoadOp::ATTACHMENT_LOAD_OP_DONT_CARE;					// We don't use stencil, so don't care for load
		attachments[0].stencilStoreOp = ASGI::AttachmentStoreOp::ATTACHMENT_STORE_OP_DONT_CARE;				// Same for store
		attachments[0].initialLayout = ASGI::ImageLayout::IMAGE_LAYOUT_UNDEFINED;						// Layout at render pass start. Initial doesn't matter, so we use undefined
		attachments[0].finalLayout = ASGI::ImageLayout::IMAGE_LAYOUT_PRESENT_SRC_KHR;					// Layout to which the attachment is transitioned when the render pass is finished
																						// As we want to present the color buffer to the swapchain, we transition to PRESENT_KHR	
																						// Depth attachment
		attachments[1].format = ASGI::Format::FORMAT_D24_UNORM_S8_UINT;
		attachments[1].samples = ASGI::SampleCountFlagBits::SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = ASGI::AttachmentLoadOp::ATTACHMENT_LOAD_OP_CLEAR;							// Clear depth at start of first subpass
		attachments[1].storeOp = ASGI::AttachmentStoreOp::ATTACHMENT_STORE_OP_DONT_CARE;						// We don't need depth after render pass has finished (DONT_CARE may result in better performance)
		attachments[1].stencilLoadOp = ASGI::AttachmentLoadOp::ATTACHMENT_LOAD_OP_DONT_CARE;					// No stencil
		attachments[1].stencilStoreOp = ASGI::AttachmentStoreOp::ATTACHMENT_STORE_OP_DONT_CARE;				// No Stencil
		attachments[1].initialLayout = ASGI::ImageLayout::IMAGE_LAYOUT_UNDEFINED;						// Layout at render pass start. Initial doesn't matter, so we use undefined
		attachments[1].finalLayout = ASGI::ImageLayout::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	// Transition to depth/stencil attachment
		// Setup attachment references
		std::vector<ASGI::AttachmentReference> colorReferences(1);																		
		colorReferences[0].attachment = 0;													// Attachment 0 is color
		colorReferences[0].layout = ASGI::ImageLayout::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;				// Attachment layout used as color during the subpass
		//
		ASGI::AttachmentReference depthReference;
		depthReference.attachment = 1;													// Attachment 1 is color
		depthReference.layout = ASGI::ImageLayout::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;		// Attachment used as depth/stemcil used during the subpass
		//
		std::vector<ASGI::SubpassDescription> subpassDescriptions(1);								// Subpass uses one color attachment
		subpassDescriptions[0].colorAttachments = colorReferences;					// Reference to the color attachment in slot 0
		subpassDescriptions[0].depthStencilAttachment = depthReference;				// Reference to the depth attachment in slot 1
		//
		ASGI::RenderPassCreateInfo renderPassCreateInfo;
		renderPassCreateInfo.attachments = std::move(attachments);
		renderPassCreateInfo.subpasses = std::move(subpassDescriptions);
		pRenderPass = ASGI::CreateRenderPass(renderPassCreateInfo);
		//create pipeline
		ASGI::GraphicsPipelineCreateInfo graphics_pipeline_create_info;
		graphics_pipeline_create_info.vertexDeclaration.vertexInputs.push_back({
			0,0,ASGI::VertexFormat::VF_Float3,0});
		graphics_pipeline_create_info.vertexDeclaration.vertexInputs.push_back({
			0,3 * sizeof(float),ASGI::VertexFormat::VF_Float3,1 });
		graphics_pipeline_create_info.vertexDeclaration.vertexInputs.push_back({
			0,6 * sizeof(float),ASGI::VertexFormat::VF_Float2, 2 });
		graphics_pipeline_create_info.shaderProgram = pGPUProgram;
		graphics_pipeline_create_info.colorBlendState.Attachments.push_back(ASGI::PipelineColorBlendAttachmentState());
		graphics_pipeline_create_info.renderPass = pRenderPass;
		graphics_pipeline_create_info.subpassIndex = 0;
		pGraphicsPipeline = ASGI::CreateGraphicsPipeline(graphics_pipeline_create_info);
		//create frame buffer
		ASGI::Extent2D extent = pSwapchain->GetExtent();
		for (int i = 0; i < pSwapchain->GetNumAttachment(); ++i) {
			std::vector<ASGI::ImageView*> attachMents(2);
			attachMents[0] = pSwapchain->GetColorAttachment(i)->GetOrigView();
			attachMents[1] = pSwapchain->GetDepthStencilAttachment(i)->GetOrigView();
			std::vector<ASGI::ClearValue> clearValues(2);
			clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearValues[1].depthStencil = { 1.0f, 0 };
			auto pFrameBuffer = ASGI::CreateFrameBuffer(pRenderPass, 2, attachMents.data(), clearValues.data(), extent.width, extent.height);
			if (pFrameBuffer == nullptr) {
				return false;
			}
			//
			frameBuffers.push_back(pFrameBuffer);
		}
		//
		struct Vertex
		{
			float position[3];
			float color[3];
			float uv[2];
		};

		// Setup vertices
		std::vector<Vertex> vertexBuffer =
		{
			{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },{1.0,1.0} },
			{ { -1.0f,  1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },{0.0,1.0} },
			{ { 0.0f, -1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },{0.5,0} }
		};
		uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);
		// Setup indices
		std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
		uint32_t indexBufferSize =3* sizeof(uint32_t);
		//
		auto bufferUpdateContex = ASGI::BeginUpdateBuffer();
		pVertexBuffer = ASGI::CreateBuffer(vertexBufferSize, ASGI::BufferUsageFlagBits::BUFFER_USAGE_VERTEX_BIT | ASGI::BufferUsageFlagBits::BUFFER_USAGE_TRANSFER_DST_BIT);
		ASGI::UpdateBuffer(pVertexBuffer, 0, vertexBufferSize, vertexBuffer.data(), bufferUpdateContex);

		pIndexBuffer = ASGI::CreateBuffer(indexBufferSize, ASGI::BufferUsageFlagBits::BUFFER_USAGE_INDEX_BIT | ASGI::BufferUsageFlagBits::BUFFER_USAGE_TRANSFER_DST_BIT);
		ASGI::UpdateBuffer(pIndexBuffer, 0, indexBufferSize, indexBuffer.data(), bufferUpdateContex);


		struct {
			glm::mat4 projectionMatrix;
			glm::mat4 modelMatrix;
			glm::mat4 viewMatrix;
		} uboVS;
		uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 256.0f);
		uboVS.viewMatrix = glm::lookAt(glm::vec3(0, 0, -2.5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		pUniformBuffer = ASGI::CreateBuffer(sizeof(uboVS), ASGI::BufferUsageFlagBits::BUFFER_USAGE_UNIFORM_BIT | ASGI::BufferUsageFlagBits::BUFFER_USAGE_TRANSFER_DST_BIT);
		ASGI::UpdateBuffer(pUniformBuffer, 0, sizeof(uboVS), &uboVS, bufferUpdateContex);
		ASGI::EndUpdateBuffer(bufferUpdateContex);

		ASGI::BindUniformBuffer(pGPUProgram, 0, 0, pUniformBuffer, 0, sizeof(uboVS));
		//
		FreeImage_Initialise();
		auto pimg = FreeImage_Load(FREE_IMAGE_FORMAT::FIF_JPEG, "G:\\picture\\girl1.jpg");
		auto imgWidth = FreeImage_GetWidth(pimg);
		auto imgHeight = FreeImage_GetHeight(pimg);
		auto pimg1 = FreeImage_ConvertTo32Bits(pimg);
		auto redMask = FreeImage_GetRedMask(pimg1);
		FreeImage_Save(FREE_IMAGE_FORMAT::FIF_PNG, pimg1, "G:\\picture\\girl11.png");
		auto pdata = FreeImage_GetBits(pimg1);
		pImage = ASGI::CreateImage2D(imgWidth, imgHeight, ASGI::Format::FORMAT_R8G8B8A8_UNORM, 1,
			ASGI::SampleCountFlagBits::SAMPLE_COUNT_1_BIT, 
			ASGI::ImageUsageFlagBits::IMAGE_USAGE_SAMPLED_BIT | ASGI::ImageUsageFlagBits::IMAGE_USAGE_TRANSFER_DST_BIT | ASGI::ImageUsageFlagBits::IMAGE_USAGE_TRANSFER_SRC_BIT);
		ASGI::UpdateImage2D(pImage, 0, 0, 0, imgWidth, imgHeight, pdata);
		pSampler = ASGI::CreateSampler();
		ASGI::BindTexture(pGPUProgram, 0, 1, pImage->GetOrigView(), pSampler);
		//
		int attachmentCount = pSwapchain->GetNumAttachment();
		for (int i = 0; i < attachmentCount; ++i) {
			auto pCmdBuffer = ASGI::CreateCmdBuffer();
			//
			ASGI::BeginCmdBuffer(pCmdBuffer);
			ASGI::BeginRenderPass(pCmdBuffer, pRenderPass, frameBuffers[i]);
			// Update dynamic viewport state
			ASGI::Viewport viewport = {};
			viewport.height = (float)height;
			viewport.width = (float)width;
			viewport.minDepth = (float) 0.0f;
			viewport.maxDepth = (float) 1.0f;
			ASGI::CmdSetViewport(pCmdBuffer, 0, 1, &viewport);

			// Update dynamic scissor state
			ASGI::Rect2D scissor = {};
			scissor.extent.width = width;
			scissor.extent.height = height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			ASGI::CmdSetScissor(pCmdBuffer, 0, 1, &scissor);

			ASGI::CmdBindPipeline(pCmdBuffer, pGraphicsPipeline);
			ASGI::CmdBindVertexBuffer(pCmdBuffer, 0, pVertexBuffer, 0);
			ASGI::CmdBindIndexBuffer(pCmdBuffer, pIndexBuffer, 0, ASGI::Format::FORMAT_R32_UINT);
			ASGI::CmdDrawIndexed(pCmdBuffer, 3, 1, 0, 0, 1);
			//
			ASGI::EndRenderPass(pCmdBuffer, pRenderPass, 0, nullptr);
			ASGI::EndCmdBuffer(pCmdBuffer);
			//
			cmdBuffers.push_back(pCmdBuffer);
		}
		return true;
	}

	void Render() override {
		auto attachmentIndex = pSwapchain->AcquireNextAttachment();
		auto pCmdBuffer = cmdBuffers[attachmentIndex];
		//
		ASGI::BeginCmdBuffer(pCmdBuffer);
		ASGI::BeginRenderPass(pCmdBuffer, pRenderPass, frameBuffers[attachmentIndex]);
		// Update dynamic viewport state
		ASGI::Viewport viewport = {};
		viewport.height = (float)height;
		viewport.width = (float)width;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		ASGI::CmdSetViewport(pCmdBuffer, 0, 1, &viewport);

		// Update dynamic scissor state
		ASGI::Rect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		ASGI::CmdSetScissor(pCmdBuffer, 0, 1, &scissor);

		ASGI::CmdBindPipeline(pCmdBuffer, pGraphicsPipeline);
		ASGI::CmdBindVertexBuffer(pCmdBuffer, 0, pVertexBuffer, 0);
		ASGI::CmdBindIndexBuffer(pCmdBuffer, pIndexBuffer, 0, ASGI::Format::FORMAT_R32_UINT);
		ASGI::CmdDrawIndexed(pCmdBuffer, 3, 1, 0, 0, 1);
		//
		ASGI::EndRenderPass(pCmdBuffer, pRenderPass, 0, nullptr);
		ASGI::EndCmdBuffer(pCmdBuffer);
		//
		auto cmdBuffer = pCmdBuffer.get();
		auto swapChain = pSwapchain;
		//
		pqueue = ASGI::AcquireExcuteQueue(ASGI::QueueType::QUEUE_TYPE_GRAPHICS);
		ASGI::SubmitCommands(pqueue, 1, &cmdBuffer, 0, nullptr, 1, &swapChain);
		ASGI::Present(pqueue, 1, &swapChain);
		ASGI::WaitQueueExcuteFinished(1, &pqueue);
	}
private:
	std::string readfile(char *path)
	{
		FILE *fp;
		if ((fp = fopen(path, "rb")) == NULL) {
			return"";
		}
		int fileSize;
		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		std::vector<char> buffer(fileSize);
		fread(buffer.data(), fileSize, 1, fp);
		fclose(fp);
		//
		return std::string(buffer.data(), buffer.size());
	}
	private:
		ASGI::graphics_context_ptr pGraphicsContext = nullptr;
		ASGI::Swapchain* pSwapchain = nullptr;
		ASGI::shader_program_ptr pGPUProgram = nullptr;
		ASGI::graphics_pipeline_ptr pGraphicsPipeline = nullptr;
		ASGI::render_pass_ptr pRenderPass;
		std::vector<ASGI::frame_buffer_ptr> frameBuffers;
		//ASGI::command_buffer_ptr pCmdBuffer;
		std::vector<ASGI::command_buffer_ptr> cmdBuffers;
		ASGI::ExcuteQueue* pqueue;

		ASGI::buffer_ptr pVertexBuffer;
		ASGI::buffer_ptr pIndexBuffer;
		ASGI::buffer_ptr pUniformBuffer;

		ASGI::image_2d_ptr pImage;
		ASGI::sampler_ptr pSampler;
};