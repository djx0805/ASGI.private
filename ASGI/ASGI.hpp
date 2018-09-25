#pragma once

#ifdef ASGI_EXPORTS
#define ASGI_API __declspec(dllexport)
#else
#define ASGI_API __declspec(dllimport)
#endif

#include <vector>
#include "GPUResource.h"

namespace ASGI {
	extern ASGI_API bool GSupportParallelCommandBuffer;

	struct Viewport {
		float    x;
		float    y;
		float    width;
		float    height;
		float    minDepth;
		float    maxDepth;
	};

	struct Offset2D {
		int32_t    x;
		int32_t    y;
	};

	struct Extent2D {
		uint32_t    width;
		uint32_t    height;
	};

	struct Rect2D {
		Offset2D    offset;
		Extent2D    extent;
	};

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

	struct PipelineShaderStage {
		ShaderModule* pVertexShader;
		ShaderModule* pGeomteryShader;
		ShaderModule* pTessControlShader;
		ShaderModule* pTessEvaluationShader;
		ShaderModule* pFragmentShader;
		PipelineShaderStage() {
			pVertexShader = nullptr;
			pGeomteryShader = nullptr;
			pTessControlShader = nullptr;
			pTessEvaluationShader = nullptr;
			pFragmentShader = nullptr;
		}
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
		PrimitiveTopology toplogy = PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		bool primitiveRestartEnable = true;
	};

	struct PipelineRasterizationState {
		bool                                 depthClampEnable = false;
		bool                                   rasterizerDiscardEnable = false;
		PolygonMode                              polygonMode = PolygonMode::POLYGON_MODE_FILL;
		CullModeFlags                            cullMode = CullModeFlagBits::CULL_MODE_BACK_BIT;
		FrontFace                                frontFace = FrontFace::FRONT_FACE_COUNTER_CLOCKWISE;
		bool                                 depthBiasEnable = false;
	};

	struct PipelineMultisampleState {
		SampleCountFlagBits                    rasterizationSamples = SampleCountFlagBits::SAMPLE_COUNT_1_BIT;
	};

	struct StencilOpState {
		StencilOp    failOp = StencilOp::STENCIL_OP_KEEP;
		StencilOp    passOp = StencilOp::STENCIL_OP_KEEP;
		StencilOp    depthFailOp = StencilOp::STENCIL_OP_KEEP;
		CompareOp    compareOp = CompareOp::COMPARE_OP_NEVER;
		uint32_t       compareMask = 0;
		uint32_t       writeMask = 0;
		uint32_t       reference = 0;
	};

	struct PipelineDepthStencilState {
		bool                                 depthTestEnable = true;
		bool                                  depthWriteEnable = true;
		CompareOp                               depthCompareOp = CompareOp::COMPARE_OP_LESS;
		bool                                depthBoundsTestEnable = false;
		bool                                stencilTestEnable = false;
		StencilOpState                          front;
		StencilOpState                          back;
		float                                     minDepthBounds = 0.0f;
		float                                     maxDepthBounds = 1.0f;
	};

	struct PipelineColorBlendAttachmentState {
		bool                       blendEnable = false;
		BlendFactor            srcColorBlendFactor;
		BlendFactor            dstColorBlendFactor;
		BlendOp                colorBlendOp;
		BlendFactor            srcAlphaBlendFactor;
		BlendFactor            dstAlphaBlendFactor;
		BlendOp                alphaBlendOp;
		uint32_t                 colorWriteMask;
	};

	struct PipelineColorBlendState {
		bool                                                                                    logicOpEnable = false;
		LogicOp                                                                             logicOp = LogicOp::LOGIC_OP_COPY;
		std::vector<PipelineColorBlendAttachmentState>  Attachments;
		float                                                                                     blendConstants[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	struct GraphicsPipelineCreateInfo {
		uint8_t viewportCount = 1;
		PipelineShaderStage shaderStage;
		PipelineVertexDeclaration vertexDeclaration;
		PipelineInputAssemblyState inputAssemblyState;
		PipelineRasterizationState rasterizationState;
		PipelineMultisampleState multisampleState;
		PipelineDepthStencilState depthStencilState;
		PipelineColorBlendState colorBlendState;
		RenderPass* renderPass;
		uint8_t subpassIndex = 0;
	};

	struct CommandBufferCreateInfo {

	};

	class BufferUpdateContext {
	public:
		virtual ~BufferUpdateContext() {}
	};

	class ImageUpdateContext {
	public:
		virtual ~ImageUpdateContext() {}
	};

	struct  Texture2DCreateInfo
	{

	};
}