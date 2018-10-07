#pragma once

#ifdef ASGI_EXPORTS
#define ASGI_API __declspec(dllexport)
#else
#define ASGI_API __declspec(dllimport)
#endif

#include <vector> 
#include "Resource.h"

namespace ASGI {
	extern ASGI_API bool GSupportParallelCommandBuffer;

	
	struct ShaderModuleCreateInfo {
		const char* path;
		uint32_t codeSize;
		char* pcode;
	};

	struct SwapchainCreateInfo {
		void* windowHandle;
		uint32_t sizeX;
		uint32_t sizeY;
		bool isFullScreen;
		Format preferredPixelFormat;
		Format preferredDepthStencilFormat;
		bool vsync;
	};

	struct AttachmentDescription {
		Format format;
		SampleCountFlagBits samples;
		AttachmentLoadOp loadOp;
		AttachmentStoreOp storeOp; 
		AttachmentLoadOp stencilLoadOp;
		AttachmentStoreOp stencilStoreOp;
		ImageLayout initialLayout;
		ImageLayout finalLayout;
	};

	struct AttachmentReference {
		int        attachment;
		ImageLayout    layout;
	};

	struct SubpassDescription {
		std::vector<AttachmentReference> inputAttachments;
		std::vector<AttachmentReference> colorAttachments;
		AttachmentReference depthStencilAttachment;
	};

	struct SubpassDependency {
		uint32_t                srcSubpass;
		uint32_t                dstSubpass;
		PipelineStageFlags    srcStageMask;
		PipelineStageFlags    dstStageMask;
		AccessFlags           srcAccessMask;
		AccessFlags           dstAccessMask;
	};

	struct RenderPassCreateInfo {
		std::vector<AttachmentDescription> attachments;
		std::vector<SubpassDescription> subpasses;
		std::vector<SubpassDependency> dependencies;
	};

	struct VertexInputItem {
		uint8_t bindingNumber;
		uint8_t offset;
		VertexFormat format;
		uint8_t location;
	};

	struct PipelineVertexDeclaration {
		std::vector<VertexInputItem> vertexInputs;
	};

	struct PipelineInputAssemblyState {
		PrimitiveTopology toplogy;
		bool primitiveRestartEnable;
		//
		PipelineInputAssemblyState() {
			toplogy = PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			primitiveRestartEnable = true;
		}
	};

	struct PipelineRasterizationState {
		bool                                 depthClampEnable;
		bool                                 rasterizerDiscardEnable;
		PolygonMode                   polygonMode;
		CullModeFlags                  cullMode;
		FrontFace                          frontFace;
		bool                                   depthBiasEnable;
		//
		PipelineRasterizationState() {
			depthClampEnable = false;
			rasterizerDiscardEnable = false;
			polygonMode = PolygonMode::POLYGON_MODE_FILL;
			cullMode = CullModeFlagBits::CULL_MODE_NONE;
			frontFace = FrontFace::FRONT_FACE_COUNTER_CLOCKWISE;
			depthBiasEnable = false;
		}
	};

	struct PipelineMultisampleState {
		SampleCountFlagBits                    rasterizationSamples;
		//
		PipelineMultisampleState() {
			rasterizationSamples = SampleCountFlagBits::SAMPLE_COUNT_1_BIT;
		}
	};

	struct StencilOpState {
		StencilOp    failOp;
		StencilOp    passOp;
		StencilOp    depthFailOp;
		CompareOp    compareOp;
		uint32_t       compareMask;
		uint32_t       writeMask;
		uint32_t       reference;
		//
		StencilOpState() {
			failOp = StencilOp::STENCIL_OP_KEEP;
			passOp = StencilOp::STENCIL_OP_KEEP;
			depthFailOp = StencilOp::STENCIL_OP_KEEP;
			compareOp = CompareOp::COMPARE_OP_NEVER;
			compareMask = 0;
			writeMask = 0;
			reference = 0;
		}
	};

	struct PipelineDepthStencilState {
		bool                                 depthTestEnable;
		bool                                  depthWriteEnable;
		CompareOp                               depthCompareOp;
		bool                                depthBoundsTestEnable;
		bool                                stencilTestEnable;
		StencilOpState                          front;
		StencilOpState                          back;
		float                                     minDepthBounds;
		float                                     maxDepthBounds;
		//
		PipelineDepthStencilState() {
			depthTestEnable = true;
			depthWriteEnable = true;
			depthCompareOp = CompareOp::COMPARE_OP_LESS;
			depthBoundsTestEnable = false;
			stencilTestEnable = false;
			minDepthBounds = 0.0f;
			maxDepthBounds = 1.0f;
		}
	};

	struct PipelineColorBlendAttachmentState {
		bool                       blendEnable;
		BlendFactor            srcColorBlendFactor;
		BlendFactor            dstColorBlendFactor;
		BlendOp                colorBlendOp;
		BlendFactor            srcAlphaBlendFactor;
		BlendFactor            dstAlphaBlendFactor;
		BlendOp                 alphaBlendOp;
		ColorComponentFlags        colorWriteMask;
		//
		PipelineColorBlendAttachmentState() {
			blendEnable = false;
			colorWriteMask = 0xf;
			srcColorBlendFactor = BlendFactor::BLEND_FACTOR_ZERO;
			dstColorBlendFactor = BlendFactor::BLEND_FACTOR_ONE;
			colorBlendOp = BlendOp::BLEND_OP_ADD;
			srcAlphaBlendFactor = BlendFactor::BLEND_FACTOR_ZERO;
			dstAlphaBlendFactor = BlendFactor::BLEND_FACTOR_ONE;
			alphaBlendOp = BlendOp::BLEND_OP_ADD;
		}
	};

	struct PipelineColorBlendState {
		bool                                                                                    logicOpEnable;
		LogicOp                                                                             logicOp;
		std::vector<PipelineColorBlendAttachmentState>  Attachments;
		float                                                                                     blendConstants[4];
		//
		PipelineColorBlendState() {
			logicOpEnable = false;
			logicOp = LogicOp::LOGIC_OP_COPY;
			blendConstants[0] = 0.0f;
			blendConstants[1] = 0.0f;
			blendConstants[2] = 0.0f;
			blendConstants[3] = 0.0f;
		}
	};

	struct GraphicsPipelineCreateInfo {
		uint8_t viewportCount;
		GPUProgram* gpuProgram;
		PipelineVertexDeclaration vertexDeclaration;
		PipelineInputAssemblyState inputAssemblyState;
		PipelineRasterizationState rasterizationState;
		PipelineMultisampleState multisampleState;
		PipelineDepthStencilState depthStencilState;
		PipelineColorBlendState colorBlendState;
		RenderPass* renderPass;
		uint8_t subpassIndex;
		//
		GraphicsPipelineCreateInfo() {
			viewportCount = 1;
			subpassIndex = 0;
		}
	};

	struct CommandBufferCreateInfo {

	};

	

	struct  Texture2DCreateInfo
	{

	};
}