#pragma once
#include <thread>
#include <sstream>
#include <mutex>
#include <unordered_map>

namespace ASGI{
	class GraphicsContext;
	class DynamicGI;
	class GraphicsContextManager {
	public:
		static GraphicsContextManager* Instance() {
			static auto graphicsContextManager = std::unique_ptr<GraphicsContextManager>(new GraphicsContextManager());
			return graphicsContextManager.get();
		}
	public:
		inline GraphicsContext* GetCurrentContext() {
			std::lock_guard<std::mutex> lock(mMutexContext);
			//
			return mThreadCurrentContext[std::this_thread::get_id()];
		}
		//
		inline void AddContext(GraphicsContext* pcontext) {
			std::lock_guard<std::mutex> lock(mMutexContext);
			//
			mThreadContexts[std::this_thread::get_id()].push_back(pcontext);
			auto itrCurrent = mThreadCurrentContext.find(std::this_thread::get_id());
			if (itrCurrent == mThreadCurrentContext.end()) {
				mThreadCurrentContext[std::this_thread::get_id()] = pcontext;
			}
		}
		//
		inline void SetCurrentContext(GraphicsContext* pcontext) {
			std::lock_guard<std::mutex> lock(mMutexContext);
			//
			auto &contexts = mThreadContexts[std::this_thread::get_id()];
			if (std::find(contexts.begin(), contexts.end(), pcontext) == contexts.end()) {
				return;
			}
			mThreadCurrentContext[std::this_thread::get_id()] = pcontext;
		}
		//
		DynamicGI* GetDynamicGI();
	private:
		std::unordered_map<std::thread::id, std::vector<GraphicsContext*> > mThreadContexts;
		std::unordered_map<std::thread::id, GraphicsContext*> mThreadCurrentContext;
		//
		std::mutex mMutexContext;
	};
}