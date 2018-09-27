#include "VulkanResource.h"
#include "VulkanGI.h"

#include "third_lib\SPIRV-Cross\spirv_cross.hpp"

#ifdef _WIN32
#pragma comment(lib, "VulkanSDK\\1.1.77.0\\Lib\\vulkan-1.lib")
#endif

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
		VkInstanceCreateInfo instance_create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			nullptr,
			0,
			&application_info,
			0,
			nullptr,
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
		uint32_t queue_families_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(mVkPhysicalDevice, &queue_families_count, nullptr);
		if (queue_families_count == 0) {
			return false;
		}
		std::vector<VkQueueFamilyProperties> queue_families;
		queue_families.resize(queue_families_count);
		vkGetPhysicalDeviceQueueFamilyProperties(mVkPhysicalDevice, &queue_families_count, &queue_families[0]);
		//
		int queueFamilyIndex = -1;
		for (int i = 0; i < queue_families.size(); ++i) {
#ifdef _WIN32
			if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && vkGetPhysicalDeviceWin32PresentationSupportKHR(mVkPhysicalDevice, i)) {
				queueFamilyIndex = i;
				break;
			}
#endif
		}
		//
		if (queueFamilyIndex < 0) {
			return false;
		}
		//
		VkDeviceQueueCreateInfo graphics_queue_create_info = {
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			queueFamilyIndex,
			1,
			nullptr
		};
		//
		vkGetPhysicalDeviceMemoryProperties(mVkPhysicalDevice, &mVkDeviceMemoryProperties);
		//
		std::vector<char const *> desired_extensions;
		desired_extensions.push_back("VK_KHR_swapchain");
		VkPhysicalDeviceFeatures desired_features;
		memset(&desired_features, 0, sizeof(VkPhysicalDeviceFeatures));
		desired_features.tessellationShader = VK_TRUE;
		desired_features.geometryShader = VK_TRUE;
		VkDeviceCreateInfo device_create_info = {
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			nullptr,
			0,
			1,
			&graphics_queue_create_info,
			0,
			nullptr,
			static_cast<uint32_t>(desired_extensions.size()),
			desired_extensions.size() > 0 ? &desired_extensions[0] : nullptr,
			&desired_features
		};
		//
		result = vkCreateDevice(mVkPhysicalDevice, &device_create_info, nullptr, &mVkLogicDevice);
		if ((result != VK_SUCCESS) || (mVkLogicDevice == VK_NULL_HANDLE)) {
			return false;
		}
		//
		vkGetDeviceQueue(mVkLogicDevice, 0, 0, &mVkGraphicsQueue);
		//
		return true;
	}

	bool VulkanGI::Init(const char* device_name) {
		GSupportParallelCommandBuffer = true;
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
		//
		if (!createLogicDevice(device_name)) {
			return false;
		}
		//
		return VKMemoryManager::Instance()->Init(mVkPhysicalDevice, mVkLogicDevice);
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
		VKShaderModule* psm = new VKShaderModule();
		//
		VkShaderModuleCreateInfo shader_module_create_info;
		shader_module_create_info.sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.pNext = nullptr;
		shader_module_create_info.flags = 0;
		shader_module_create_info.codeSize = buffer.size() * sizeof(uint32_t);
		shader_module_create_info.pCode = (const uint32_t*)(buffer.data());
		if (vkCreateShaderModule(mVkLogicDevice, &shader_module_create_info, nullptr, &psm->mShaderModule) != VkResult::VK_SUCCESS)
		{
			delete psm;
			return nullptr;
		}
		//
		psm->mSpirvCompiler = std::unique_ptr<spirv_cross::Compiler>(new spirv_cross::Compiler(std::move(buffer)));
		//
		return psm;
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
			subpassDescriptions[i].flags = 0;
		}
		//
		std::vector<VkSubpassDependency> dependencies(create_info.dependencies.size());
		for (int i = 0; i < dependencies.size(); ++i) {
			auto &dependence = create_info.dependencies[i];
			//
			dependencies[i].dependencyFlags = 0;
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
		if (vkCreateRenderPass(mVkLogicDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			return nullptr;
		}
		//
		auto pres = new VKRenderPass();
		pres->mVkRenderPass = renderPass;
		return pres;
	}

	GraphicsPipeline* VulkanGI::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& create_info) {
		if (create_info.shaderStage.pVertexShader == nullptr || create_info.shaderStage.pFragmentShader == nullptr) {
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
			1.0f,                                                               // float                                          minSampleShading
			nullptr,                                                          // const VkSampleMask                            *pSampleMask
			VK_FALSE,                                                     // VkBool32                                       alphaToCoverageEnable
			VK_FALSE                                                      // VkBool32                                       alphaToOneEnable
		};

		std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states;
		for (auto &itr : create_info.colorBlendState.Attachments) {
			VkPipelineColorBlendAttachmentState tmp;
			tmp.alphaBlendOp = (VkBlendOp)itr.alphaBlendOp;
			tmp.blendEnable = itr.blendEnable;
			tmp.colorBlendOp = (VkBlendOp)itr.colorBlendOp;
			tmp.colorWriteMask = itr.colorWriteMask;
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
			VK_DYNAMIC_STATE_STENCIL_REFERENCE,
			VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV,
			VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT,
			VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT
		};
		VkPipelineDynamicStateCreateInfo dynamic_state_creat_info = {
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(dynamic_states.size()),
			dynamic_states.data()
		};
		//
		std::list<spirv_cross::EntryPoint> enteryPoints;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		//
		std::unordered_map<uint8_t, std::vector<VkDescriptorSetLayoutBinding> > descriptorSets;
		std::vector<VkPushConstantRange> pushConstantRanges;
		//
		auto collectResource = [&](spirv_cross::Compiler* pspirvCompiler, uint32_t stageFlag, VkShaderModule shaderModule)->void {
			auto vs_resource = pspirvCompiler->get_shader_resources();
			for (auto &ubo : vs_resource.uniform_buffers)
			{
				descriptorSets[pspirvCompiler->get_decoration(ubo.id, spv::Decoration::DecorationDescriptorSet)].push_back({
					pspirvCompiler->get_decoration(ubo.id, spv::Decoration::DecorationBinding),
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					1,
					stageFlag,
					NULL,
				});
			};
			for (auto &simpler : vs_resource.sampled_images) {
				descriptorSets[pspirvCompiler->get_decoration(simpler.id, spv::Decoration::DecorationDescriptorSet)].push_back({
					pspirvCompiler->get_decoration(simpler.id, spv::Decoration::DecorationBinding),
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					1,
					stageFlag,
					NULL,
				});
			}
			for (auto &pushConstant : vs_resource.push_constant_buffers) {
				auto ranges = pspirvCompiler->get_active_buffer_ranges(pushConstant.id);
				auto type = pspirvCompiler->get_type(pushConstant.type_id);
				auto size = pspirvCompiler->get_declared_struct_size(type);
				//
				VkPushConstantRange tmp;
				tmp.offset = ranges[0].offset;
				tmp.size = size - tmp.offset;
				tmp.stageFlags = stageFlag;
				pushConstantRanges.push_back(tmp);
			}
			//
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = (VkShaderStageFlagBits)stageFlag;
			shaderStage.module = shaderModule;
			enteryPoints.push_back(std::move(pspirvCompiler->get_entry_points_and_stages()[0]));
			shaderStage.pName = enteryPoints.back().name.c_str();
			shaderStages.push_back(shaderStage);
		};
		//
		if (create_info.shaderStage.pVertexShader != nullptr) {
			auto pshader = (VKShaderModule*)create_info.shaderStage.pVertexShader;
			collectResource(pshader->mSpirvCompiler.get(), VK_SHADER_STAGE_VERTEX_BIT, pshader->mShaderModule);
		}
		if (create_info.shaderStage.pTessControlShader != nullptr) {
			auto pshader = (VKShaderModule*)create_info.shaderStage.pTessControlShader;
			collectResource(pshader->mSpirvCompiler.get(), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, pshader->mShaderModule);
		}
		if (create_info.shaderStage.pTessEvaluationShader != nullptr) {
			auto pshader = (VKShaderModule*)create_info.shaderStage.pTessEvaluationShader;
			collectResource(pshader->mSpirvCompiler.get(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, pshader->mShaderModule);
		}
		if (create_info.shaderStage.pGeomteryShader != nullptr) {
			auto pshader = (VKShaderModule*)create_info.shaderStage.pGeomteryShader;
			collectResource(pshader->mSpirvCompiler.get(), VK_SHADER_STAGE_GEOMETRY_BIT, pshader->mShaderModule);
		}
		if (create_info.shaderStage.pFragmentShader != nullptr) {
			auto pshader = (VKShaderModule*)create_info.shaderStage.pFragmentShader;
			collectResource(pshader->mSpirvCompiler.get(), VK_SHADER_STAGE_FRAGMENT_BIT, pshader->mShaderModule);
		}
		//
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts(descriptorSets.size());
		for (auto &dsb : descriptorSets) {
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
			descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCreateInfo.pBindings = dsb.second.data();
			descriptorSetLayoutCreateInfo.bindingCount = dsb.second.size();

			if (vkCreateDescriptorSetLayout(mVkLogicDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayouts[dsb.first]) != VK_SUCCESS) {
				return nullptr;
			}
		}
		//
		VkPipelineLayout pipelineLayout;
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRanges.size();
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
		if (vkCreatePipelineLayout(mVkLogicDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
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
		if (vkCreateGraphicsPipelines(mVkLogicDevice, nullptr, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			return nullptr;
		}
		//
		auto pres = new VKGraphicsPipeline();
		pres->mVkPipeLine = graphicsPipeline;
		pres->mVkPipelineLayout = pipelineLayout;
		//
		return pres;
	}

	Swapchain* VulkanGI::CreateSwapchain(const SwapchainCreateInfo& create_info) {
		if (mSwapchain == nullptr) {
			mSwapchain = new  VKSwapchain();
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
		result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mVkPhysicalDevice, mSwapchain->mVkSurface, &mSwapchain->mVkSurfaceCapabilities);
		if (result != VK_SUCCESS) {
			return nullptr;
		}
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
		result = vkCreateSwapchainKHR(mVkLogicDevice, &swapchain_create_info, nullptr, &mSwapchain->mVkSwapchain);
		if (result != VK_SUCCESS) {
			return nullptr;
		}
		//
		return mSwapchain;
	}

	bool VulkanGI::createBuffer(uint64_t size, VkBufferUsageFlags usageFlags, uint32_t createFlags, VKBuffer* pres) {
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = size;
		bufferInfo.usage = usageFlags;
		if (VKMemoryManager::Instance()->CreateBuffer(bufferInfo, &pres->mVkBuffer, (VKMemory::MemoryUsage)createFlags, pres->mMemory) != VK_SUCCESS) {
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
		vkCmdCopyBuffer(mVkTemporaryCommandBuffer, stagingBuffer, buffer->mVkBuffer, 1, &vbCopyRegion);
		//
		auto res = EndSingleTimeCommands();
		VKMemoryManager::Instance()->DestoryBuffer(stagingBuffer, pmemory);
		//
		return res;
	}

	bool VulkanGI::BeginSingleTimeCommands() {
		VkCommandBufferBeginInfo cmdBufBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		if (vkBeginCommandBuffer(mVkTemporaryCommandBuffer, &cmdBufBeginInfo) == VK_SUCCESS) {
			return true;
		}
		//
		return false;
	}

	bool VulkanGI::EndSingleTimeCommands() {
		vkEndCommandBuffer(mVkTemporaryCommandBuffer);

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mVkTemporaryCommandBuffer;

		if (vkQueueSubmit(mVkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS &&
			vkQueueWaitIdle(mVkGraphicsQueue) != VK_SUCCESS) {
			return false;
		}
		//
		return true;
	}

	VertexBuffer* VulkanGI::CreateVertexBuffer(uint64_t size, BufferUsageFlags usageFlags) {
		auto pres = new  VKVertexBuffer();
		if (!createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VKMemory::MemoryUsage::VK_MEMORY_USAGE_GPU_ONLY, pres)) {
			delete pres;
			return nullptr;
		}
		//
		return pres;
	}

	IndexBuffer* VulkanGI::CreateIndexBuffer(uint32_t size, BufferUsageFlags usageFlags) {
		auto pres = new  VKIndexBuffer();
		if (!createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VKMemory::MemoryUsage::VK_MEMORY_USAGE_GPU_ONLY, pres)) {
			delete pres;
			return nullptr;
		}
		//
		return pres;
	}

	UniformBuffer* VulkanGI::CreateUniformBuffer(uint32_t size, BufferUsageFlags usageFlags) {
		auto pres = new  VKUniformBuffer();
		if (!createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VKMemory::MemoryUsage::VK_MEMORY_USAGE_GPU_ONLY, pres)) {
			delete pres;
			return nullptr;
		}
		//
		return pres;
	}

	BufferUpdateContext* VulkanGI::BeginUpdateBuffer() {
		return new VKBufferUpdateContext();
	}

	bool VulkanGI::EndUpdateBuffer(BufferUpdateContext* pUpdateContext) {
		std::vector<std::array<void*, 3> >copys(((VKBufferUpdateContext*)pUpdateContext)->updates.size());
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
			vbCopyRegion.dstOffset = ((VKBuffer*)itm[0])->mMemory->GetOffset();
			vbCopyRegion.size = ((VKBuffer*)itm[0])->mMemory->GetSize();
			vkCmdCopyBuffer(mVkTemporaryCommandBuffer, (VkBuffer)itm[1], ((VKBuffer*)(itm[0]))->mVkBuffer, 1, &vbCopyRegion);
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

	void VulkanGI::UpdateVertexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext) {
		updateBuffer((VKBuffer*)pbuffer, offset, size, pdata, pUpdateContext);
	}

	void VulkanGI::UpdateIndexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext) {
		updateBuffer((VKBuffer*)pbuffer, offset, size, pdata, pUpdateContext);
	}

	void VulkanGI::UpdateUniformBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, void* pdata, BufferUpdateContext* pUpdateContext) {
		updateBuffer((VKBuffer*)pbuffer, offset, size, pdata, pUpdateContext);
	}

	void* VulkanGI::MapVertexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode) {
		return nullptr;
	}

	void VulkanGI::UnMapVertexBuffer(VertexBuffer* pbuffer) {
	}

	void* VulkanGI::MapIndexBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode) {
		return nullptr;
	}

	void VulkanGI::UnMapIndexBuffer(VertexBuffer* pbuffer) {

	}

	void* VulkanGI::MapUniformBuffer(VertexBuffer* pbuffer, uint32_t offset, uint32_t size, MapMode mapMode) {
		return nullptr;
	}

	void VulkanGI::UnMapUniformBuffer(VertexBuffer* pbuffer) {

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
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageCreateInfo.extent = { sizeX, sizeY, 1 };
		imageCreateInfo.usage = (usageFlags<< 2) | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		
		VkImage vkImage;
		VKMemory* pmemory = nullptr;
		if (VKMemoryManager::Instance()->CreateImage(imageCreateInfo, &vkImage, VKMemory::MemoryUsage::VK_MEMORY_USAGE_GPU_ONLY, pmemory) != VK_SUCCESS) {
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
		auto pres = new VKImage2D(format, sizeX, sizeY, numMips);
		pres->mVkImage = vkImage;
		pres->mMemory = pmemory;
		pres->mUsageFlag = usageFlags;
		pres->mOrgiView = new VKImageView(pres, imageView, imageViewInfo);
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
		return new VKImageUpdateContext();
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
			mVkTemporaryCommandBuffer,
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
			mVkTemporaryCommandBuffer,
			stagingBuffer,
			((VKImage2D*)pimg)->mVkImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferCopyRegion);
		//
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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
		vkCmdPipelineBarrier(
			mVkTemporaryCommandBuffer,
			GetImageBarrierFlags(((VKImage2D*)pimg)->mLayoutBarrier[level], imageMemoryBarrier.srcAccessMask, imageMemoryBarrier.oldLayout),
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


	CommandBuffer* VulkanGI::CreateCommandBuffer(const CommandBufferCreateInfo& create_info) {
		return nullptr;
	}

	void VulkanGI::CmdSetViewport(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdSetScissor(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdSetLineWidth(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdBindGraphicsPipeline(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdBindIndexBuffer(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdBindVertexBuffer(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdFillBuffer(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdUpdateBuffer(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdClearColorImage(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdClearDepthStencilImage(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdDraw(CommandBuffer& commandBuffer) {
	}

	void VulkanGI::CmdDrawIndexed(CommandBuffer& commandBuffer) {
	}
}