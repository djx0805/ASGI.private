// ASGI.cpp : 定义 DLL 应用程序的导出函数。
//
#include "ASGI.h"
#include "VulkanGI.h"

namespace ASGI {
	bool GSupportParallelCommandBuffer = false;

	DynamicGI* pGI = nullptr;

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

	VertexBuffer* CreateVertexBuffer(const VertexBufferCreateInfo& create_info) {
		return pGI->CreateVertexBuffer(create_info);
	}

	IndexBuffer* CreateIndexBuffer(const IndexBufferCreateInfo& create_info) {
		return pGI->CreateIndexBuffer(create_info);
	}

	Texture2D* CreateTexture2D(const Texture2DCreateInfo& create_info) {
		return pGI->CreateTexture2D(create_info);
	}

	CommandBuffer* CreateCommandBuffer(const CommandBufferCreateInfo& create_info) {
		return pGI->CreateCommandBuffer(create_info);
	}

	void CmdSetViewport(CommandBuffer& commandBuffer) {
		pGI->CmdSetViewport(commandBuffer);
	}

	void CmdSetScissor(CommandBuffer& commandBuffer) {
		return;
	}

	void CmdSetLineWidth(CommandBuffer& commandBuffer) {
		return;
	}

	void CmdBindGraphicsPipeline(CommandBuffer& commandBuffer) {

	}

	void CmdBindIndexBuffer(CommandBuffer& commandBuffer) {

	}

	void CmdBindVertexBuffer(CommandBuffer& commandBuffer) {

	}

	void CmdDraw(CommandBuffer& commandBuffer) {

	}

	void CmdDrawIndexed(CommandBuffer& commandBuffer) {

	}
}
