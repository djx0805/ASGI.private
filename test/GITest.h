#pragma once
#include "GraphicWindow.h"
#include "..\ASGI\ASGI.h"


class GITest : public GraphicWindow {
public:
	bool PrepareRender() override {
		//
		if (!ASGI::Init(ASGI::GIType::GI_VULKAN, "GeForce GTX 850M")) {
			return false;
		}

		ASGI::SwapchainCreateInfo swapchain_create_info = {
			GetHWND(),
			800,
			600,
			false,
			ASGI::Format::FORMAT_B8G8R8A8_UNORM,
			true
		};
		ASGI::Swapchain* pSwapchain = ASGI::CreateSwapchain(swapchain_create_info);
		if (pSwapchain == nullptr) {
			return false;
		}
		//
		auto vertexShaderCode = readfile("F:\\VulkanLearn\\Vulkan-master\\data\\shaders\\deferred\\deferred.vert");
		ASGI::ShaderModuleCreateInfo vertex_shader_module_create_info;
		vertex_shader_module_create_info.path = "F:\\VulkanLearn\\Vulkan-master\\data\\shaders\\deferred\\deferred.vert";
		auto pVSModule = ASGI::CreateShaderModule(vertex_shader_module_create_info);

		auto fragmentShaderCode = readfile("F:\\VulkanLearn\\Vulkan-master\\data\\shaders\\deferred\\deferred.frag");
		ASGI::ShaderModuleCreateInfo fragment_shader_module_create_info;
		fragment_shader_module_create_info.path = "F:\\VulkanLearn\\Vulkan-master\\data\\shaders\\deferred\\deferred.frag";
		auto pFGModule = ASGI::CreateShaderModule(fragment_shader_module_create_info);

		ASGI::GraphicsPipelineCreateInfo graphics_pipeline_create_info;
		graphics_pipeline_create_info.vertexDeclaration.vertexInputs.push_back({
			0,0,ASGI::VertexFormat::VF_Float3,0});
		graphics_pipeline_create_info.vertexDeclaration.vertexInputs.push_back({
			1,0,ASGI::VertexFormat::VF_Float2,1});
		graphics_pipeline_create_info.shaderStage.pVertexShader = pVSModule;
		graphics_pipeline_create_info.shaderStage.pFragmentShader = pFGModule;
		ASGI::CreateGraphicsPipeline(graphics_pipeline_create_info);
		//
		return true;
	}

	void Render() override;
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
};