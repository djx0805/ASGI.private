#pragma once

#include <unordered_map>
#include <queue>
#include <mutex>
#include "VulkanSDK\1.1.77.0\Include\vulkan\vulkan.h"
#include "ASGI.hpp"

namespace ASGI {
	class VKCmdBufferManager {
		struct CmdBufferItm {
			VkCommandPool cmdPool;
			VkCommandBuffer cmdBuffer;
			VkPipelineBindPoint bindPoint;
			VkCommandBufferLevel level;
		};
	public:
		VKCmdBufferManager(VkDevice logicDevice, uint32_t graphicsQueueFamilyIndex, uint32_t computeQueueFamilyIndex) {
			mLogicDevice = logicDevice;
			mGraphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
			mComputeQueueFamilyIndex = computeQueueFamilyIndex;
			//
			mUploadCmdBuffer = AcquirePrimaryCmdBuffer(VK_PIPELINE_BIND_POINT_GRAPHICS);
		}

		inline VkCommandBuffer GetUpLoadCmdBuffer() {
			return mUploadCmdBuffer;
		}

		VkCommandBuffer AcquirePrimaryCmdBuffer(VkPipelineBindPoint bindPoint) {
			std::lock_guard<std::mutex> lock(mMutex);
			//
			if (bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS && !mFreeGPrimaryCmdBuffers.empty()) {
				auto pbuffer =  mFreeGPrimaryCmdBuffers.front().cmdBuffer;
				mUsedPrimaryCmdBuffers[(long long)pbuffer] = mFreeGPrimaryCmdBuffers.front();
				mFreeGPrimaryCmdBuffers.pop();
				return pbuffer;
			}
			else if (bindPoint == VK_PIPELINE_BIND_POINT_COMPUTE && !mFreeCPrimaryCmdBuffers.empty()) {
				auto pbuffer = mFreeCPrimaryCmdBuffers.front().cmdBuffer;
				mUsedPrimaryCmdBuffers[(long long)pbuffer] = mFreeCPrimaryCmdBuffers.front();
				mFreeCPrimaryCmdBuffers.pop();
				return pbuffer;
			}
			//
			CmdBufferItm tmp;
			tmp.bindPoint = bindPoint;
			tmp.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			//
			VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
			cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolCreateInfo.pNext = nullptr;
			cmdPoolCreateInfo.queueFamilyIndex = bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS ? mGraphicsQueueFamilyIndex : mComputeQueueFamilyIndex;
			cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			if (vkCreateCommandPool(mLogicDevice, &cmdPoolCreateInfo, nullptr, &tmp.cmdPool) != VK_SUCCESS) {
				return nullptr;
			}
			//
			VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.commandPool = tmp.cmdPool;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAllocInfo.pNext = nullptr;
			cmdBufferAllocInfo.commandBufferCount = 1;
			if (vkAllocateCommandBuffers(mLogicDevice, &cmdBufferAllocInfo, &tmp.cmdBuffer) != VK_SUCCESS) {
				vkDestroyCommandPool(mLogicDevice, tmp.cmdPool, nullptr);
				return nullptr;
			}
			//
			mUsedPrimaryCmdBuffers[(long long)tmp.cmdBuffer] = tmp;
			//
			return tmp.cmdBuffer;
		}

		VkCommandBuffer AcquireSecondCmdBuffer(VkPipelineBindPoint bindPoint) {
			std::lock_guard<std::mutex> lock(mMutex);
			//
			if (bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS && !mFreeGSecondCmdBuffers.empty()) {
				auto pbuffer = mFreeGSecondCmdBuffers.front().cmdBuffer;
				mUsedSecondCmdBuffers[(long long)pbuffer] = mFreeGSecondCmdBuffers.front();
				mFreeGSecondCmdBuffers.pop();
				return pbuffer;
			}
			else if (bindPoint == VK_PIPELINE_BIND_POINT_COMPUTE && !mFreeCSecondCmdBuffers.empty()) {
				auto pbuffer = mFreeCSecondCmdBuffers.front().cmdBuffer;
				mUsedSecondCmdBuffers[(long long)pbuffer] = mFreeCSecondCmdBuffers.front();
				mFreeCSecondCmdBuffers.pop();
				return pbuffer;
			}
			//
			CmdBufferItm tmp;
			tmp.bindPoint = bindPoint;
			tmp.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			//
			VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
			cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolCreateInfo.pNext = nullptr;
			cmdPoolCreateInfo.queueFamilyIndex = bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS ? mGraphicsQueueFamilyIndex : mComputeQueueFamilyIndex;
			cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			if (vkCreateCommandPool(mLogicDevice, &cmdPoolCreateInfo, nullptr, &tmp.cmdPool) != VK_SUCCESS) {
				return nullptr;
			}
			//
			VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.commandPool = tmp.cmdPool;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			cmdBufferAllocInfo.pNext = nullptr;
			cmdBufferAllocInfo.commandBufferCount = 1;
			if (vkAllocateCommandBuffers(mLogicDevice, &cmdBufferAllocInfo, &tmp.cmdBuffer) != VK_SUCCESS) {
				vkDestroyCommandPool(mLogicDevice, tmp.cmdPool, nullptr);
				return nullptr;
			}
			//
			mUsedPrimaryCmdBuffers[(long long)tmp.cmdBuffer] = tmp;
			//
			return tmp.cmdBuffer;
		}

