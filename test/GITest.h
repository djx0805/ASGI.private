#pragma once
#include "GraphicWindow.h"
#include "..\ASGI\ASGI.h"


class GITest : public GraphicWindow {
public:
	bool PrepareRender() override {
		//
		if (!ASGI::Init(ASGI::GIType::GI_VULKAN, "GeForce GTX 850M")) {
			return false;
		}

		ASGI::SwapchainCreateInfo swapchain_create_info = {
			GetHWND(),
			800,
			600,
			false,
			ASGI::Format::FORMAT_B8G8R8A8_UNORM,
			true
		};
		ASGI::Swapchain* pSwapchain = ASGI::CreateSwapchain(swapchain_create_info);
		if (pSwapchain == nullptr) {
			return false;
		}

		return true;
	}

	void Render() override;
};