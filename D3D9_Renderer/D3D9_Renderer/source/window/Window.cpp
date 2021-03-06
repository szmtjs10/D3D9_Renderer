#include <cassert>
#include <stdio.h>

#include "Window.h"

namespace renderer
{
	Window::Window()
		:m_hWindow(nullptr),
		m_windowClass{ 0 }
	{
	}

	Window::~Window()
	{
		::DestroyWindow(m_hWindow);
		::UnregisterClass(m_windowClass.lpszClassName, m_windowClass.hInstance);
	}

	void Window::CreateD3DWindow(LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		m_hWindow = CreateWindowEx(NULL,
			lpClassName,
			lpWindowName,
			dwStyle,
			X,
			Y,
			nWidth,
			nHeight,
			hWndParent,
			hMenu,
			hInstance,
			lpParam);
		assert(m_hWindow);
	}

	void Window::SetUpAndRegesterWindow(HINSTANCE hInstance, LPCWSTR windowClassName)
	{
		ZeroMemory(&m_windowClass, sizeof(WNDCLASSEX));

		m_windowClass.hInstance = hInstance;
		m_windowClass.lpszClassName = windowClassName;
		m_windowClass.cbSize = sizeof(WNDCLASSEX);
		m_windowClass.style = CS_HREDRAW | CS_VREDRAW;
		m_windowClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
		m_windowClass.lpfnWndProc = this->WindowMessageProc;

		RegisterClassEx(&m_windowClass);
	}

	LRESULT Window::WindowMessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
        case WM_CLOSE:
            ::PostQuitMessage(0);
            return 0;
            break;
		default:
			break;
		}

		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}
}