		void FreeCmdBuffer(VkCommandBuffer cmdBuffer) {
			{
				auto itr = mUsedPrimaryCmdBuffers.find((long long)cmdBuffer);
				if (itr != mUsedPrimaryCmdBuffers.end()) {
					if (itr->second.bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
							mFreeGPrimaryCmdBuffers.push(itr->second);
							mUsedPrimaryCmdBuffers.erase(itr);
							return;
					}
					else if (itr->second.bindPoint == VK_PIPELINE_BIND_POINT_COMPUTE) {
						mFreeCPrimaryCmdBuffers.push(itr->second);
						mUsedPrimaryCmdBuffers.erase(itr);
						return;
					}
				}
			}
			//
			{
				auto itr = mUsedSecondCmdBuffers.find((long long)cmdBuffer);
				if (itr != mUsedSecondCmdBuffers.end()) {
					if (itr->second.bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
						mFreeGSecondCmdBuffers.push(itr->second);
						mUsedPrimaryCmdBuffers.erase(itr);
						return;
					}
					else if (itr->second.bindPoint == VK_PIPELINE_BIND_POINT_COMPUTE) {
						mFreeCSecondCmdBuffers.push(itr->second);
						mUsedPrimaryCmdBuffers.erase(itr);
						return;
					}
				}
			}
		}
	private:
		VkDevice mLogicDevice;
		uint32_t mGraphicsQueueFamilyIndex;
		uint32_t mComputeQueueFamilyIndex;
		//
		VkCommandBuffer mUploadCmdBuffer;
		std::queue<CmdBufferItm> mFreeGPrimaryCmdBuffers;
		std::queue<CmdBufferItm> mFreeGSecondCmdBuffers;
		std::queue<CmdBufferItm> mFreeCPrimaryCmdBuffers;
		std::queue<CmdBufferItm> mFreeCSecondCmdBuffers;
		std::unordered_map<long long, CmdBufferItm> mUsedPrimaryCmdBuffers;
		std::unordered_map<long long, CmdBufferItm> mUsedSecondCmdBuffers;
		std::mutex mMutex;
		//std::unordered_map<long long, 
	};

	class VKCommandBuffer;
	class VKCommand {
	public:
		virtual ~VKCommand() {}
		VKCommand() {
			pnext = nullptr;
		}
		//
		virtual void excute(CommandBuffer* cmdBuffer) {}
		//
		VKCommand* pnext = nullptr;
	};


	//
	class VKCommandBuffer : public CommandBuffer {
		friend class VulkanGI;
	public:
		static void ExcuteParallel(CommandBuffer* pCmdBuffer, CommandBuffer* pSecondCmdBuffer);
	public:
		inline static VKCommandBuffer* Cast(CommandBuffer* pcmd) {
			return (VKCommandBuffer*)pcmd;
		}
	public:
		int mUnExcuteSecondCmdBufferCount = 0;
		//
		VKCommandBuffer() {
			mBindingCmdBuffer = nullptr;
			mUnExcuteSecondCmdBufferCount = 0;
			mCmdBufferLevel = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			//
			mHead = new VKCommand();
			mLastCmd = mHead;
		}

		inline void PushCommand(VKCommand* pcmd) {
			mLastCmd->pnext = pcmd;
			mLastCmd = mLastCmd->pnext;
		}

		inline VkCommandBuffer GetBindingCmdBuffer() {
			return mBindingCmdBuffer;
		}

		inline uint32_t GetNumSecondCmdBuffer() {
			return mSecondCmdBuffers.size();
		}

		inline void MergeTo(VKCommandBuffer* targetCmdBuffer) {
			targetCmdBuffer->mLastCmd->pnext = mHead->pnext;
			targetCmdBuffer->mLastCmd = mLastCmd;
			//
			mHead->pnext = nullptr;
			mLastCmd = mHead;
		}

		VkResult Excute();
	private:
		VKCommand* mHead;
		VKCommand* mLastCmd;
		std::mutex mMtxSecondCmdBuffer;
		std::condition_variable mCond;
		VkCommandBuffer mBindingCmdBuffer;
		VkCommandBufferLevel mCmdBufferLevel;
		std::vector<CommandBuffer*> mSecondCmdBuffers;
	};

