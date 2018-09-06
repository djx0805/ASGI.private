#pragma once

#include "ASGI.hpp"
#include "GPUResource.h"

namespace ASGI {
	class DynamicGI {
	public:
		virtual ~DynamicGI() {}

		virtual bool Init(const char* device_name = nullptr) = 0;
		//graphics pipeline
		virtual VertexShader* CreateVertexShader() = 0;
		virtual TessControlShader* CreateTessControlShader() = 0;
		virtual TessEvaluationShader* CreateTessEvaluationShader() = 0;
		virtual GeometryShader* CreateGeometryShader() = 0;
		virtual FragmentShader* CreateFragmentShader() = 0;
		virtual GraphicsPipeline* CreateGraphicsPipeline() = 0;
		virtual Swapchain* CreateSwapchain(void* windowHandle, unsigned int sizeX, unsigned int sizeY, bool bIsFullscreen, GIFormat pixelFormat, Swapchain* oldSwapchain) = 0;
		//render resource
		virtual VertexBuffer* CreateVertexBuffer(const VertexBufferCreateInfo& create_info) = 0;
		virtual IndexBuffer* CreateIndexBuffer(const IndexBufferCreateInfo& create_info) = 0;
		virtual UniformBuffer* CreateUniformBuffer() = 0;
		virtual Texture2D* CreateTexture2D(const Texture2DCreateInfo& create_info) = 0;
		//render command
		virtual CommandBuffer* CreateCommandBuffer(const CommandBufferCreateInfo& create_info) = 0;
		virtual void CmdSetViewport(CommandBuffer& commandBuffer) = 0;
		virtual void CmdSetScissor(CommandBuffer& commandBuffer) = 0;
		virtual void CmdSetLineWidth(CommandBuffer& commandBuffer) = 0;
		virtual void CmdBindGraphicsPipeline(CommandBuffer& commandBuffer) = 0;
		virtual void CmdBindIndexBuffer(CommandBuffer& commandBuffer) = 0;
		virtual void CmdBindVertexBuffer(CommandBuffer& commandBuffer) = 0;
		virtual void CmdFillBuffer(CommandBuffer& commandBuffer) = 0;
		virtual void CmdUpdateBuffer(CommandBuffer& commandBuffer) = 0;
		virtual void CmdClearColorImage(CommandBuffer& commandBuffer) = 0;
		virtual void CmdClearDepthStencilImage(CommandBuffer& commandBuffer) = 0;
		virtual void CmdDraw(CommandBuffer& commandBuffer) = 0;
		virtual void CmdDrawIndexed(CommandBuffer& commandBuffer) = 0;
	};
}
