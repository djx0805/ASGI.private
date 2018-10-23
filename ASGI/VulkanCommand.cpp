#include "VulkanCommand.h"
#include "VulkanResource.h"

namespace ASGI {
	void VKCmdBeginRenderPass::excute(CommandBuffer* cmdBuffer) {
		auto tmp = VKCommandBuffer::Cast(cmdBuffer);
		//
		auto vkFrameBuffer = VKFrameBuffer::Cast(mFrameBuffer);
		std::vector<VkClearValue> clearValues(vkFrameBuffer->GetNumAttachment());
		for (int i = 0; i < clearValues.size(); ++i) {
			auto &clearValue = vkFrameBuffer->GetClearValue(i);
			memcpy(&clearValues[i], &clearValue, sizeof(clearValue));
		}
		//
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = NULL;
		renderPassBeginInfo.renderPass = VKRenderPass::Cast(mRenderPass)->GetRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = VKFrameBuffer::Cast(mFrameBuffer)->GetWidth();
		renderPassBeginInfo.renderArea.extent.height = VKFrameBuffer::Cast(mFrameBuffer)->GetHeight();
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.framebuffer = VKFrameBuffer::Cast(mFrameBuffer)->GetFrameBuffer();
		//
		vkCmdBeginRenderPass(tmp->GetBindingCmdBuffer(), &renderPassBeginInfo, tmp->GetNumSecondCmdBuffer() > 0 ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : VK_SUBPASS_CONTENTS_INLINE);
	}


	void VKCmdBindPipeline::excute(CommandBuffer* cmdBuffer) {
		if (mGraphicsPipeline == nullptr && mComputePipeline == nullptr) {
			return;
		}
		//
		auto tmp = VKCommandBuffer::Cast(cmdBuffer);
		//
		VkPipelineBindPoint pipelineBindPoint = mGraphicsPipeline != nullptr ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;
		VkPipeline pipeline = mGraphicsPipeline != nullptr ? VKGraphicsPipeline::Cast(mGraphicsPipeline)->GetVKPipleline() : VKComputePipeline::Cast(mComputePipeline)->GetVKPiipeline();
		VkPipelineLayout pipelineLayout = mGraphicsPipeline != nullptr ? VKGraphicsPipeline::Cast(mGraphicsPipeline)->GetPipelineLayout() : VKComputePipeline::Cast(mComputePipeline)->GetPipelineLayout();
		//
		vkCmdBindPipeline(tmp->GetBindingCmdBuffer(), pipelineBindPoint, pipeline);
		//
		auto gpuProgram = mGraphicsPipeline != nullptr ? VKGraphicsPipeline::Cast(mGraphicsPipeline)->GetGPUProgram() : VKComputePipeline::Cast(mComputePipeline)->GetGPUProgram();
		auto &descriptorSets = VKGPUProgram::Cast(gpuProgram)->GetDescriptorSets();
		//
		vkCmdBindDescriptorSets(tmp->GetBindingCmdBuffer(), pipelineBindPoint, pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
	}

	void VKCmdSetViewport::excute(CommandBuffer* cmdBuffer) {
		auto viewportCount = mViewports.size();
		std::vector<VkViewport> viewports(viewportCount);
		for (int i = 0; i < viewportCount; ++i) {
			viewports[i].x = mViewports[i].x;
			viewports[i].y = mViewports[i].y;
			viewports[i].width = mViewports[i].width;
			viewports[i].height = mViewports[i].height;
			viewports[i].minDepth = mViewports[i].minDepth;
			viewports[i].maxDepth = mViewports[i].maxDepth;
		}

		vkCmdSetViewport(VKCommandBuffer::Cast(cmdBuffer)->GetBindingCmdBuffer(), mFirstViewport, viewportCount, viewports.data());
	}

	void VKCmdSetScissor::excute(CommandBuffer* cmdBuffer) {
		auto scissorCount = mScissors.size();
		std::vector<VkRect2D> scissors(scissorCount);
		for (int i = 0; i < scissorCount; ++i) {
			scissors[i].extent.width = mScissors[i].extent.width;
			scissors[i].extent.height = mScissors[i].extent.height;
			scissors[i].offset.x = mScissors[i].offset.x;
			scissors[i].offset.y = mScissors[i].offset.y;
		}
		//
		vkCmdSetScissor(VKCommandBuffer::Cast(cmdBuffer)->GetBindingCmdBuffer(), mFirstScissor, scissorCount, scissors.data());
	}

	void VKCmdSetLineWidth::excute(CommandBuffer* cmdBuffer) {
		vkCmdSetLineWidth(VKCommandBuffer::Cast(cmdBuffer)->GetBindingCmdBuffer(), mLineWidth);
	}

	void VKCmdBindIndexBuffer::excute(CommandBuffer* cmdBuffer) {
		vkCmdBindIndexBuffer(VKCommandBuffer::Cast(cmdBuffer)->GetBindingCmdBuffer(),
			VKBuffer::Cast(mBuffer)->GetVKBuffer(),
			mOffset,
			GetFormatSize(mFormat) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
	}

	void VKCmdBindVertexBuffer::excute(CommandBuffer* cmdBuffer) {
		VkBuffer buffers[1] = { VKBuffer::Cast(mBuffer)->GetVKBuffer() };
		VkDeviceSize offsets[1] = { mOffset };
		//
		vkCmdBindVertexBuffers( VKCommandBuffer::Cast(cmdBuffer)->GetBindingCmdBuffer(),
			mBindingIndex, 1, buffers, offsets );
	}

	void VKCmdDraw::excute(CommandBuffer* cmdBuffer) {
		vkCmdDraw(VKCommandBuffer::Cast(cmdBuffer)->GetBindingCmdBuffer(),
			mVirtexCount, mInstanceCount, mFirstVertex, mFirstInstance);
	}

	void VKCmdDrawIndexed::excute(CommandBuffer* cmdBuffer) {
		vkCmdDrawIndexed(VKCommandBuffer::Cast(cmdBuffer)->GetBindingCmdBuffer(),
			mIndexCount, mInstanceCount, mFirstIndex, mVertexOffset, mFirstInstance);
	}
}