	//
	class VKCmdBeginRenderPass : public VKCommand {
	public:
		VKCmdBeginRenderPass(RenderPass* renderPass, FrameBuffer* frameBuffer) {
			mRenderPass = renderPass;
			mFrameBuffer = frameBuffer;
		}

		void excute(CommandBuffer* cmdBuffer) override;
	private:
		RenderPass* mRenderPass;
		FrameBuffer* mFrameBuffer;
	};

	class VKCmdEndSubRenderPass : public VKCommand {
	public:

	};

	class VKCmdEndRenderPass : public VKCommand {
	public:
		void excute(CommandBuffer* cmdBuffer) override {
			auto tmp = VKCommandBuffer::Cast(cmdBuffer);
			//
			vkCmdEndRenderPass(tmp->GetBindingCmdBuffer());
			vkEndCommandBuffer(tmp->GetBindingCmdBuffer());
		}
	};

	class VKCmdBeginComputePass : public VKCommand {
	public:

	};

	class VKCmdEndComputePass : public VKCommand {
	public:

	};

	class VKCmdBindPipeline : public VKCommand {
	public:
		VKCmdBindPipeline(GraphicsPipeline* pipeline) {
			mGraphicsPipeline = pipeline;
			mComputePipeline = nullptr;
		}

		void excute(CommandBuffer* cmdBuffer) override;
	private:
		GraphicsPipeline* mGraphicsPipeline;
		ComputePipeline* mComputePipeline;
	};

	class VKCmdSetViewport : public VKCommand {
	public:
		VKCmdSetViewport(uint32_t   firstViewport, uint32_t  viewportCount, Viewport*  viewports) {
			mFirstViewport = firstViewport;
			mViewportCount = viewportCount;
			mViewports = viewports;
		}

		void excute(CommandBuffer* cmdBuffer) override;
	private:
		uint32_t mFirstViewport;
		uint32_t mViewportCount;
		Viewport* mViewports;
	};

	class VKCmdSetScissor : public VKCommand {
	public:
		VKCmdSetScissor(uint32_t firstScissor, uint32_t scissorCount, Rect2D* scissors) {
			mFirstScissor = firstScissor;
			mScissorCount = scissorCount;
			mScissors = scissors;
		}
		//
		void excute(CommandBuffer* cmdBuffer) override;
	private:
		uint32_t mFirstScissor;
		uint32_t mScissorCount;
		Rect2D* mScissors;
	};

	class VKCmdSetLineWidth : public VKCommand {
	public:
		VKCmdSetLineWidth(float lineWidth) {
			mLineWidth = lineWidth;
		}
		//
		void excute(CommandBuffer* cmdBuffer) override;
	private:
		float mLineWidth;
	};

	class VKCmdBindIndexBuffer : public VKCommand {
	public:
		VKCmdBindIndexBuffer(Buffer* pbuffer, uint32_t offset, Format indexFormat) {
			mBuffer = pbuffer;
			mOffset = offset;
			mFormat = indexFormat;
		}
		//
		void excute(CommandBuffer* cmdBuffer) override;
	private:
		Buffer* mBuffer;
		uint32_t mOffset;
		Format mFormat;
	};

	class VKCmdBindVertexBuffer : public VKCommand {
	public:
		VKCmdBindVertexBuffer(uint32_t bindingIndex, Buffer* pbuffer, uint32_t offset) {
			mBindingIndex = bindingIndex;
			mBuffer = pbuffer;
			mOffset = offset;
		}
		//
		void excute(CommandBuffer* cmdBuffer) override;
	private:
		uint32_t mBindingIndex;
		Buffer* mBuffer;
		uint32_t mOffset;
	};

	class VKCmdDraw : public VKCommand {
	public:
		VKCmdDraw(uint32_t virtexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
			mVirtexCount = virtexCount;
			mInstanceCount = instanceCount;
			mFirstVertex = firstVertex;
			mFirstInstance = firstInstance;
		}
		//
		void excute(CommandBuffer* cmdBuffer) override;
	private:
		uint32_t mVirtexCount;
		uint32_t mInstanceCount;
		uint32_t mFirstVertex;
		uint32_t mFirstInstance;
	};

	class VKCmdDrawIndexed : public VKCommand {
	public:
		VKCmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) {
			mIndexCount = indexCount;
			mInstanceCount = instanceCount;
			mFirstIndex = firstIndex;
			mVertexOffset = vertexOffset;
			mFirstInstance = firstInstance;
		}
		//
		void excute(CommandBuffer* cmdBuffer) override;
	private:
		uint32_t mIndexCount;
		uint32_t mInstanceCount;
		uint32_t mFirstIndex;
		uint32_t mVertexOffset;
		uint32_t mFirstInstance;
	};
}