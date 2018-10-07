#pragma once
#include<functional>

namespace ASGI {
	class CommandBuffer;
	struct CmdBufferTask {
		CommandBuffer* mCmdBuffer;
		CommandBuffer* mSecondCmdBuffer;
		std::function<void(CommandBuffer*, CommandBuffer*)> excute;
		//
		CmdBufferTask() {}
		CmdBufferTask(CommandBuffer* cmdBuffer, CommandBuffer* secondCmdBuffer, std::function<void(CommandBuffer*, CommandBuffer*)> excute_) {
			mCmdBuffer = cmdBuffer;
			mSecondCmdBuffer = secondCmdBuffer;
			excute = excute_;
		}
	};
	class ICmdBufferTaskQueue {
	public:
		virtual ~ICmdBufferTaskQueue() {}
		//
		virtual void PushTask(CmdBufferTask) = 0;
	};
}
