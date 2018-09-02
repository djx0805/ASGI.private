#pragma once
#include<Windows.h>
#include<string>
#include<atomic>

class GraphicWindow
{
	friend LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend unsigned __stdcall window_thread(void* pdata);
public:
	GraphicWindow();
	virtual ~GraphicWindow();
	//
	virtual bool Init(const std::wstring& title, bool full_screen, int width, int height);
	virtual void Run();
	//
	virtual void OnClose() {}
	virtual void OnSize(int cx, int cy) {}
	virtual void OnLBTNDown(POINT position) {}
	virtual void OnLBTNUp(POINT position) {}
	virtual void OnLBTNDBClick(POINT position) {}
	virtual void OnRBTNDown(POINT position) {}
	virtual void OnRBTNUp(POINT position) {}
	virtual void OnRBTNDBClick(POINT position) {}
	virtual void OnMBTNDown(POINT position) {}
	virtual void OnMouseWheel(int delta, POINT position) {}
	virtual void OnMouseMove(POINT position) {}
	virtual void OnKeyDown(char key, POINT position) {}
	//
	virtual bool PrepareRender() { return false; }
	virtual void Render();
	//
	HWND GetHWND() { return win_handle; }
	
	bool IsRunning() { return is_running; }
protected:
	virtual void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//
	bool isLBTNDown() { return current_wparam & MK_LBUTTON; }
	bool isRBTNDown() { return current_wparam & MK_RBUTTON; }
	bool isMBTNDown() { return current_wparam & MK_MBUTTON; }
	bool isCTRLDown() { return current_wparam & MK_CONTROL; }
	bool isSHIFTDown() { return current_wparam & MK_SHIFT; }
protected:
	HWND win_handle = nullptr;
	int width = 800, height = 600;
	std::wstring title = L"";
	static std::wstring name;
	bool resizing = false;
	bool fullscreen = false;
	std::atomic<bool> is_running = false;
private:
	WPARAM current_wparam;
	LPARAM current_lparam;
	POINT current_mouse_position;
};

