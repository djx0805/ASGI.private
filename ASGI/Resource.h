#pragma once
#include <atomic>
#include "Definitions.hpp"
#include "ref_ptr.h"

namespace ASGI {
	class Resource {
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

	class VertexBuffer;
	class IndexBuffer;
	class UniformBuffer;
	class Buffer : public Resource {
	public:
			virtual ~Buffer() {};
			//
			virtual VertexBuffer* asVertexBuffer() { return nullptr; }
			virtual IndexBuffer* asIndexBuffer() { return nullptr; }
			virtual UniformBuffer* asUniformBuffer() { return nullptr; }
	};
	typedef ref_ptr<Buffer> buffer_ptr;

	class VertexBuffer : public Buffer {
	public:
		virtual VertexBuffer* asVertexBuffer() { return this; }
	};
	typedef ref_ptr<VertexBuffer> vertex_buffer_ptr;

	class IndexBuffer : public Buffer {
	public:
		virtual IndexBuffer* asIndexBuffer() { return this; }
	};
	typedef ref_ptr<IndexBuffer> index_buffer_ptr;

	class UniformBuffer : public Buffer {
	public:
		virtual UniformBuffer* asUniformBuffer() { return this; }
	};
	typedef ref_ptr<UniformBuffer> uniform_buffer_ptr;

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

	class CommandBuffer : public Resource {

	};
	typedef ref_ptr<CommandBuffer> command_buffer_ptr;

	class ShaderModule : public Resource {
	public:
		virtual ~ShaderModule() {}
	};
	typedef ref_ptr<ShaderModule> shader_module_ptr;

	class RenderPass : public Resource {

	};
	typedef ref_ptr<RenderPass> render_pass_ptr;

	class GraphicsPipeline : public Resource {

	};
	typedef ref_ptr<GraphicsPipeline> graphics_pipeline_ptr;

	class Swapchain : public Resource {
	public:
		virtual Format GetSurfaceFormat() = 0;
	};
	typedef ref_ptr<Swapchain> swapchain_ptr;
}
