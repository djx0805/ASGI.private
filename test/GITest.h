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

		return true;
	}

	void Render() override;
};