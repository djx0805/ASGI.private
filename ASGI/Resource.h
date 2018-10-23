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
		virtual Swapchain* GetSwapchain() = 0;
		virtual GIType GetGIType() = 0;
		virtual const char* GetDeviceName() = 0;
	protected:
		virtual ~GraphicsContext();
	};
	typedef ref_ptr<GraphicsContext> graphics_context_ptr;


	class GraphicsResource : public Resource {
	public:
		virtual GraphicsContext* GetContext() { return mContext; }
	protected:
		GraphicsResource(GraphicsContext* pcontext) {
			mContext = pcontext;
		}
		//
		virtual ~GraphicsResource() {}
	protected:
		GraphicsContext* mContext;
	};

	class Buffer : public GraphicsResource {
	public:
		inline BufferUsageFlags GetUsageFlags() { return mUsageFlags; }
		inline uint64_t GetSize() { return mSize; }
	protected:
		Buffer(GraphicsContext* pcontext, BufferUsageFlags usageFlags, uint64_t size) : GraphicsResource(pcontext) {
			mUsageFlags = usageFlags;
			mSize = size;
		}
		//
		virtual ~Buffer() {};
	protected:
		BufferUsageFlags mUsageFlags;
		uint64_t mSize;
	};
	typedef ref_ptr<Buffer> buffer_ptr;

	class BufferUpdateContext : public GraphicsResource {
	protected:
		BufferUpdateContext(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~BufferUpdateContext() {}
	};
	typedef ref_ptr<BufferUpdateContext> buffer_update_contex_ptr;


	class ImageView;
	class Image2D;
	class Image : public GraphicsResource {
	public:
		virtual Image2D* asImage2D() { return nullptr; }
		virtual ImageView* GetOrigView() = 0;
		//
	public:
		inline Format GetFormat() {
			return mFormat;
		}
	protected:
		Image(GraphicsContext* pcontext, Format format) : GraphicsResource(pcontext) {
			mFormat = format;
		}
		//
		virtual ~Image() {}
	protected:
		Format mFormat;
	};
	typedef ref_ptr<Image> image_ptr;

	class Image2D : public Image {
	public:
		virtual Image2D* asImage2D() { return this; }
		//
	public:
		inline Extent2D GetSize() { return mExtent; }
		inline uint32_t GetNumMip() { return mNumMip; }
	protected:
		Image2D(GraphicsContext* pcontext, Format format, uint32_t sizeX, uint32_t sizeY, uint32_t numMip) : Image(pcontext, format) {
			mExtent.width = sizeX;
			mExtent.height = sizeY;
			mNumMip = numMip;
		}
	protected:
		Extent2D mExtent;
		uint32_t mNumMip;
	};
	typedef ref_ptr<Image2D> image_2d_ptr;

	class Sampler : public GraphicsResource {
	protected:
		Sampler(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~Sampler() {}
	};
	typedef ref_ptr<Sampler> sampler_ptr;

	class ImageUpdateContext : public GraphicsResource {
	protected:
		ImageUpdateContext(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~ImageUpdateContext() {}
	};
	typedef ref_ptr<ImageUpdateContext> image_update_context_ptr;

	class ImageView : public GraphicsResource {
	public:
		virtual Image* GetSrcImage() = 0;
	protected:
		ImageView(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~ImageView() {}
	};
	typedef ref_ptr<ImageView> image_view_ptr;

	class ExcuteQueue : public GraphicsResource {
	public:
		virtual QueueType GetType() = 0;
	protected:
		ExcuteQueue(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~ExcuteQueue() {}
	};

	class CommandBuffer : public GraphicsResource {
	protected:
		CommandBuffer(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~CommandBuffer() {}
	};
	typedef ref_ptr<CommandBuffer> command_buffer_ptr;

	class ShaderModule : public GraphicsResource {
	protected:
		ShaderModule(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~ShaderModule() {}
	};
	typedef ref_ptr<ShaderModule> shader_module_ptr;

	class ShaderProgram : public GraphicsResource {
	public:
		virtual ShaderModule* GetVertexShader() = 0;
		virtual ShaderModule* GetGeomteryShader() = 0;
		virtual ShaderModule* GetTessControlShader() = 0;
		virtual ShaderModule* GetTessEvaluationShader() = 0;
		virtual ShaderModule* GetFragmentShader() = 0;
	protected:
		ShaderProgram(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~ShaderProgram() {}
	};
	typedef ref_ptr<ShaderProgram> shader_program_ptr;

	class RenderPass : public GraphicsResource {
	protected:
		RenderPass(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~RenderPass() {}
	};
	typedef ref_ptr<RenderPass> render_pass_ptr;

	class GraphicsPipeline : public GraphicsResource {
	public:
		virtual ShaderProgram* GetGPUProgram() = 0;
	protected:
		GraphicsPipeline(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~GraphicsPipeline() {}
	};
	typedef ref_ptr<GraphicsPipeline> graphics_pipeline_ptr;

	class Swapchain : public GraphicsResource {
	public:
		virtual Format GetColorFormat() = 0;
		virtual Format GetDepthStencilFormat() = 0;
		virtual uint32_t GetNumAttachment() = 0;
		virtual uint32_t AcquireNextAttachment() = 0;
		virtual Image2D* GetColorAttachment(uint32_t index) = 0;
		virtual Image2D* GetDepthStencilAttachment(uint32_t index) = 0;
		virtual Extent2D GetExtent() = 0;
	protected:
		Swapchain(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~Swapchain() {}
	};
	typedef ref_ptr<Swapchain> swapchain_ptr;

	class ComputePass : public GraphicsResource {
	protected:
		ComputePass(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~ComputePass() {}
	};

	class ComputePipeline : public GraphicsResource {
	public:
		virtual ShaderProgram* GetGPUProgram() = 0;
	protected:
		ComputePipeline(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~ComputePipeline() {}
	};

	class FrameBuffer : public GraphicsResource {
	public:
		virtual Extent2D GetExtent() = 0;
		virtual uint32_t GetNumAttachment() = 0;
		virtual ImageView* GetAttachment(uint32_t index) = 0;
		virtual ClearValue GetClearValue(uint32_t index) = 0;
		virtual void SetClearValue(uint32_t index, ClearValue clearValue) = 0;
	protected:
		FrameBuffer(GraphicsContext* pcontext) : GraphicsResource(pcontext) {}
		virtual ~FrameBuffer() {}
	};
	typedef ref_ptr<FrameBuffer> frame_buffer_ptr;
}
