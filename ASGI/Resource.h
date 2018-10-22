#pragma once
#include <atomic>
#include "Definitions.hpp"
#include "ref_ptr.h"

namespace ASGI {
	class Resource {
		template<class T> friend class ref_ptr;
	public:
		inline int ref() const
		{
			mRefCount++;
			return mRefCount.load();
		}

		inline int unref() const
		{
			if (mRefCount.load() == 0)
				return 0;
			//
			int newRef = mRefCount--;
			if (newRef == 0)
				delete this;
			//
			return newRef;
		}

		inline int unref_nodelete() const
		{
			if (mRefCount.load() > 0)
				mRefCount--;
			return mRefCount;
		}

		inline int referenceCount() const { return mRefCount.load(); }
	protected:
		Resource(){}
		virtual ~Resource() {}
	private:
		mutable std::atomic<int>   mRefCount = 0;
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

	struct Viewport {
		float    x;
		float    y;
		float    width;
		float    height;
		float    minDepth;
		float    maxDepth;
	};

	union ClearColorValue {
		float       float32[4];
		int32_t     int32[4];
		uint32_t    uint32[4];
	};

	struct ClearDepthStencilValue {
		float       depth;
		uint32_t    stencil;
	};

	union ClearValue {
		ClearColorValue           color;
		ClearDepthStencilValue    depthStencil;
	};

	class Swapchain;
	class GraphicsContext : public Resource {
	public:
		virtual ~GraphicsContext() {}
		virtual Swapchain* GetSwapchain() = 0;
		virtual GIType GetGIType() = 0;
		virtual const char* GetDeviceName() = 0;
	};
	typedef ref_ptr<GraphicsContext> graphics_context_ptr;

	class Buffer : public Resource {
	public:
		Buffer(BufferUsageFlags usageFlags, uint64_t size) {
			mUsageFlags = usageFlags;
			mSize = size;
		}
		virtual ~Buffer() {};
		//
		inline BufferUsageFlags GetUsageFlags() { return mUsageFlags; }
		inline uint64_t GetSize() { return mSize; }
	protected:
		BufferUsageFlags mUsageFlags;
		uint64_t mSize;
	};
	typedef ref_ptr<Buffer> buffer_ptr;

	class BufferUpdateContext : public Resource {
	public:
		virtual ~BufferUpdateContext() {}
	};
	typedef ref_ptr<BufferUpdateContext> buffer_update_contex_ptr;


	class ImageView;
	class Image2D;
	class Image : public Resource {
	public:
		Image(Format format) {
			mFormat = format;
		}

		virtual ~Image() {}
		//
		virtual Image2D* asImage2D() { return nullptr; }
		virtual ImageView* GetOrigView() = 0;
		//
	public:
		inline Format GetFormat() {
			return mFormat;
		}
	protected:
		Format mFormat;
	};
	typedef ref_ptr<Image> image_ptr;

	class Image2D : public Image {
	public:
		Image2D(Format format, uint32_t sizeX, uint32_t sizeY, uint32_t numMip) : Image(format) {
			mExtent.width = sizeX;
			mExtent.height = sizeY;
			mNumMip = numMip;
		}
		virtual Image2D* asImage2D() { return this; }
		//
	public:
		inline Extent2D GetSize() { return mExtent; }
		inline uint32_t GetNumMip() { return mNumMip; }
	protected:
		Extent2D mExtent;
		uint32_t mNumMip;
	};
	typedef ref_ptr<Image2D> image_2d_ptr;

	class Sampler : public Resource {
	public:
		virtual ~Sampler() {}
	};
	typedef ref_ptr<Sampler> sampler_ptr;

	class ImageUpdateContext : public Resource {
	public:
		virtual ~ImageUpdateContext() {}
	};
	typedef ref_ptr<ImageUpdateContext> image_update_context_ptr;

	class ImageView : public Resource {
	public:
		virtual ~ImageView() {}
		//
		virtual Image* GetSrcImage() = 0;
	};
	typedef ref_ptr<ImageView> image_view_ptr;

	class ExcuteQueue : public Resource {
	public:
		virtual ~ExcuteQueue() {}
		virtual QueueType GetType() = 0;
	};

	class CommandBuffer : public Resource {
	public:
		virtual ~CommandBuffer() {}
	};
	typedef ref_ptr<CommandBuffer> command_buffer_ptr;

	class Semaphore : public Resource {
	public:
		virtual ~Semaphore() {}
	};

	class Fence : public Resource {
	public:
		virtual ~Fence() {}
	};

	class ShaderModule : public Resource {
	public:
		virtual ~ShaderModule() {}
	};
	typedef ref_ptr<ShaderModule> shader_module_ptr;

	class ShaderProgram : public Resource {
	public:
		virtual ~ShaderProgram() {}
		virtual ShaderModule* GetVertexShader() = 0;
		virtual ShaderModule* GetGeomteryShader() = 0;
		virtual ShaderModule* GetTessControlShader() = 0;
		virtual ShaderModule* GetTessEvaluationShader() = 0;
		virtual ShaderModule* GetFragmentShader() = 0;
	};
	typedef ref_ptr<ShaderProgram> shader_program_ptr;

	class RenderPass : public Resource {
	public:

	};
	typedef ref_ptr<RenderPass> render_pass_ptr;

	class GraphicsPipeline : public Resource {
	public:
		virtual ~GraphicsPipeline() {}
		virtual ShaderProgram* GetGPUProgram() = 0;
	};
	typedef ref_ptr<GraphicsPipeline> graphics_pipeline_ptr;

	class Swapchain : public Resource {
	public:
		virtual Format GetColorFormat() = 0;
		virtual Format GetDepthStencilFormat() = 0;
		virtual uint32_t GetNumAttachment() = 0;
		virtual uint32_t AcquireNextAttachment() = 0;
		virtual Image2D* GetColorAttachment(uint32_t index) = 0;
		virtual Image2D* GetDepthStencilAttachment(uint32_t index) = 0;
		virtual Extent2D GetExtent() = 0;
	};
	typedef ref_ptr<Swapchain> swapchain_ptr;

	class ComputePass : public Resource {
	public:
		virtual CommandBuffer* GetCmdBuffer() = 0;
	};

	class ComputePipeline : public Resource {
	public:
		virtual ~ComputePipeline() {}
		//
		virtual ShaderProgram* GetGPUProgram() = 0;
	};

	class FrameBuffer : public Resource {
	public:
		virtual ~FrameBuffer() {}
		//
		virtual Extent2D GetExtent() = 0;
		virtual uint32_t GetNumAttachment() = 0;
		virtual ImageView* GetAttachment(uint32_t index) = 0;
		virtual ClearValue GetClearValue(uint32_t index) = 0;
		virtual void SetClearValue(uint32_t index, ClearValue clearValue) = 0;
	};
	typedef ref_ptr<FrameBuffer> frame_buffer_ptr;
}
