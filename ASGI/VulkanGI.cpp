#include "VulkanResource.h"
#include "VulkanGI.h"

#include "third_lib\SPIRV-Cross\spirv_cross.hpp"

#ifdef _WIN32
#pragma comment(lib, "VulkanSDK\\1.1.77.0\\Lib\\vulkan-1.lib")
#endif

#include <iostream>

#include "GraphicsContextManager.h"

namespace ASGI {
	bool VulkanGI::getInstanceLevelExtensions() {
		uint32_t extensions_count = 0;
		VkResult res = VK_SUCCESS;

		res = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);
		if (res != VK_SUCCESS || extensions_count == 0) {
			return false;
		}
		//
		mVkInstanceExtensions.resize(extensions_count);
		res = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, &mVkInstanceExtensions[0]);
		if (res != VK_SUCCESS || extensions_count == 0) {
			return false;
		}
		//
		return true;
	}

	bool VulkanGI::createVKInstance(std::vector<char const *>& desired_extensions) {
		VkApplicationInfo application_info = {
			VK_STRUCTURE_TYPE_APPLICATION_INFO,
			nullptr,
			"vulkanGI",
			VK_MAKE_VERSION(1, 0, 0),
			"vulkanGI",
			VK_MAKE_VERSION(1, 0, 0),
			VK_MAKE_VERSION(1, 0, 0)
		};
		//
		const std::vector<const char*> validationLayers = {
			"VK_LAYER_LUNARG_standard_validation"
		};
		VkInstanceCreateInfo instance_create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			nullptr,
			0,
			&application_info,
			validationLayers.size(),
			validationLayers.size()>0 ? validationLayers.data() : nullptr,
			static_cast<uint32_t>(desired_extensions.size()),
			desired_extensions.size() > 0 ? &desired_extensions[0] : nullptr
		};
		//
		VkResult result = vkCreateInstance(&instance_create_info, nullptr, &mVkInstance);
		if ((result != VK_SUCCESS) || (mVkInstance == VK_NULL_HANDLE)) {
			return false;
		}
		//
		return true;
	}

	bool VulkanGI::createLogicDevice(const char* physic_device_name) {
		uint32_t devices_count = 0;
		VkResult result = VK_SUCCESS;
		result = vkEnumeratePhysicalDevices(mVkInstance, &devices_count, nullptr);
		if ((result != VK_SUCCESS) ||
			(devices_count == 0)) {
			return false;
		}
		//
		std::vector<VkPhysicalDevice> available_devices;
		available_devices.resize(devices_count);
		result = vkEnumeratePhysicalDevices(mVkInstance, &devices_count, &available_devices[0]);
		if ((result != VK_SUCCESS) ||
			(devices_count == 0)) {
			return false;
		}
		//
		for (int n = 0; n < available_devices.size(); ++n) {
			vkGetPhysicalDeviceProperties(available_devices[n], &mVkDeviceProperties);
			//
			mVkPhysicalDevice = available_devices[n];
			vkGetPhysicalDeviceFeatures(available_devices[n], &mVkDeviceFeatures);
			//
			if (physic_device_name == nullptr || std::string(mVkDeviceProperties.deviceName) == std::string(physic_device_name)) {
				//
				break;
			}
		}
		//
		uint32_t extensions_count = 0;
		result = vkEnumerateDeviceExtensionProperties(mVkPhysicalDevice, nullptr, &extensions_count, nullptr);
		if ((result != VK_SUCCESS) ||
			(extensions_count == 0)) {
			return false;
		}
		mVkDeviceExtensions.resize(extensions_count);
		result = vkEnumerateDeviceExtensionProperties(mVkPhysicalDevice, nullptr, &extensions_count, &mVkDeviceExtensions[0]);
		//
		vkGetPhysicalDeviceMemoryProperties(mVkPhysicalDevice, &mVkDeviceMemoryProperties);
		//
		if (!mLogicDevice.Init(mVkPhysicalDevice)) {
			return false;
		}
		//
		return true;
	}

	VkBool32 __stdcall debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) {
		std::cout << "validation layer: " << msg << std::endl;
		return VK_FALSE;
	}
	VkResult CreateDebugReportCallbackEXT(VkInstance instance, const    VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	bool VulkanGI::Init(const char* device_name, ICmdBufferTaskQueue* cmdBufferTaskQueue) {
		GSupportParallelCommandBuffer = true;
		mCmdBufferTaskQueue = cmdBufferTaskQueue;
		//
		if (!getInstanceLevelExtensions()) {
			return false;
		}
		//
		std::vector<char const *> desired_extensions;
#ifdef _DEBUG
		desired_extensions.push_back("VK_EXT_debug_report");
		//desired_extensions.push_back("VK_EXT_debug_utils");
#endif // DEBUG
#ifdef _WIN32
		desired_extensions.push_back("VK_KHR_surface");
		desired_extensions.push_back("VK_KHR_win32_surface");
#endif // _WIN32
		if (!createVKInstance(desired_extensions)) {
			return false;
		}
#ifdef _DEBUG
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		VkDebugReportCallbackEXT callback;
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;//pUserData 也可以指定数据，类似userData
		if (CreateDebugReportCallbackEXT(mVkInstance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
			vkDestroyInstance(mVkInstance, nullptr);
			return false;
		}
#endif // DEBUG
		//
		if (!createLogicDevice(device_name)) {
			return false;
		}
		//
		mCmdBufferManger = new VKCmdBufferManager(mLogicDevice.GetDevice(), mLogicDevice.GetGraphicsQueueFamilyIndex(), mLogicDevice.GetComputeQueueFamilyIndex());
		return VKMemoryManager::Instance()->Init(mVkPhysicalDevice, mLogicDevice.GetDevice());
	}


	std::vector<uint32_t> read_spirv_file(const char *path)
	{
		FILE *file = fopen(path, "rb");
		if (!file) {
			fprintf(stderr, "Failed to open SPIRV file: %s\n", path);
			return {};
		}

		fseek(file, 0, SEEK_END);
		long len = ftell(file) / sizeof(uint32_t);
		rewind(file);

		std::vector<uint32_t> spirv(len);
		if (fread(spirv.data(), sizeof(uint32_t), len, file) != size_t(len))
			spirv.clear();

		fclose(file);
		return spirv;
	}

	ShaderModule* VulkanGI::CreateShaderModule(const ShaderModuleCreateInfo& create_info) {
		std::vector<uint32_t> buffer = read_spirv_file(create_info.path);
		if (buffer.size() == 0) {
			return nullptr;
		}
		//
		VKShaderModule* psm = new VKShaderModule(GraphicsContextManager::Instance()->GetCurrentContext());
		//
		VkShaderModuleCreateInfo shader_module_create_info;
		shader_module_create_info.sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.pNext = nullptr;
		shader_module_create_info.flags = 0;
		shader_module_create_info.codeSize = buffer.size() * sizeof(uint32_t);
		shader_module_create_info.pCode = (const uint32_t*)(buffer.data());
		if (vkCreateShaderModule(mLogicDevice.GetDevice(), &shader_module_create_info, nullptr, &psm->mShaderModule) != VkResult::VK_SUCCESS)
		{
			delete psm;
			return nullptr;
		}
		//
		psm->mSpirvCompiler = std::unique_ptr<spirv_cross::Compiler>(new spirv_cross::Compiler(std::move(buffer)));
		psm->mEntryName = psm->mSpirvCompiler->get_entry_points_and_stages()[0].name;
		//
		return psm;
	}

	ShaderProgram* VulkanGI::CreateShaderProgram(ShaderModule* pVertexShader, ShaderModule* pGeomteryShader, ShaderModule* pTessControlShader, ShaderModule* pTessEvaluationShader, ShaderModule* pFragmentShader) {
		VKGPUProgram* gpuProgram = new VKGPUProgram(GraphicsContextManager::Instance()->GetCurrentContext(),
			                                                                               pVertexShader, pGeomteryShader, pTessControlShader, pTessEvaluationShader, pFragmentShader);
		//
		if (!gpuProgram->InitDescriptorSet(mLogicDevice.GetDevice())) {
			return nullptr;
		}
		//
		return gpuProgram;
	}

	RenderPass* VulkanGI::CreateRenderPass(const RenderPassCreateInfo& create_info) {
		std::vector<VkAttachmentDescription> attachmentDescs(create_info.attachments.size());
		for (int i = 0; i < attachmentDescs.size(); ++i) {
			attachmentDescs[i].flags = 0;
			attachmentDescs[i].format = (VkFormat)create_info.attachments[i].format;
			attachmentDescs[i].loadOp = (VkAttachmentLoadOp)create_info.attachments[i].loadOp;
			attachmentDescs[i].storeOp = (VkAttachmentStoreOp)create_info.attachments[i].storeOp;
			attachmentDescs[i].initialLayout = (VkImageLayout)create_info.attachments[i].initialLayout;
			attachmentDescs[i].finalLayout = (VkImageLayout)create_info.attachments[i].finalLayout;
			attachmentDescs[i].samples = (VkSampleCountFlagBits)create_info.attachments[i].samples;
			attachmentDescs[i].stencilLoadOp = (VkAttachmentLoadOp)create_info.attachments[i].stencilLoadOp;
			attachmentDescs[i].stencilStoreOp = (VkAttachmentStoreOp)create_info.attachments[i].stencilStoreOp;
		}
		//
		std::vector<VkSubpassDescription> subpassDescriptions(create_info.subpasses.size());
		std::vector<std::vector<VkAttachmentReference> > subpassColorReferences(subpassDescriptions.size());
		std::vector<VkAttachmentReference> subpassDepthReferences(subpassDescriptions.size());
		for (int i = 0; i < subpassDescriptions.size(); ++i) {
			auto &subpass = create_info.subpasses[i];
			//
			subpassDescriptions[i].pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
			//
			std::vector<VkAttachmentReference> inputReferences(subpass.inputAttachments.size());
			for (int n = 0; i < inputReferences.size(); ++n) {
				inputReferences[n].attachment = subpass.inputAttachments[n].attachment;
				inputReferences[n].layout = (VkImageLayout)subpass.inputAttachments[n].layout;
			}
			//
			auto &colorReferences = subpassColorReferences[i];
			colorReferences.resize(subpass.colorAttachments.size());
			for (int n = 0; n < colorReferences.size(); ++n) {
				colorReferences[n].attachment = subpass.colorAttachments[n].attachment;
				colorReferences[n].layout = (VkImageLayout)subpass.colorAttachments[n].layout;
			}
			subpassDescriptions[i].colorAttachmentCount = colorReferences.size();
			subpassDescriptions[i].pColorAttachments = colorReferences.data();
			//
			if (create_info.subpasses[i].depthStencilAttachment.attachment >= 0) {
				auto &depthStencilReference = subpassDepthReferences[i];
				depthStencilReference.attachment = create_info.subpasses[i].depthStencilAttachment.attachment;
				depthStencilReference.layout = (VkImageLayout)create_info.subpasses[i].depthStencilAttachment.layout;
				subpassDescriptions[i].pDepthStencilAttachment = &depthStencilReference;
			}
			else {
				subpassDescriptions[i].pDepthStencilAttachment = nullptr;
			}
			//
			subpassDescriptions[i].pResolveAttachments = nullptr;
			subpassDescriptions[i].preserveAttachmentCount = 0;
			subpassDescriptions[i].pPreserveAttachments = nullptr;
		}
		//
		std::vector<VkSubpassDependency> dependencies(create_info.dependencies.size());
		for (int i = 0; i < dependencies.size(); ++i) {
			auto &dependence = create_info.dependencies[i];
			//
			dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies[i].srcSubpass = dependence.srcSubpass;
			dependencies[i].dstSubpass = dependence.dstSubpass;
			dependencies[i].srcStageMask = (VkPipelineStageFlags)dependence.srcStageMask;
			dependencies[i].dstStageMask = (VkPipelineStageFlags)dependence.dstStageMask;
			dependencies[i].srcAccessMask = (VkAccessFlags)dependence.srcAccessMask;
			dependencies[i].dstAccessMask = (VkAccessFlags)dependence.dstAccessMask;
		}
		//
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
		renderPassInfo.pAttachments = attachmentDescs.data();
		renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
		renderPassInfo.pSubpasses = subpassDescriptions.data();
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();
		//
		VkRenderPass renderPass;
		if (vkCreateRenderPass(mLogicDevice.GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			return nullptr;
		}
		//
		auto pres = new VKRenderPass(GraphicsContextManager::Instance()->GetCurrentContext());
		pres->mVkRenderPass = renderPass;
		return pres;
	}

	GraphicsPipeline* VulkanGI::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& create_info) {
		if (create_info.shaderProgram == nullptr) {
			return nullptr;
		}
		//
		std::vector<VkVertexInputBindingDescription> vertex_input_binding_desc;
		std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_desc;

		std::unordered_map<uint8_t, std::vector<VertexFormat> > vbos;
		for (auto &vertexInput : create_info.vertexDeclaration.vertexInputs) {
			vbos[vertexInput.bindingNumber].push_back(vertexInput.format);
			//
			VkVertexInputAttributeDescription tmp;
			tmp.binding = vertexInput.bindingNumber;
			tmp.format = VkFormat(vertexInput.format);
			tmp.location = vertexInput.location;
			tmp.offset = vertexInput.offset;
			vertex_input_attribute_desc.push_back(tmp);
		}
		for (auto &vbo : vbos) {
			uint16_t stride = 0;
			for (auto &vf : vbo.second) {
				stride += GetFormatSize((Format)vf);
			}
			//
			VkVertexInputBindingDescription tmp;
			tmp.binding = vbo.first;
			tmp.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			tmp.stride = stride;
			vertex_input_binding_desc.push_back(tmp);
		}

		VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			nullptr,
			0,
			vertex_input_binding_desc.size(),
			vertex_input_binding_desc.data(),
			vertex_input_attribute_desc.size(),
			vertex_input_attribute_desc.data()
		};
		//
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			nullptr,
			0,
			(VkPrimitiveTopology)create_info.inputAssemblyState.toplogy,
			create_info.inputAssemblyState.primitiveRestartEnable
		};
		//
		VkPipelineViewportStateCreateInfo viewport_state_create_info = {
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			nullptr,
			0,
			create_info.viewportCount,
			nullptr, //&view_port,
			create_info.viewportCount,
			nullptr //&dummyScissor
		};
		//
		VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,     // VkStructureType                                sType
			nullptr,                                                          // const void                                    *pNext
			0,                                                                  // VkPipelineMultisampleStateCreateFlags          flags
			(VkSampleCountFlagBits)create_info.multisampleState.rasterizationSamples,                                        // VkSampleCountFlagBits                          rasterizationSamples
			VK_FALSE,                                                     // VkBool32                                       sampleShadingEnable
			0.0f,                                                               // float                                          minSampleShading
			nullptr,                                                          // const VkSampleMask                            *pSampleMask
			VK_FALSE,                                                     // VkBool32                                       alphaToCoverageEnable
			VK_FALSE                                                      // VkBool32                                       alphaToOneEnable
		};

		std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states;
		for (auto &itr : create_info.colorBlendState.Attachments) {
			VkPipelineColorBlendAttachmentState tmp = {};
			tmp.blendEnable = itr.blendEnable;
			tmp.colorWriteMask = itr.colorWriteMask;
			tmp.alphaBlendOp = (VkBlendOp)itr.alphaBlendOp;
			tmp.colorBlendOp = (VkBlendOp)itr.colorBlendOp;
			tmp.dstAlphaBlendFactor = (VkBlendFactor)itr.dstAlphaBlendFactor;
			tmp.dstColorBlendFactor = (VkBlendFactor)itr.dstColorBlendFactor;
			tmp.srcAlphaBlendFactor = (VkBlendFactor)itr.srcAlphaBlendFactor;
			tmp.srcColorBlendFactor = (VkBlendFactor)itr.srcColorBlendFactor;
			color_blend_attachment_states.push_back(tmp);
		}

		VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,     // VkStructureType                                sType
			nullptr,                                                                                                          // const void                                    *pNext
			0,                                                                                                                   // VkPipelineColorBlendStateCreateFlags           flags
			create_info.colorBlendState.logicOpEnable,                                                // VkBool32                                       logicOpEnable
			(VkLogicOp)create_info.colorBlendState.logicOp,                                       // VkLogicOp                                      logicOp
			color_blend_attachment_states.size(),                                                          // uint32_t                                       attachmentCount
			color_blend_attachment_states.data(),                                                         // const VkPipelineColorBlendAttachmentState     *pAttachments
		};
		memcpy(color_blend_state_create_info.blendConstants, create_info.colorBlendState.blendConstants, sizeof(float)*4);
		//
		VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info = {};
		pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipeline_rasterization_state_create_info.polygonMode = (VkPolygonMode)create_info.rasterizationState.polygonMode;
		pipeline_rasterization_state_create_info.cullMode = create_info.rasterizationState.cullMode;
		pipeline_rasterization_state_create_info.frontFace = (VkFrontFace)create_info.rasterizationState.frontFace;
		pipeline_rasterization_state_create_info.depthClampEnable = create_info.rasterizationState.depthClampEnable;
		pipeline_rasterization_state_create_info.rasterizerDiscardEnable =create_info.rasterizationState.rasterizerDiscardEnable;
		pipeline_rasterization_state_create_info.depthBiasEnable = create_info.rasterizationState.depthBiasEnable;
		pipeline_rasterization_state_create_info.lineWidth = 1.0f;
		//
		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
		pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilStateCreateInfo.depthTestEnable = create_info.depthStencilState.depthTestEnable;
		pipelineDepthStencilStateCreateInfo.depthWriteEnable = create_info.depthStencilState.depthWriteEnable;
		pipelineDepthStencilStateCreateInfo.depthCompareOp = (VkCompareOp)create_info.depthStencilState.depthCompareOp;
		pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = create_info.depthStencilState.depthBoundsTestEnable;
		pipelineDepthStencilStateCreateInfo.minDepthBounds = create_info.depthStencilState.minDepthBounds;
		pipelineDepthStencilStateCreateInfo.maxDepthBounds = create_info.depthStencilState.maxDepthBounds;
		pipelineDepthStencilStateCreateInfo.stencilTestEnable = create_info.depthStencilState.stencilTestEnable;
		//
		pipelineDepthStencilStateCreateInfo.back.failOp = (VkStencilOp)create_info.depthStencilState.back.failOp;
		pipelineDepthStencilStateCreateInfo.back.passOp = (VkStencilOp)create_info.depthStencilState.back.passOp;
		pipelineDepthStencilStateCreateInfo.back.compareOp = (VkCompareOp)create_info.depthStencilState.back.compareOp;
		pipelineDepthStencilStateCreateInfo.back.compareMask = create_info.depthStencilState.back.compareMask;
		pipelineDepthStencilStateCreateInfo.back.writeMask = create_info.depthStencilState.back.writeMask;
		pipelineDepthStencilStateCreateInfo.back.reference = create_info.depthStencilState.back.reference;
		//
		pipelineDepthStencilStateCreateInfo.front.failOp = (VkStencilOp)create_info.depthStencilState.front.failOp;
		pipelineDepthStencilStateCreateInfo.front.passOp = (VkStencilOp)create_info.depthStencilState.front.passOp;
		pipelineDepthStencilStateCreateInfo.front.compareOp = (VkCompareOp)create_info.depthStencilState.front.compareOp;
		pipelineDepthStencilStateCreateInfo.front.compareMask = create_info.depthStencilState.front.compareMask;
		pipelineDepthStencilStateCreateInfo.front.writeMask = create_info.depthStencilState.front.writeMask;
		pipelineDepthStencilStateCreateInfo.front.reference = create_info.depthStencilState.front.reference;
		//
		std::vector<VkDynamicState> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_LINE_WIDTH,
			VK_DYNAMIC_STATE_DEPTH_BIAS,
			VK_DYNAMIC_STATE_BLEND_CONSTANTS,
			VK_DYNAMIC_STATE_DEPTH_BOUNDS,
			VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
			VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
			VK_DYNAMIC_STATE_STENCIL_REFERENCE
		};
		VkPipelineDynamicStateCreateInfo dynamic_state_creat_info = {
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(dynamic_states.size()),
			dynamic_states.data()
		};
		//
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		auto gpuProgram = VKGPUProgram::Cast(create_info.shaderProgram);
		if (gpuProgram->mVertexShader != nullptr) {
			auto shaderModule = VKShaderModule::Cast(gpuProgram->mVertexShader);
			//
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			shaderStage.module = shaderModule->mShaderModule;
			shaderStage.pName = shaderModule->mEntryName.c_str();
			shaderStages.push_back(shaderStage);
		}
		if (gpuProgram->mGeomteryShader != nullptr) {
			auto shaderModule = VKShaderModule::Cast(gpuProgram->mGeomteryShader);
			//
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
			shaderStage.module = shaderModule->mShaderModule;
			shaderStage.pName = shaderModule->mEntryName.c_str();
			shaderStages.push_back(shaderStage);
		}
		if (gpuProgram->mTessControlShader != nullptr) {
			auto shaderModule = VKShaderModule::Cast(gpuProgram->mTessControlShader);
			//
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			shaderStage.module = shaderModule->mShaderModule;
			shaderStage.pName = shaderModule->mEntryName.c_str();
			shaderStages.push_back(shaderStage);
		}
		if (gpuProgram->mTessEvaluationShader != nullptr) {
			auto shaderModule = VKShaderModule::Cast(gpuProgram->mTessEvaluationShader);
			//
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			shaderStage.module = shaderModule->mShaderModule;
			shaderStage.pName = shaderModule->mEntryName.c_str();
			shaderStages.push_back(shaderStage);
		}
		if (gpuProgram->mFragmentShader != nullptr) {
			auto shaderModule = VKShaderModule::Cast(gpuProgram->mFragmentShader);
			//
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderStage.module = shaderModule->mShaderModule;
			shaderStage.pName = shaderModule->mEntryName.c_str();
			shaderStages.push_back(shaderStage);
		}
		//
		VkPipelineLayout pipelineLayout;
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = gpuProgram->mDescriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = gpuProgram->mDescriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = gpuProgram->mPushConstantRanges.size();
		pipelineLayoutCreateInfo.pPushConstantRanges = gpuProgram->mPushConstantRanges.data();
		if (vkCreatePipelineLayout(mLogicDevice.GetDevice(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			return nullptr;
		}
		//
		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.layout = pipelineLayout;
		pipelineCreateInfo.renderPass = ((VKRenderPass*)create_info.renderPass)->mVkRenderPass;
		pipelineCreateInfo.subpass = create_info.subpassIndex;
		pipelineCreateInfo.basePipelineIndex = -1;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.pVertexInputState = &vertex_input_state_create_info;
		pipelineCreateInfo.pTessellationState = nullptr;
		pipelineCreateInfo.pInputAssemblyState = &input_assembly_state_create_info;
		pipelineCreateInfo.pRasterizationState = &pipeline_rasterization_state_create_info;
		pipelineCreateInfo.pColorBlendState = &color_blend_state_create_info;
		pipelineCreateInfo.pMultisampleState = &multisample_state_create_info;
		pipelineCreateInfo.pViewportState = &viewport_state_create_info;
		pipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
		pipelineCreateInfo.pDynamicState = &dynamic_state_creat_info;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();
		//
		VkPipeline graphicsPipeline;
		if (vkCreateGraphicsPipelines(mLogicDevice.GetDevice(), nullptr, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			return nullptr;
		}
		//
		auto pres = new VKGraphicsPipeline(GraphicsContextManager::Instance()->GetCurrentContext());
		pres->mVkPipeLine = graphicsPipeline;
		pres->mVkPipelineLayout = pipelineLayout;
		pres->mGPUProgram = gpuProgram;
		//
		return pres;
	}

	Swapchain* VulkanGI::CreateSwapchain(const SwapchainCreateInfo& create_info) {
		if (mSwapchain == nullptr) {
			mSwapchain = new  VKSwapchain(GraphicsContextManager::Instance()->GetCurrentContext(), mLogicDevice.GetDevice());
		}
		//
		VkResult result;
		//
		if (mSwapchain->mVkSurface == nullptr) {
#ifdef _WIN32
			VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
				VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
				nullptr,
				0,
				GetModuleHandle(nullptr),
				(HWND)create_info.windowHandle
			};
			result = vkCreateWin32SurfaceKHR(mVkInstance, &surfaceCreateInfo, nullptr, &mSwapchain->mVkSurface);
			if (result != VK_SUCCESS) {
				return nullptr;
			}
#endif
		}
		//
		VkBool32 supportedSurface;
		result = vkGetPhysicalDeviceSurfaceSupportKHR(mVkPhysicalDevice, mLogicDevice.GetGraphicsQueueFamilyIndex(), mSwapchain->mVkSurface, &supportedSurface);
		if (result != VK_SUCCESS || !supportedSurface) {
			return nullptr;
		}
		result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mVkPhysicalDevice, mSwapchain->mVkSurface, &mSwapchain->mVkSurfaceCapabilities);
		
		//find supported surface format
		uint32_t numFormat;
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(mVkPhysicalDevice, mSwapchain->mVkSurface, &numFormat, nullptr);
		if (result != VK_SUCCESS) {
			return nullptr;
		}
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		surfaceFormats.resize(numFormat);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(mVkPhysicalDevice, mSwapchain->mVkSurface, &numFormat, &surfaceFormats[0]);
		//
		mSwapchain->mVkSurfaceFormat = surfaceFormats[0];
		for (uint32_t i = 0; i < numFormat; ++i) {
			if (create_info.preferredPixelFormat == surfaceFormats[i].format) {
				mSwapchain->mVkSurfaceFormat = surfaceFormats[i];
				break;
			}
		}
		//
		// Find a supported composite alpha format (not all devices support alpha opaque)
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Simply select the first composite alpha format available
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags) {
			if (mSwapchain->mVkSurfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag) {
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}
		//
		VkExtent2D swapchainExtent = {};
		// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
		if (mSwapchain->mVkSurfaceCapabilities.currentExtent.width == (uint32_t)-1) {
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchainExtent.width = create_info.sizeX;
			swapchainExtent.height = create_info.sizeY;
		} else {
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = mSwapchain->mVkSurfaceCapabilities.currentExtent;
		}
		mSwapchain->mExtent.width = swapchainExtent.width;
		mSwapchain->mExtent.height = swapchainExtent.height;
		//
		uint32_t presentModeCount;
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(mVkPhysicalDevice, mSwapchain->mVkSurface, &presentModeCount, nullptr);
		if (result != VK_SUCCESS) {
			return nullptr;
		}
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(mVkPhysicalDevice, mSwapchain->mVkSurface, &presentModeCount, &presentModes[0]);
		// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
		// This mode waits for the vertical blank ("v-sync")
		mSwapchain->mVkPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		// If v-sync is not requested, try to find a mailbox mode
		// It's the lowest latency non-tearing present mode available
		if (!create_info.vsync) {
			for (size_t i = 0; i < presentModeCount; i++)
			{
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
					mSwapchain->mVkPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if ((mSwapchain->mVkPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
					mSwapchain->mVkPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		}
		//
		VkSwapchainCreateInfoKHR swapchain_create_info;
		swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.pNext = NULL;
		swapchain_create_info.flags = 0;
		swapchain_create_info.surface = mSwapchain->mVkSurface;
		swapchain_create_info.minImageCount = mSwapchain->mVkSurfaceCapabilities.maxImageCount > 2 ? 3 : 2;
		swapchain_create_info.imageFormat = mSwapchain->mVkSurfaceFormat.format;
		swapchain_create_info.imageColorSpace = mSwapchain->mVkSurfaceFormat.colorSpace;
		swapchain_create_info.imageExtent = swapchainExtent;
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0;
		swapchain_create_info.pQueueFamilyIndices = NULL;
		swapchain_create_info.presentMode = mSwapchain->mVkPresentMode;
		swapchain_create_info.oldSwapchain = mSwapchain->mVkSwapchain;
		// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
		swapchain_create_info.clipped = VK_TRUE;
		swapchain_create_info.compositeAlpha = compositeAlpha;

		// Enable transfer source on swap chain images if supported
		if (mSwapchain->mVkSurfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
			swapchain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		// Enable transfer destination on swap chain images if supported
		if (mSwapchain->mVkSurfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
			swapchain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		result = vkCreateSwapchainKHR(mLogicDevice.GetDevice(), &swapchain_create_info, nullptr, &mSwapchain->mVkSwapchain);
		if (result != VK_SUCCESS) {
			return nullptr;
		}
		//
		uint32_t imageCount;
		vkGetSwapchainImagesKHR(mLogicDevice.GetDevice(), mSwapchain->mVkSwapchain, &imageCount, NULL);

		// Get the swap chain images
		std::vector<VkImage> swapchainImgs(imageCount);
		vkGetSwapchainImagesKHR(mLogicDevice.GetDevice(), mSwapchain->mVkSwapchain, &imageCount, swapchainImgs.data());

		// Get the swap chain buffers containing the image and imageview
		mSwapchain->mColorAttachments.resize(imageCount);
		for (uint32_t i = 0; i < imageCount; i++)
		{
			mSwapchain->mColorAttachments[i] = new VKImage2D(GraphicsContextManager::Instance()->GetCurrentContext(), (Format)swapchain_create_info.imageFormat, swapchainExtent.width, swapchainExtent.height, 1);
			mSwapchain->mColorAttachments[i]->mVkImage = swapchainImgs[i];
			mSwapchain->mColorAttachments[i]->mUsageFlag = swapchain_create_info.imageUsage;
			//
			VkImageViewCreateInfo colorAttachmentView = {};
			colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorAttachmentView.pNext = NULL;
			colorAttachmentView.format = swapchain_create_info.imageFormat;
			colorAttachmentView.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorAttachmentView.subresourceRange.baseMipLevel = 0;
			colorAttachmentView.subresourceRange.levelCount = 1;
			colorAttachmentView.subresourceRange.baseArrayLayer = 0;
			colorAttachmentView.subresourceRange.layerCount = 1;
			colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorAttachmentView.flags = 0;

			colorAttachmentView.image = swapchainImgs[i];
			VkImageView imgView;
			if (vkCreateImageView(mLogicDevice.GetDevice(), &colorAttachmentView, nullptr, &imgView) != VK_SUCCESS) {
				return nullptr;
			}
			VKImageView* pview = new VKImageView(GraphicsContextManager::Instance()->GetCurrentContext(), mSwapchain->mColorAttachments[i], imgView, colorAttachmentView);
			mSwapchain->mColorAttachments[i]->mOrgiView = pview;
			//create depth stencil attachment
			if (create_info.preferredDepthStencilFormat == 0) {
				continue;
			}
			auto usage = ImageUsageFlagBits::IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | ImageUsageFlagBits::IMAGE_USAGE_TRANSFER_SRC_BIT;
			auto depthStencilImg = CreateImage2D(swapchainExtent.width, swapchainExtent.height, create_info.preferredDepthStencilFormat, 1, SampleCountFlagBits::SAMPLE_COUNT_1_BIT, usage);
			if (depthStencilImg == nullptr) {
				return nullptr;
			}
			mSwapchain->mDepthStencilAttachments.push_back((VKImage2D*)depthStencilImg);
		}
		//
		return mSwapchain;
	}

	FrameBuffer* VulkanGI::CreateFrameBuffer(RenderPass* targetRenderPass, uint8_t numAttachment, ImageView** attachments, ClearValue* clearValues, uint32_t width, uint32_t height) {
		std::vector<VkImageView> imageViews(numAttachment);
		for (int i = 0; i < numAttachment; ++i) {
			imageViews[i] = VKImageView::Cast(attachments[i])->mImageView;
		}
		//
		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = NULL;
		frameBufferCreateInfo.renderPass = VKRenderPass::Cast(targetRenderPass)->mVkRenderPass;
		frameBufferCreateInfo.attachmentCount = numAttachment;
		frameBufferCreateInfo.pAttachments = imageViews.data();
		frameBufferCreateInfo.width = width;
		frameBufferCreateInfo.height = height;
		frameBufferCreateInfo.layers = 1;
		//
		VKFrameBuffer* res = new VKFrameBuffer(GraphicsContextManager::Instance()->GetCurrentContext());
		if (vkCreateFramebuffer(mLogicDevice.GetDevice(), &frameBufferCreateInfo, nullptr, &res->mFrameBuffer) != VK_SUCCESS) {
			return nullptr;
		}
		for (int i = 0; i < numAttachment; ++i) {
			res->AddAttachment(attachments[i], clearValues[i]);
		}
		res->mExtent.width = width;
		res->mExtent.height = height;
		//
		return res;
	}

	ComputePass* VulkanGI::CreateComputePass() {
		return nullptr;
	}

	ComputePipeline* VulkanGI::CreateComputePipeline() {
		return nullptr;
	}

	bool VulkanGI::createBuffer(uint64_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VKBuffer* pres) {
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = size;
		bufferInfo.usage = usageFlags;
		VKMemory::MemoryUsage memoryUsage;
		if (memoryPropertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			memoryUsage = VKMemory::MemoryUsage::VK_MEMORY_USAGE_GPU_ONLY;
		}
		else if(memoryPropertyFlags == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			memoryUsage = VKMemory::MemoryUsage::VK_MEMORY_USAGE_CPU_ONLY;
		}
		else if (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
			memoryUsage = VKMemory::MemoryUsage::VK_MEMORY_USAGE_GPU_TO_CPU;
		}
		else {
			memoryUsage = VKMemory::MemoryUsage::VK_MEMORY_USAGE_CPU_TO_GPU;
		}
		//
		if (VKMemoryManager::Instance()->CreateBuffer(bufferInfo, &pres->mVkBuffer, memoryUsage, pres->mMemory) != VK_SUCCESS) {
			return false;
		}
		//
		return true;
	}

	bool VulkanGI::updateBuffer(VKBuffer* buffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext) {
		auto memoryPropertyFlags = mVkDeviceMemoryProperties.memoryTypes[buffer->mMemory->GetMemoryTypeIndex()].propertyFlags;
		//
		if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
			void* pdstData = nullptr;
			if (VKMemoryManager::Instance()->MapMemory(buffer->mMemory, &pdstData) == VK_SUCCESS) {
				memcpy(pdstData, pdata, size);
				VKMemoryManager::Instance()->UnMapMemory(buffer->mMemory);
				if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
					VKMemoryManager::Instance()->FlushAllocation(buffer->mMemory, offset, size);
				}
				//
				return true;
			}
		}
		//
		//uint32_t tmp = buffer->mMemory->GetSize();
		if (pUpdateContext != nullptr) {
			VKBufferUpdateContext::UpdateItem tmp;
			tmp.dstBuffer = buffer;
			tmp.offset = offset;
			tmp.size = size;
			tmp.pdata = pdata;
			((VKBufferUpdateContext*)pUpdateContext)->updates.push_back(tmp);
			//
			return true;
		}
		//
		VkBufferCreateInfo vbInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		vbInfo.size = size;
		vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VKMemory* pmemory = nullptr;
		if (VKMemoryManager::Instance()->CreateBuffer(vbInfo, &stagingBuffer, VKMemory::MemoryUsage::VK_MEMORY_USAGE_CPU_ONLY, pmemory) != VK_SUCCESS) {
			return false;
		}
		//
		void* pbuffer = nullptr;
		VKMemoryManager::Instance()->MapMemory(pmemory, &pbuffer);
		memcpy(pbuffer, pdata, size);
		VKMemoryManager::Instance()->UnMapMemory(pmemory);
		//
		if (!BeginSingleTimeCommands()) {
			VKMemoryManager::Instance()->DestoryBuffer(stagingBuffer, pmemory);
			return false;
		}
		//
		VkBufferCopy vbCopyRegion = {};
		vbCopyRegion.srcOffset = 0;
		vbCopyRegion.dstOffset = offset;
		vbCopyRegion.size = size;
		vkCmdCopyBuffer(mCmdBufferManger->GetUpLoadCmdBuffer(), stagingBuffer, buffer->mVkBuffer, 1, &vbCopyRegion);
		//
		auto res = EndSingleTimeCommands();
		VKMemoryManager::Instance()->DestoryBuffer(stagingBuffer, pmemory);
		//
		return res;
	}

	bool VulkanGI::BeginSingleTimeCommands() {
		VkCommandBufferBeginInfo cmdBufBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		if (vkBeginCommandBuffer(mCmdBufferManger->GetUpLoadCmdBuffer(), &cmdBufBeginInfo) == VK_SUCCESS) {
			return true;
		}
		//
		return false;
	}

	bool VulkanGI::EndSingleTimeCommands() {
		vkEndCommandBuffer(mCmdBufferManger->GetUpLoadCmdBuffer());
		//
		auto cmdBuffer = mCmdBufferManger->GetUpLoadCmdBuffer();
		if (mLogicDevice.ExcuteCmdOnIdleGraphicsQueue(&cmdBuffer, true) != VK_SUCCESS) {
			return false;
		}
		//
		return true;
	}

	Buffer* VulkanGI::CreateBuffer(uint64_t size, BufferUsageFlags usageFlags) {
		auto pres = new  VKBuffer(GraphicsContextManager::Instance()->GetCurrentContext(), usageFlags, size);
		VkBufferUsageFlags bufferUsageFlags = 0;
		if (usageFlags & BufferUsageFlagBits::BUFFER_USAGE_TRANSFER_SRC_BIT) bufferUsageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (usageFlags & BufferUsageFlagBits::BUFFER_USAGE_TRANSFER_DST_BIT) bufferUsageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if(usageFlags & BufferUsageFlagBits::BUFFER_USAGE_INDEX_BIT) bufferUsageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (usageFlags & BufferUsageFlagBits::BUFFER_USAGE_INDIRECT_BIT) bufferUsageFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		if (usageFlags & BufferUsageFlagBits::BUFFER_USAGE_STORAGE_BIT) bufferUsageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (usageFlags & BufferUsageFlagBits::BUFFER_USAGE_STORAGE_TEXEL_BIT) bufferUsageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		if (usageFlags & BufferUsageFlagBits::BUFFER_USAGE_UNIFORM_BIT) bufferUsageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (usageFlags & BufferUsageFlagBits::BUFFER_USAGE_UNIFORM_TEXEL_BIT) bufferUsageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		if (usageFlags & BufferUsageFlagBits::BUFFER_USAGE_VERTEX_BIT) bufferUsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		//
		VkMemoryPropertyFlags memoryPropertyFlags = 0;
		usageFlags &= ~BufferUsageFlagBits::BUFFER_USAGE_TRANSFER_SRC_BIT;
		usageFlags &= ~BufferUsageFlagBits::BUFFER_USAGE_TRANSFER_DST_BIT;
		if (usageFlags == BufferUsageFlagBits::BUFFER_USAGE_UPLOAD) {
			memoryPropertyFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}
		else if (usageFlags & BufferUsageFlagBits::BUFFER_USAGE_UPLOAD) {
			memoryPropertyFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		}
		else if (usageFlags & BufferUsageFlagBits::BUFFER_USAGE_READBACK) {
			memoryPropertyFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
		}
		else {
			memoryPropertyFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		//
		if (!createBuffer(size, bufferUsageFlags, memoryPropertyFlags, pres)) {
			delete pres;
			return nullptr;
		}
		//
		return pres;
	}

	BufferUpdateContext* VulkanGI::BeginUpdateBuffer() {
		return new VKBufferUpdateContext(GraphicsContextManager::Instance()->GetCurrentContext());
	}

	bool VulkanGI::EndUpdateBuffer(BufferUpdateContext* pUpdateContext) {
		std::vector<std::array<void*, 5> >copys(((VKBufferUpdateContext*)pUpdateContext)->updates.size());
		//
		int index = 0;
		for (auto &itm : ((VKBufferUpdateContext*)pUpdateContext)->updates) {
			VkBufferCreateInfo vbInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			vbInfo.size = itm.size;
			vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkBuffer stagingBuffer = VK_NULL_HANDLE;
			VKMemory* pmemory = nullptr;
			if (VKMemoryManager::Instance()->CreateBuffer(vbInfo, &stagingBuffer, VKMemory::MemoryUsage::VK_MEMORY_USAGE_CPU_ONLY, pmemory) != VK_SUCCESS) {
				continue;
			}
			//
			void* pbuffer = nullptr;
			VKMemoryManager::Instance()->MapMemory(pmemory, &pbuffer);
			memcpy(pbuffer, itm.pdata, itm.size);
			VKMemoryManager::Instance()->UnMapMemory(pmemory);
			//
			copys[index][0] = itm.dstBuffer;
			copys[index][1] = stagingBuffer;
			copys[index][2] = pmemory;
			copys[index][3] = (void*)itm.offset;
			copys[index][4] = (void*)itm.size;
			++index;
		}
		//
		if (!BeginSingleTimeCommands()) {
			return false;
		}
		//
		for (auto & itm : copys) {
			VkBufferCopy vbCopyRegion = {};
			vbCopyRegion.srcOffset = 0;
			vbCopyRegion.dstOffset = (uint32_t)itm[3];
			vbCopyRegion.size =(uint32_t)itm[4];
			vkCmdCopyBuffer(mCmdBufferManger->GetUpLoadCmdBuffer(), (VkBuffer)itm[1], ((VKBuffer*)(itm[0]))->mVkBuffer, 1, &vbCopyRegion);
		}
		//
		auto res = EndSingleTimeCommands();
		//
		for (auto & itm : copys) {
			VKMemory* pmemory = (VKMemory*)std::move(itm[2]);
			VKMemoryManager::Instance()->DestoryBuffer((VkBuffer)itm[1], pmemory);
		}
		//
		return res;
	}

	void VulkanGI::UpdateBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext) {
		updateBuffer((VKBuffer*)pbuffer, offset, size, pdata, pUpdateContext);
	}

	void* VulkanGI::MapBuffer(Buffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode) {
		return nullptr;
	}

	void VulkanGI::UnMapBuffer(Buffer* pbuffer) {
	}

	void VulkanGI::BindUniformBuffer(ShaderProgram* pProgram, uint8_t setIndex, uint32_t bindingIndex, Buffer* pbuffer, uint32_t offset, uint32_t size) {
		auto gpuProgram = VKGPUProgram::Cast(pProgram);
		auto uniformBuffer = VKBuffer::Cast(pbuffer);
		assert((uniformBuffer->mUsageFlags&BufferUsageFlagBits::BUFFER_USAGE_UNIFORM_BIT) != 0);
		//
		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = uniformBuffer->mVkBuffer;
		bufferInfo.offset = offset;
		bufferInfo.range = size;
		//
		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.pNext = NULL;
		writeDescriptorSet.dstSet = gpuProgram->mDescriptorSets[gpuProgram->mIndexSet[setIndex]];
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.dstBinding = bindingIndex;
		writeDescriptorSet.pBufferInfo = &bufferInfo;
		writeDescriptorSet.descriptorCount = 1;
		//
		vkUpdateDescriptorSets(mLogicDevice.GetDevice(), 1, &writeDescriptorSet, 0, NULL);
	}

	VkImageAspectFlags getImageAspectFlags(Format format, ImageUsageFlags usageFlags) {
		if ((usageFlags & ImageUsageFlagBits::IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
			switch (format)
			{
			case ASGI::FORMAT_D16_UNORM:
			case ASGI::FORMAT_X8_D24_UNORM_PACK32:
			case ASGI::FORMAT_D32_SFLOAT:
				return VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
			case ASGI::FORMAT_S8_UINT:
				return  VK_IMAGE_ASPECT_STENCIL_BIT;
			case ASGI::FORMAT_D16_UNORM_S8_UINT:
			case ASGI::FORMAT_D24_UNORM_S8_UINT:
			case ASGI::FORMAT_D32_SFLOAT_S8_UINT:
				return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		//
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
	Image2D* VulkanGI::CreateImage2D(uint32_t sizeX, uint32_t sizeY, Format format, uint32_t numMips, SampleCountFlagBits samples, ImageUsageFlags usageFlags) {
		VkImageUsageFlags imageUsageFlags = 0;
		if (usageFlags & ImageUsageFlagBits::IMAGE_USAGE_TRANSFER_SRC_BIT) imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (usageFlags & ImageUsageFlagBits::IMAGE_USAGE_TRANSFER_DST_BIT) imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (usageFlags & ImageUsageFlagBits::IMAGE_USAGE_COLOR_ATTACHMENT_BIT) imageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (usageFlags & ImageUsageFlagBits::IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) imageUsageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (usageFlags & ImageUsageFlagBits::IMAGE_USAGE_INPUT_ATTACHMENT_BIT) imageUsageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		if (usageFlags & ImageUsageFlagBits::IMAGE_USAGE_SAMPLED_BIT) imageUsageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (usageFlags & ImageUsageFlagBits::IMAGE_USAGE_STORAGE_BIT) imageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		if (usageFlags & ImageUsageFlagBits::IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) imageUsageFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		//
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = NULL;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = (VkFormat)format;
		imageCreateInfo.mipLevels = numMips;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = (VkSampleCountFlagBits)samples;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.extent = { sizeX, sizeY, 1 };
		imageCreateInfo.usage = imageUsageFlags;

		
		VkImage vkImage;
		VKMemory* pmemory = nullptr;
		VKMemory::MemoryUsage memoryUsage;
		usageFlags &= ~ImageUsageFlagBits::IMAGE_USAGE_TRANSFER_DST_BIT;
		usageFlags &= ~ImageUsageFlagBits::IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (usageFlags == ImageUsageFlagBits::IMAGE_USAGE_UPLOAD) {
			memoryUsage = VKMemory::MemoryUsage::VK_MEMORY_USAGE_CPU_ONLY;
		}
		else if (usageFlags & ImageUsageFlagBits::IMAGE_USAGE_UPLOAD) {
			memoryUsage = VKMemory::MemoryUsage::VK_MEMORY_USAGE_CPU_TO_GPU;
		}
		else if (usageFlags & ImageUsageFlagBits::IMAGE_USAGE_READBACK) {
			memoryUsage = VKMemory::MemoryUsage::VK_MEMORY_USAGE_GPU_TO_CPU;
		}
		else {
			memoryUsage = VKMemory::MemoryUsage::VK_MEMORY_USAGE_GPU_ONLY;
		}
		if (VKMemoryManager::Instance()->CreateImage(imageCreateInfo, &vkImage, memoryUsage, pmemory) != VK_SUCCESS) {
			return nullptr;
		}
		//
		VkImageViewCreateInfo imageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		imageViewInfo.image = vkImage;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = (VkFormat)format;
		imageViewInfo.subresourceRange.aspectMask = getImageAspectFlags(format, usageFlags);
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		VkImageView imageView;
		if (VKMemoryManager::Instance()->CreateImageView(imageViewInfo, &imageView) != VK_SUCCESS) {
			VKMemoryManager::Instance()->DestoryImage(vkImage, pmemory);
			return nullptr;
		}
		//
		auto pres = new VKImage2D(GraphicsContextManager::Instance()->GetCurrentContext(), format, sizeX, sizeY, numMips);
		pres->mVkImage = vkImage;
		pres->mMemory = pmemory;
		pres->mUsageFlag = usageFlags;
		pres->mOrgiView = new VKImageView(GraphicsContextManager::Instance()->GetCurrentContext(), pres, imageView, imageViewInfo);
		//
		return pres;
	}

	ImageView* VulkanGI::CreateImageView(Image2D* srcImage, uint32_t mipLevel) {
		return nullptr;
	}

	ImageView* VulkanGI::CreateImageView(Image2D* srcImage, uint32_t mipLevel, uint32_t numMipLevels, Format format) {
		return nullptr;
	}


	ImageUpdateContext* VulkanGI::BeginUpdateImage() {
		return new VKImageUpdateContext(GraphicsContextManager::Instance()->GetCurrentContext());
	}

	bool VulkanGI::EndUpdateImage(ImageUpdateContext* pUpdateContext) {
		return false;
	}

	bool VulkanGI::UpdateImage2D(Image2D* pimg, uint32_t level, uint32_t offsetX, uint32_t offsetY, uint32_t sizeX, uint32_t sizeY, void* pdata, ImageUpdateContext* pUpdateContext) {
		if (pUpdateContext != nullptr) {

		}
		//
		VkBufferCreateInfo vbInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		vbInfo.size = sizeX*sizeY*GetFormatSize(pimg->GetFormat());
		vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VKMemory* pmemory = nullptr;
		if (VKMemoryManager::Instance()->CreateBuffer(vbInfo, &stagingBuffer, VKMemory::MemoryUsage::VK_MEMORY_USAGE_CPU_ONLY, pmemory) != VK_SUCCESS) {
			return false;
		}
		//
		void* pbuffer = nullptr;
		VKMemoryManager::Instance()->MapMemory(pmemory, &pbuffer);
		memcpy(pbuffer, pdata, vbInfo.size);
		VKMemoryManager::Instance()->UnMapMemory(pmemory);
		//
		if (!BeginSingleTimeCommands()) {
			VKMemoryManager::Instance()->DestoryBuffer(stagingBuffer, pmemory);
			return false;
		}
		//
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = ((VKImageView*)(pimg->GetOrigView()))->mViewInfo.subresourceRange.aspectMask;
		bufferCopyRegion.imageSubresource.mipLevel = level;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = sizeX;
		bufferCopyRegion.imageExtent.height = sizeY;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = 0;
		//
		VkImageSubresourceRange subresourceRange = {};
		// Image only contains color data
		subresourceRange.aspectMask = ((VKImageView*)(pimg->GetOrigView()))->mViewInfo.subresourceRange.aspectMask;
		// Start at first mip level
		subresourceRange.baseMipLevel = level;
		// We will transition on all mip levels
		subresourceRange.levelCount = 1;
		// The 2D texture only has one layer
		subresourceRange.layerCount = 1;

		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = ((VKImage2D*)pimg)->mVkImage;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
		// Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
		// Destination pipeline stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
		vkCmdPipelineBarrier(
			mCmdBufferManger->GetUpLoadCmdBuffer(),
			GetImageBarrierFlags(((VKImage2D*)pimg)->mLayoutBarrier[level], imageMemoryBarrier.srcAccessMask, imageMemoryBarrier.oldLayout),
			GetImageBarrierFlags(VKImageLayoutBarrier::TransferDest, imageMemoryBarrier.dstAccessMask, imageMemoryBarrier.newLayout),
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
		//
		((VKImage2D*)pimg)->mLayoutBarrier[level] = VKImageLayoutBarrier::TransferDest;
		//
		vkCmdCopyBufferToImage(
			mCmdBufferManger->GetUpLoadCmdBuffer(),
			stagingBuffer,
			((VKImage2D*)pimg)->mVkImage,
			imageMemoryBarrier.newLayout,
			1,
			&bufferCopyRegion);
		//
		VkPipelineStageFlags stageFlags = 0;
		if ((bufferCopyRegion.imageSubresource.aspectMask & VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
			stageFlags = GetImageBarrierFlags(VKImageLayoutBarrier::PixelShaderRead, imageMemoryBarrier.dstAccessMask, imageMemoryBarrier.newLayout);
			((VKImage2D*)pimg)->mLayoutBarrier[level] = VKImageLayoutBarrier::PixelShaderRead;
		}
		else if ((bufferCopyRegion.imageSubresource.aspectMask & VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT) != 0 ||
			(bufferCopyRegion.imageSubresource.aspectMask & VkImageAspectFlagBits::VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
			stageFlags = GetImageBarrierFlags(VKImageLayoutBarrier::DepthStencilAttachment, imageMemoryBarrier.dstAccessMask, imageMemoryBarrier.newLayout);
			((VKImage2D*)pimg)->mLayoutBarrier[level] = VKImageLayoutBarrier::PixelDepthStencilRead;
		}
		vkCmdPipelineBarrier (
			mCmdBufferManger->GetUpLoadCmdBuffer(),
			GetImageBarrierFlags(VKImageLayoutBarrier::TransferDest, imageMemoryBarrier.srcAccessMask, imageMemoryBarrier.oldLayout),
			stageFlags,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
		//
		if (!EndSingleTimeCommands()) {
			VKMemoryManager::Instance()->DestoryBuffer(stagingBuffer, pmemory);
			return false;
		}
		//
		VKMemoryManager::Instance()->DestoryBuffer(stagingBuffer, pmemory);
		return true;
	}

	Sampler* VulkanGI::CreateSampler(float minLod, float maxLod, float  mipLodBias,
		Filter magFilter, Filter minFilter,
		SamplerMipmapMode mipmapMode,
		SamplerAddressMode addressModeU,
		SamplerAddressMode addressModeV,
		SamplerAddressMode addressModeW,
		float  maxAnisotropy, CompareOp compareOp,
		BorderColor borderColor) {
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext = NULL;
		samplerCreateInfo.minLod = minLod;
		samplerCreateInfo.maxLod = maxLod;
		samplerCreateInfo.mipLodBias = mipLodBias;
		samplerCreateInfo.magFilter = (VkFilter)magFilter;
		samplerCreateInfo.minFilter = (VkFilter)minFilter;
		samplerCreateInfo.mipmapMode = (VkSamplerMipmapMode)mipmapMode;
		samplerCreateInfo.addressModeU = (VkSamplerAddressMode)addressModeU;
		samplerCreateInfo.addressModeV = (VkSamplerAddressMode)addressModeV;
		samplerCreateInfo.addressModeW = (VkSamplerAddressMode)addressModeW;
		samplerCreateInfo.compareOp = (VkCompareOp)compareOp;
		samplerCreateInfo.borderColor = (VkBorderColor)borderColor;
		if (maxAnisotropy <= 1.0 || !mVkDeviceFeatures.samplerAnisotropy) {
			samplerCreateInfo.anisotropyEnable = VK_FALSE;
			samplerCreateInfo.maxAnisotropy = 1.0;
		}
		else {
			samplerCreateInfo.anisotropyEnable = VK_TRUE;
			samplerCreateInfo.maxAnisotropy = maxAnisotropy > mVkDeviceProperties.limits.maxSamplerAnisotropy ? mVkDeviceProperties.limits.maxSamplerAnisotropy : maxAnisotropy;
		}

		VkSampler sampler;
		if (vkCreateSampler(mLogicDevice.GetDevice(), &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
			return nullptr;
		}
		//
		VKSampler* res = new VKSampler(GraphicsContextManager::Instance()->GetCurrentContext());
		res->mVkSampler = sampler;
		return res;
	}

	void VulkanGI::BindTexture(ShaderProgram* pProgram, uint8_t setIndex, uint32_t bindingIndex, ImageView* pImgView, Sampler* pSampler) {
		auto gpuProgram = VKGPUProgram::Cast(pProgram);
		auto imageView= VKImageView::Cast(pImgView);
		//
		VkDescriptorImageInfo imgInfo;
		imgInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
		imgInfo.imageView = imageView->mImageView;
		imgInfo.sampler = VKSampler::Cast(pSampler)->mVkSampler;
		//
		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.pNext = NULL;
		writeDescriptorSet.dstSet = gpuProgram->mDescriptorSets[gpuProgram->mIndexSet[setIndex]];
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.dstBinding = bindingIndex;
		writeDescriptorSet.pImageInfo = &imgInfo;
		writeDescriptorSet.descriptorCount = 1;
		//
		vkUpdateDescriptorSets(mLogicDevice.GetDevice(), 1, &writeDescriptorSet, 0, NULL);
	}

	ExcuteQueue* VulkanGI::AcquireExcuteQueue(QueueType queueType) {
		if (queueType == QueueType::QUEUE_TYPE_GRAPHICS) {
			return mLogicDevice.GetIdleGraphicsQueue();
		}
		else if (queueType == QueueType::QUEUE_TYPE_COMPUTE) {
			return mLogicDevice.GetIdleComputeQueue();
		}
		//
		return nullptr;
	}

	void VulkanGI::WaitQueueExcuteFinished(uint32_t numWaiteQueue, ExcuteQueue** excuteQueues) {
		mLogicDevice.WaiteQueueFinished(numWaiteQueue, excuteQueues);
	}

	bool VulkanGI::SubmitCommands(ExcuteQueue* excuteQueue, uint32_t numBuffers, CommandBuffer** cmdBuffers, uint32_t numWaiteQueue, ExcuteQueue** waiteQueues, uint32_t numSwapchain, Swapchain** waiteSwapchains, bool waiteFinished) {
		if (mCmdBufferTaskQueue != nullptr) {
			for (uint32_t i = 0; i < numBuffers; ++i) {
				auto tmp = VKCommandBuffer::Cast(cmdBuffers[i]);
				std::unique_lock<std::mutex> lk(tmp->mMtxSecondCmdBuffer);
				tmp->mCond.wait(lk, [&] {return tmp->mUnExcuteSecondCmdBufferCount == 0; });
			}
		}
		//
		for (uint32_t i = 0; i < numBuffers; ++i) {
			if (VKCommandBuffer::Cast(cmdBuffers[i])->Excute() != VK_SUCCESS) {
				return false;
			}
		}
		//
		if (mLogicDevice.ExcuteCommands(excuteQueue, numBuffers, cmdBuffers, numWaiteQueue, waiteQueues, numSwapchain, waiteSwapchains, waiteFinished) != VK_SUCCESS) {
			return false;
		}
		//
		return true;
	}

	void VulkanGI::Present(ExcuteQueue* excuteQueue, uint32_t numSwapchain, Swapchain** swapchains, bool waiteFinished) {
		std::vector<VkSwapchainKHR> vkSwapchains(numSwapchain);
		std::vector<uint32_t> imgIndexes(numSwapchain);
		for (uint32_t i = 0; i < numSwapchain; ++i) {
			auto tmp = VKSwapchain::Cast(swapchains[i]);
			vkSwapchains[i] = tmp->mVkSwapchain;
			imgIndexes[i] = tmp->mCurrentAttachmentIndex;
		}
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = numSwapchain;
		presentInfo.pSwapchains = vkSwapchains.data();
		presentInfo.pImageIndices = imgIndexes.data();

		auto tmp = VKExcuteQueue::Cast(excuteQueue);
		presentInfo.pWaitSemaphores = &tmp->signalingSemaphore;
		presentInfo.waitSemaphoreCount = 1;
		//
		if (vkQueuePresentKHR(tmp->mQueue, &presentInfo) != VK_SUCCESS) {
			std::cout << "faild" << std::endl;
		}
	}

	CommandBuffer* VulkanGI::CreateCmdBuffer() {
		return new VKCommandBuffer(GraphicsContextManager::Instance()->GetCurrentContext());
	}

	bool VulkanGI::BeginRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, FrameBuffer* frameBuffer) {
		auto primaryBuffer = VKCommandBuffer::Cast(cmdBuffer);
		if (primaryBuffer->mBindingCmdBuffer == nullptr) {
			primaryBuffer->mBindingCmdBuffer = mCmdBufferManger->AcquirePrimaryCmdBuffer(VK_PIPELINE_BIND_POINT_GRAPHICS);
			if (primaryBuffer->mBindingCmdBuffer == nullptr) {
				return false;
			}
		}
		//
		primaryBuffer->mSecondCmdBuffers.clear();
		//
		primaryBuffer->PushCommand(new VKCmdBeginRenderPass(renderPass, frameBuffer));
		return true;
	}

	void VulkanGI::EndSubRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) {
		auto primaryBuffer = VKCommandBuffer::Cast(cmdBuffer);
		//
		for (uint32_t i = 0; i < numSecondCmdBuffer; ++i) {
			auto secondBuffer = VKCommandBuffer::Cast(secondCmdBuffers[i]);
			if (mCmdBufferTaskQueue != nullptr && secondBuffer->mBindingCmdBuffer == nullptr) {
					secondBuffer->mBindingCmdBuffer = mCmdBufferManger->AcquireSecondCmdBuffer(VK_PIPELINE_BIND_POINT_GRAPHICS);
			}
			//
			if (mCmdBufferTaskQueue != nullptr) {
				primaryBuffer->mSecondCmdBuffers.push_back(secondCmdBuffers[i]);
				//
				{
					std::lock_guard<std::mutex> lock(primaryBuffer->mMtxSecondCmdBuffer);
					primaryBuffer->mUnExcuteSecondCmdBufferCount++;
				}
				//
				mCmdBufferTaskQueue->PushTask(CmdBufferTask(cmdBuffer, secondCmdBuffers[i], VKCommandBuffer::ExcuteParallel));
			}
			else {
				secondBuffer->MergeTo(primaryBuffer);
			}
		}
		//
		primaryBuffer->PushCommand(new VKCmdEndSubRenderPass());
	}

	void VulkanGI::EndRenderPass(CommandBuffer* cmdBuffer, RenderPass* renderPass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) {
		auto primaryBuffer = VKCommandBuffer::Cast(cmdBuffer);
		//
		for (uint32_t i = 0; i < numSecondCmdBuffer; ++i) {
			auto secondBuffer = VKCommandBuffer::Cast(secondCmdBuffers[i]);
			if (mCmdBufferTaskQueue != nullptr && secondBuffer->mBindingCmdBuffer == nullptr) {
				secondBuffer->mBindingCmdBuffer = mCmdBufferManger->AcquireSecondCmdBuffer(VK_PIPELINE_BIND_POINT_GRAPHICS);
			}
			//
			if (mCmdBufferTaskQueue != nullptr) {
				primaryBuffer->mSecondCmdBuffers.push_back(secondCmdBuffers[i]);
				//
				{
					std::lock_guard<std::mutex> lock(primaryBuffer->mMtxSecondCmdBuffer);
					primaryBuffer->mUnExcuteSecondCmdBufferCount++;
				}
				//
				mCmdBufferTaskQueue->PushTask(CmdBufferTask(cmdBuffer, secondCmdBuffers[i], VKCommandBuffer::ExcuteParallel));
			}
			else {
				secondBuffer->MergeTo(primaryBuffer);
			}
		}
		//
		primaryBuffer->PushCommand(new VKCmdEndRenderPass());
	}

	bool VulkanGI::BeginComputePass(CommandBuffer* cmdBuffer, ComputePass* computePass) {
		VKCommandBuffer::Cast(cmdBuffer)->PushCommand(new VKCmdBeginComputePass());
		return true;
	}

	void VulkanGI::EndComputePass(CommandBuffer* cmdBuffer, ComputePass* computePass, uint32_t numSecondCmdBuffer, CommandBuffer** secondCmdBuffers) {
		auto primaryBuffer = VKCommandBuffer::Cast(cmdBuffer);
		primaryBuffer->PushCommand(new VKCmdEndSubRenderPass());
		//
		for (uint32_t i = 0; i < numSecondCmdBuffer; ++i) {
			{
				std::lock_guard<std::mutex> lock(primaryBuffer->mMtxSecondCmdBuffer);
				primaryBuffer->mUnExcuteSecondCmdBufferCount++;
			}
			//
			mCmdBufferTaskQueue->PushTask(CmdBufferTask(cmdBuffer, secondCmdBuffers[i], VKCommandBuffer::ExcuteParallel));
		}
	}
}