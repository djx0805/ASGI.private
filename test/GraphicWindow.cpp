#include "stdafx.h"
#include "GraphicWindow.h"
#include <process.h>

#include <iostream>
#include <ppl.h>
#include <concurrent_unordered_map.h>

#include <string>
#include <vector>
#include <array>


concurrency::concurrent_unordered_map<HWND, GraphicWindow*> graphic_windows;
std::wstring GraphicWindow::name = L"VulkanLearn";

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto itr = graphic_windows.find(hWnd);
	if(itr == graphic_windows.end())
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	//
	auto pcurrent_window = itr->second;
	if (pcurrent_window != nullptr)
	{
		pcurrent_window->handleMessages(hWnd, uMsg, wParam, lParam);
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

GraphicWindow::GraphicWindow()
{
}


GraphicWindow::~GraphicWindow()
{

}


unsigned __stdcall window_thread(void* pdata) {
	GraphicWindow* pwindow = (GraphicWindow*)pdata;
	//
	

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (pwindow->fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = screenWidth;
		dmScreenSettings.dmPelsHeight = screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if ((pwindow->width != screenWidth) && (pwindow->height != screenHeight))
		{
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, L"Fullscreen Mode not supported!\n Switch to window mode?", L"Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					pwindow->fullscreen = FALSE;
				}
				else
				{
					return FALSE;
				}
			}
		}

	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (pwindow->fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = pwindow->fullscreen ? (long)screenWidth : (long)pwindow->width;
	windowRect.bottom = pwindow->fullscreen ? (long)screenHeight : (long)pwindow->height;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	pwindow->win_handle = CreateWindowEx(
		dwExStyle,
		pwindow->name.c_str(),
		pwindow->title.c_str(),
		dwStyle,
		0, 0,
		pwindow->width,
		pwindow->height,
		NULL, NULL, GetModuleHandle(NULL), NULL);
	if (pwindow->win_handle == nullptr) {
		std::cout << "create window failed" << std::endl;
		exit(1);
	}
	//
	graphic_windows[pwindow->win_handle] = pwindow;
	//
	if (!pwindow->fullscreen)
	{
		// Center on screen
		uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
		SetWindowPos(pwindow->win_handle, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	ShowWindow(pwindow->win_handle, SW_SHOW);
	SetForegroundWindow(pwindow->win_handle);
	SetFocus(pwindow->win_handle);
	//
	std::cout << "create window successed" << std::endl;
	//
	pwindow->is_running = true;
	//
	UpdateWindow(pwindow->win_handle);
	//
	if (!pwindow->PrepareRender()) {
		std::cout << "failed to prepare render contex" << std::endl;
		return 0;
	}
	//
	MSG msg;
	while (true)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//
		if (msg.message == WM_QUIT)
		{
			break;
		}
		//
		pwindow->Render();
	}
	//
	pwindow->is_running = false;
	//
	return msg.wParam;
}
bool GraphicWindow::Init(const std::wstring& title, bool full_screen, int width, int height)
{
	static bool win_class_registered = false;
	if (!win_class_registered)
	{
		WNDCLASSEX wndClass;
		ZeroMemory(&wndClass, sizeof(wndClass));
		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wndClass.lpfnWndProc = WndProc;
		wndClass.hInstance = GetModuleHandle(nullptr);
		wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.lpszClassName = name.c_str();

		if (!RegisterClassEx(&wndClass))
		{
			std::cout << "Could not register window class!\n";
			fflush(stdout);
			exit(1);
		}
		//
		win_class_registered = true;
	}
	//
	this->name = name;
	this->title = title;
	this->width = width;
	this->height = height;
	this->fullscreen = full_screen;
	//
	return true;
}

void GraphicWindow::Run()
{
	//_beginthreadex(NULL, 0, window_thread, this, 0, NULL);
	window_thread(this);
}

void GraphicWindow::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	current_wparam = wParam;
	current_lparam = lParam;
	//
	switch (uMsg)
	{
	case WM_ERASEBKGND:
		break;
	case WM_CLOSE:
		OnClose();
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
	case WM_ACTIVATE:
		break;
	case WM_PAINT:
		PAINTSTRUCT paint;
		::BeginPaint(win_handle, &paint);
		::EndPaint(win_handle, &paint);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			OnClose();
			PostQuitMessage(0);
			break;
		default:
			OnKeyDown(wParam, current_mouse_position);
		}
		break;
	case WM_KEYUP:
		break;
	case WM_RBUTTONDOWN:
		OnRBTNDown(current_mouse_position);
		break;
	case WM_LBUTTONDOWN:
		OnLBTNDown(current_mouse_position);
		break;
	case WM_MBUTTONDOWN:
		OnMBTNDown(current_mouse_position);
		break;
	case WM_LBUTTONUP:
		OnLBTNUp(current_mouse_position);
		break;
	case WM_RBUTTONUP:
		OnRBTNUp(current_mouse_position);
		break;
	case WM_MBUTTONUP:
		break;
	case WM_LBUTTONDBLCLK:
		OnLBTNDBClick(current_mouse_position);
		break;
	case WM_RBUTTONDBLCLK:
		OnRBTNDBClick(current_mouse_position);
		break;
	case WM_MOUSEWHEEL:
	{
		short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		OnMouseWheel(wheelDelta, current_mouse_position);
		break;
	}
	case WM_MOUSEMOVE:
		current_mouse_position.x = LOWORD(lParam);
		current_mouse_position.y = HIWORD(lParam);
		//
		OnMouseMove(current_mouse_position);
		break;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			if ((resizing) || ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
			{
				width = LOWORD(lParam);
				height = HIWORD(lParam);
				OnSize(width, height);
			}
		}
		break;
	case WM_ENTERSIZEMOVE:
		resizing = true;
		break;
	case WM_EXITSIZEMOVE:
		resizing = false;
		break;
	}
}



void GraphicWindow::Render()
{

}
