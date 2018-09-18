#include "VulkanResource.h"
#include "VulkanGI.h"

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
		return true;
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

	GraphicsPipeline* VulkanGI::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& create_info) {
		if (create_info.shaderStage.pVertexShader == nullptr || create_info.shaderStage.pFragmentShader == nullptr) {
			return nullptr;
		}
		//
		spirv_cross::ShaderResources vertexShaderRS = ((VKShaderModule*)create_info.shaderStage.pVertexShader)->mSpirvCompiler->get_shader_resources();
		spirv_cross::ShaderResources fragmentShaderRS = ((VKShaderModule*)create_info.shaderStage.pFragmentShader)->mSpirvCompiler->get_shader_resources();
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
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			false
		};
		//
		VkPipelineViewportStateCreateInfo viewport_state_create_info = {
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			nullptr,
			0,
			1,
			nullptr, //&view_port,
			1,
			nullptr //&dummyScissor
		};
		//
		VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,     // VkStructureType                                sType
			nullptr,                                                      // const void                                    *pNext
			0,                                                            // VkPipelineMultisampleStateCreateFlags          flags
			VK_SAMPLE_COUNT_1_BIT,                                        // VkSampleCountFlagBits                          rasterizationSamples
			VK_FALSE,                                                     // VkBool32                                       sampleShadingEnable
			1.0f,                                                         // float                                          minSampleShading
			nullptr,                                                      // const VkSampleMask                            *pSampleMask
			VK_FALSE,                                                     // VkBool32                                       alphaToCoverageEnable
			VK_FALSE                                                      // VkBool32                                       alphaToOneEnable
		};

		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
			VK_FALSE,                                                     // VkBool32                                       blendEnable
			VK_BLEND_FACTOR_ONE,                                          // VkBlendFactor                                  srcColorBlendFactor
			VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  dstColorBlendFactor
			VK_BLEND_OP_ADD,                                              // VkBlendOp                                      colorBlendOp
			VK_BLEND_FACTOR_ONE,                                          // VkBlendFactor                                  srcAlphaBlendFactor
			VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  dstAlphaBlendFactor
			VK_BLEND_OP_ADD,                                              // VkBlendOp                                      alphaBlendOp
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |         // VkColorComponentFlags                          colorWriteMask
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,     // VkStructureType                                sType
			nullptr,                                                      // const void                                    *pNext
			0,                                                            // VkPipelineColorBlendStateCreateFlags           flags
			VK_FALSE,                                                     // VkBool32                                       logicOpEnable
			VK_LOGIC_OP_COPY,                                             // VkLogicOp                                      logicOp
			1,                                                            // uint32_t                                       attachmentCount
			&color_blend_attachment_state,                                // const VkPipelineColorBlendAttachmentState     *pAttachments
		{ 0.0f, 0.0f, 0.0f, 0.0f }                                    // float                                          blendConstants[4]
		};
		//
		VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info = {};
		pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipeline_rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline_rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE;
		pipeline_rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
		pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
		pipeline_rasterization_state_create_info.depthBiasEnable = VK_FALSE;
		pipeline_rasterization_state_create_info.lineWidth = 1.0f;
		//
		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
		pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
		pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
		pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
		pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.front = pipelineDepthStencilStateCreateInfo.back;
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
			VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT,
			VK_DYNAMIC_STATE_BEGIN_RANGE,
			VK_DYNAMIC_STATE_END_RANGE,
			VK_DYNAMIC_STATE_RANGE_SIZE
		};
		VkPipelineDynamicStateCreateInfo dynamic_state_creat_info = {
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(dynamic_states.size()),
			dynamic_states.data()
		};
		//
		std::unordered_map<uint8_t, std::vector<VkDescriptorSetLayoutBinding> > descriptorSets;
		//vertex shader resource;
		{
			auto vs_spirv_Compiler = ((VKShaderModule*)create_info.shaderStage.pVertexShader)->mSpirvCompiler.get();
			auto vs_resource = vs_spirv_Compiler->get_shader_resources();
			for (auto &ubo : vs_resource.uniform_buffers)
			{
				descriptorSets[vs_spirv_Compiler->get_decoration(ubo.id, spv::Decoration::DecorationDescriptorSet)].push_back({
					vs_spirv_Compiler->get_decoration(ubo.id, spv::Decoration::DecorationBinding),
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					1,
					VK_SHADER_STAGE_VERTEX_BIT,
					NULL,
					});
			};
			for (auto &simpler : vs_resource.sampled_images) {
				descriptorSets[vs_spirv_Compiler->get_decoration(simpler.id, spv::Decoration::DecorationDescriptorSet)].push_back({
					vs_spirv_Compiler->get_decoration(simpler.id, spv::Decoration::DecorationBinding),
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					1,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					NULL,
				});
			}
		}
		//fragment shader resource;
		{
			auto fg_spirv_Compiler = ((VKShaderModule*)create_info.shaderStage.pFragmentShader)->mSpirvCompiler.get();
			auto fg_resource = fg_spirv_Compiler->get_shader_resources();
			for (auto &ubo : fg_resource.uniform_buffers)
			{
				descriptorSets[fg_spirv_Compiler->get_decoration(ubo.id, spv::Decoration::DecorationDescriptorSet)].push_back({
					fg_spirv_Compiler->get_decoration(ubo.id, spv::Decoration::DecorationBinding),
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					1,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					NULL,
					});
			};
			for (auto &simpler : fg_resource.sampled_images) {
				descriptorSets[fg_spirv_Compiler->get_decoration(simpler.id, spv::Decoration::DecorationDescriptorSet)].push_back({
					fg_spirv_Compiler->get_decoration(simpler.id, spv::Decoration::DecorationBinding),
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					1,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					NULL,
					});
			}
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
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		return nullptr;
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

	VertexBuffer* VulkanGI::CreateVertexBuffer(const VertexBufferCreateInfo& create_info) {
		return nullptr;
	}

	IndexBuffer* VulkanGI::CreateIndexBuffer(const IndexBufferCreateInfo& create_info) {
		return nullptr;
	}

	UniformBuffer* VulkanGI::CreateUniformBuffer(const UniformBufferCreateInfo& create_info) {
		return nullptr;
	}

	Texture2D* VulkanGI::CreateTexture2D(const Texture2DCreateInfo& create_info) {
		return nullptr;
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