#include "VulkanResource.h"

#include "third_lib\SPIRV-Cross\spirv_cross.hpp"

namespace ASGI {
	void VKCommandBuffer::ExcuteParallel(CommandBuffer* pCmdBuffer, CommandBuffer* pSecondCmdBuffer) {
		auto tmp = VKCommandBuffer::Cast(pCmdBuffer);
		//excute
		auto pcmd = tmp->mHead->pnext;
		while (pcmd != nullptr) {
			pcmd->excute(pCmdBuffer);
			pcmd = pcmd->pnext;
		}
		//
		{
			std::lock_guard<std::mutex> lock(tmp->mMtxSecondCmdBuffer);
			--tmp->mUnExcuteSecondCmdBufferCount;
		}
		//
		tmp->mCond.notify_all();
	}

	VkResult VKCommandBuffer::Excute() {
		auto pcmd = mHead->pnext;
		while (pcmd != nullptr) {
			pcmd->excute(this);
			pcmd = pcmd->pnext;
		}
		//
		return VK_SUCCESS;
	}


	bool VKGPUProgram::InitDescriptorSet(VkDevice logicDevice) {
		mLogicDevice = logicDevice;
		//
		std::unordered_map<uint8_t, std::vector<VkDescriptorSetLayoutBinding> > descriptorSets;
		std::unordered_map<VkDescriptorType, std::vector<std::pair<uint8_t, int> > > typedDescriptors;
		//
		auto collectResource = [&](spirv_cross::Compiler* pspirvCompiler, uint32_t stageFlag, VkShaderModule shaderModule)->void {
			auto vs_resource = pspirvCompiler->get_shader_resources();
			for (auto &ubo : vs_resource.uniform_buffers)
			{
				auto setIndex = pspirvCompiler->get_decoration(ubo.id, spv::Decoration::DecorationDescriptorSet);
				descriptorSets[setIndex].push_back({
					pspirvCompiler->get_decoration(ubo.id, spv::Decoration::DecorationBinding),
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					1,
					stageFlag,
					NULL,
				});
				//
				typedDescriptors[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER].push_back(std::pair<uint8_t, int>(setIndex, descriptorSets[setIndex].size() - 1));
			};
			for (auto &simpler : vs_resource.sampled_images) {
				auto setIndex = pspirvCompiler->get_decoration(simpler.id, spv::Decoration::DecorationDescriptorSet);
				descriptorSets[setIndex].push_back({
					pspirvCompiler->get_decoration(simpler.id, spv::Decoration::DecorationBinding),
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					1,
					stageFlag,
					NULL,
				});
				//
				typedDescriptors[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER].push_back(std::pair<uint8_t, int>(setIndex, descriptorSets[setIndex].size() - 1));
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
				mPushConstantRanges.push_back(tmp);
			}
		};
		//
		if (mVertexShader != nullptr) {
			auto pshader = (VKShaderModule*)mVertexShader.get();
			collectResource(pshader->mSpirvCompiler.get(), VK_SHADER_STAGE_VERTEX_BIT, pshader->mShaderModule);
		}
		if (mTessControlShader != nullptr) {
			auto pshader = (VKShaderModule*)mTessControlShader.get();
			collectResource(pshader->mSpirvCompiler.get(), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, pshader->mShaderModule);
		}
		if (mTessEvaluationShader != nullptr) {
			auto pshader = (VKShaderModule*)mTessEvaluationShader.get();
			collectResource(pshader->mSpirvCompiler.get(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, pshader->mShaderModule);
		}
		if (mGeomteryShader != nullptr) {
			auto pshader = (VKShaderModule*)mGeomteryShader.get();
			collectResource(pshader->mSpirvCompiler.get(), VK_SHADER_STAGE_GEOMETRY_BIT, pshader->mShaderModule);
		}
		if (mFragmentShader != nullptr) {
			auto pshader = (VKShaderModule*)mFragmentShader.get();
			collectResource(pshader->mSpirvCompiler.get(), VK_SHADER_STAGE_FRAGMENT_BIT, pshader->mShaderModule);
		}
		//
		mDescriptorSetLayouts.resize(descriptorSets.size());
		int index = 0;
		for (auto &dsb : descriptorSets) {
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
			descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCreateInfo.pBindings = dsb.second.data();
			descriptorSetLayoutCreateInfo.bindingCount = dsb.second.size();

			if (vkCreateDescriptorSetLayout(mLogicDevice, &descriptorSetLayoutCreateInfo, nullptr, &mDescriptorSetLayouts[index]) != VK_SUCCESS) {
				return false;
			}
			//
			mIndexSet[dsb.first] = index++;
		}
		//
		std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
		for (auto &itr : typedDescriptors) {
			VkDescriptorPoolSize descriptorPoolSize = {};
			descriptorPoolSize.type = itr.first;
			descriptorPoolSize.descriptorCount = itr.second.size();
			descriptorPoolSizes.push_back(descriptorPoolSize);
		}
		VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.pNext = NULL;
		descriptorPoolInfo.poolSizeCount = descriptorPoolSizes.size();
		descriptorPoolInfo.pPoolSizes = descriptorPoolSizes.data();
		descriptorPoolInfo.maxSets = descriptorSets.size();
		if (vkCreateDescriptorPool(mLogicDevice, &descriptorPoolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
			return false;
		}
		//
		mDescriptorSets.resize(mDescriptorSetLayouts.size());
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext = NULL;
		descriptorSetAllocateInfo.descriptorPool = mDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = mDescriptorSetLayouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = mDescriptorSetLayouts.size();

		if (vkAllocateDescriptorSets(mLogicDevice, &descriptorSetAllocateInfo, mDescriptorSets.data()) != VK_SUCCESS) {
			return false;
		}
		//
		return true;
	}
}