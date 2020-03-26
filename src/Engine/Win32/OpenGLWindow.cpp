// Windows
#include <Windows.h>
#include <windowsx.h>

// STD
#include <chrono>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Win32/Windows.hpp>
#include <Engine/Win32/OpenGLWindow.hpp>
#include <Engine/Clock.hpp>


namespace {
	template<class T>
	T getFunctionPointerGL(const char* name) {
		auto addr = wglGetProcAddress(name);
		ENGINE_ASSERT(addr, "Unable to get WGL function pointer for ", name, " - ", Engine::Win32::getLastErrorMessage());
		return reinterpret_cast<T>(addr);

		// TODO: only needed for gl 1?
		// if (addr) { return addr; }
		//static auto module = [](){
		//	const auto handle = GetModuleHandleW(L"opengl32.dll");
		//	if (!handle) {
		//		abort(); // TODO: handle error. GetLastError
		//	}
		//	return  handle;
		//}();
		//
		//addr = GetProcAddress(module, name);
	}
}

namespace Engine::Win32 {
	OpenGLWindow::OpenGLWindow(const PixelFormat& pixelFormat, const ContextFormat& contextFormat) {
		static const WGLPointers ptrs = OpenGLWindow::init();

		windowHandle = CreateWindowExW(
			0, // TODO:
			className,
			L"My Window Title",
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
			//WS_POPUP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			1900,
			1300,
			0,
			0,
			GetModuleHandleW(nullptr),
			nullptr
		);
		ENGINE_ASSERT(windowHandle, "Unable to create window. - ", getLastErrorMessage());
		SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		deviceContext = GetDC(windowHandle);
		ENGINE_ASSERT(deviceContext, "Unable to get Windows device context - ", getLastErrorMessage());

		// TODO: srgb?
		// TODO: figure out what we actually want
		const int pixelAttributes[] = {
			WGL_DRAW_TO_WINDOW_ARB, true,
			WGL_SUPPORT_OPENGL_ARB, true,
			WGL_DOUBLE_BUFFER_ARB, true,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, pixelFormat.colorBits,
			WGL_ALPHA_BITS_ARB, pixelFormat.alphaBits,
			WGL_DEPTH_BITS_ARB, pixelFormat.depthBits,
			WGL_STENCIL_BITS_ARB, pixelFormat.stencilBits,
			//WGL_SAMPLE_BUFFERS_ARB, true,
			//WGL_SAMPLES_ARB, 4,
			0
		};

		int winPixelFormat;
		UINT numFormats;
	
		// TODO: glfw manually chooses pixel format - wglGetPixelFormatAttribivARB, WGL_NUMBER_PIXEL_FORMATS_ARB
		// TODO: do we want multiple results?
		// TODO: verify chosen pixel format
		ENGINE_ASSERT(ptrs.wglChoosePixelFormatARB(deviceContext, pixelAttributes, nullptr, 1, &winPixelFormat, &numFormats),
			"Unable to find suitable pixel format - ", getLastErrorMessage()
		);

		PIXELFORMATDESCRIPTOR pixelFormatDesc;
		ENGINE_ASSERT(DescribePixelFormat(deviceContext, winPixelFormat, sizeof(pixelFormatDesc), &pixelFormatDesc),
			"Unable to find describe pixel format - ", getLastErrorMessage()
		);

		ENGINE_ASSERT(SetPixelFormat(deviceContext, winPixelFormat, &pixelFormatDesc),
			"Unable to set pixel format - ", getLastErrorMessage()
		);

		// TODO: figure out what we actually want
		const int contextAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, contextFormat.majorVersion,
			WGL_CONTEXT_MINOR_VERSION_ARB, contextFormat.minorVersion,
			WGL_CONTEXT_FLAGS_ARB, contextFormat.debug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};
	
		renderContext = ptrs.wglCreateContextAttribsARB(deviceContext, nullptr, contextAttributes);
		ENGINE_ASSERT(renderContext, "Unable to create WGL render context - ", getLastErrorMessage());
	}

	OpenGLWindow::~OpenGLWindow() {
		ENGINE_ASSERT(wglMakeCurrent(nullptr, nullptr), "Unable to make WGL render context non-current - ", getLastErrorMessage());
		ENGINE_ASSERT(wglDeleteContext(renderContext), "Unable to delete WGL render context - ", getLastErrorMessage());
		ReleaseDC(windowHandle, deviceContext);
		ENGINE_ASSERT(DestroyWindow(windowHandle), "Unable to destroy window for - ", getLastErrorMessage());
	}

	void OpenGLWindow::show() {
		ShowWindow(windowHandle, SW_SHOW);
	}

	void OpenGLWindow::makeContextCurrent() {
		wglMakeCurrent(deviceContext, renderContext);
	}

	void OpenGLWindow::poll() {
		for (MSG msg; PeekMessageW(&msg, windowHandle, 0, 0, PM_REMOVE);) {
			TranslateMessage(&msg); // Needed for text input
			DispatchMessageW(&msg);
		}
	}

	void OpenGLWindow::swapBuffers() {
		SwapBuffers(deviceContext);
	}

	HWND OpenGLWindow::getWin32WindowHandle() const {
		return windowHandle;
	}

	bool OpenGLWindow::shouldClose() const {
		return close;
	}

	glm::ivec2 OpenGLWindow::getFramebufferSize() const {
		RECT rect;
		GetClientRect(windowHandle, &rect);
		return glm::ivec2{rect.right, rect.bottom};
	}

	auto OpenGLWindow::init() -> WGLPointers {
		puts("OpenGLWindow::init");
		const auto hInstance = GetModuleHandleW(nullptr);

		const WNDCLASSEXW windowClass {
			.cbSize = sizeof(windowClass),
			.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
			.lpfnWndProc = &windowProc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = hInstance,
			.hIcon = 0,
			.hCursor = 0,
			.hbrBackground = 0,
			.lpszMenuName = nullptr,
			.lpszClassName = className,
			.hIconSm = 0,
		};
		ENGINE_ASSERT(RegisterClassExW(&windowClass), "Unable to register OpenGLWindow's WNDCLASSEX - ", getLastErrorMessage());

		const auto tempWindow = CreateWindowExW(
			0, // TODO:
			className,
			L"Temp window for gl loading",
			WS_OVERLAPPEDWINDOW, // TODO:
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance,
			nullptr
		);
		ENGINE_ASSERT(tempWindow, "Unable to create temporary window for wgl function loading - ", getLastErrorMessage());

		const PIXELFORMATDESCRIPTOR tempPixelFormatDesc{
			.nSize = sizeof(tempPixelFormatDesc),
			.nVersion = 1,
			.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			.iPixelType = PFD_TYPE_RGBA,
			.cColorBits = 24,
			.cRedBits = 0,
			.cRedShift = 0,
			.cGreenBits = 0,
			.cGreenShift = 0,
			.cBlueBits = 0,
			.cBlueShift = 0,
			.cAlphaBits = 0,
			.cAlphaShift = 0,
			.cAccumBits = 0,
			.cAccumRedBits = 0,
			.cAccumGreenBits = 0,
			.cAccumBlueBits = 0,
			.cAccumAlphaBits = 0,
			.cDepthBits = 24,
			.cStencilBits = 8,
			.cAuxBuffers = 0,
			.iLayerType = 0,
			.bReserved = 0,
			.dwLayerMask = 0,
			.dwVisibleMask = 0,
			.dwDamageMask = 0,
		};

		const auto tempDeviceContext = GetDC(tempWindow);
		ENGINE_ASSERT(tempDeviceContext, "Unable to get Windows device context - ", getLastErrorMessage());

		const auto tempPixelFormat = ChoosePixelFormat(tempDeviceContext, &tempPixelFormatDesc);
		ENGINE_ASSERT(tempPixelFormat, "Unable to get Windows pixel format - ", getLastErrorMessage());

		ENGINE_ASSERT(SetPixelFormat(tempDeviceContext, tempPixelFormat, &tempPixelFormatDesc),
			"Unable to set Windows pixel format - ", getLastErrorMessage()
		);

		const auto tempRenderContext = wglCreateContext(tempDeviceContext);
		ENGINE_ASSERT(tempRenderContext, "Unable to create WGL render context - ", getLastErrorMessage());

		ENGINE_ASSERT(wglMakeCurrent(tempDeviceContext, tempRenderContext),
			"Unable to make WGL render context current - ", getLastErrorMessage()
		);

		WGLPointers pointers = {
			.wglChoosePixelFormatARB = getFunctionPointerGL<PFNWGLCHOOSEPIXELFORMATARBPROC>("wglChoosePixelFormatARB"),
			.wglCreateContextAttribsARB = getFunctionPointerGL<PFNWGLCREATECONTEXTATTRIBSARBPROC>("wglCreateContextAttribsARB"),
		};

		ENGINE_ASSERT(wglMakeCurrent(nullptr, nullptr), "Unable to make WGL render context non-current - ", getLastErrorMessage());
		ENGINE_ASSERT(wglDeleteContext(tempRenderContext), "Unable to delete temporary WGL render context - ", getLastErrorMessage());
		ReleaseDC(tempWindow, tempDeviceContext);
		ENGINE_ASSERT(DestroyWindow(tempWindow), "Unable to destroy temporary window for WGL function loading - ", getLastErrorMessage());

		return pointers;
	}
	
	LRESULT OpenGLWindow::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		// TODO: dont need to get window for messages we dont handle
		// TODO: HANDLE_MESSAGE(WM_PAINT); style

		switch (uMsg) {
			// TODO: WM_SIZE, and WM_(create?) should also call sizing callback
			case WM_SIZE: {
				const int32 w = LOWORD(lParam);
				const int32 h = HIWORD(lParam);
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				window.resizeCallback(window.userdata, w, h);
				break;
			}
			case WM_CLOSE: {
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				window.close = true;
				break;
			}
			case WM_SYSKEYDOWN: // TODO: raw input plz
			case WM_KEYDOWN: {
				// As far as i can tell there is no way to get a more precise timestamp
				// GetMessageTime is in ms and usually has a resolution of 10ms-16ms
				// https://devblogs.microsoft.com/oldnewthing/20140122-00/?p=2013
				const int16 scancode = (lParam & 0xFF'00'00) >> 16;
				const bool extended = lParam & (1 << 24);
				const bool repeat = lParam & (1 << 30);
				if (!repeat) {
					auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
					window.keyPressCallback(window.userdata, scancode, extended);
				}
				break;
			}
			case WM_SYSKEYUP:
			case WM_KEYUP: {
				const int16 scancode = (lParam & 0xFF'00'00) >> 16;
				const bool extended = lParam & (1 << 24);
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				window.keyReleaseCallback(window.userdata, scancode, extended);
				break;
			}
			case WM_CHAR: {
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				window.charCallback(window.userdata, static_cast<wchar_t>(wParam));
				break;
			}
			case WM_MOUSEMOVE: {
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

				if (!window.mouseInWindow) {
					TRACKMOUSEEVENT event = {
						.cbSize = sizeof(event),
						.dwFlags = TME_LEAVE,
						.hwndTrack = hWnd,
						.dwHoverTime = 0,
					};
					TrackMouseEvent(&event);
					window.mouseEnterCallback(window.userdata);
					window.mouseInWindow = true;
				}

				const int32 x = GET_X_LPARAM(lParam);
				const int32 y = GET_Y_LPARAM(lParam);

				if (x != window.lastMousePos.x) {
					window.lastMousePos.x = x;
					window.mouseMoveCallback(window.userdata, 0, x);
				}

				if (y != window.lastMousePos.y) {
					window.lastMousePos.y = y;
					window.mouseMoveCallback(window.userdata, 1, y);
				}

				break;
			}
			case WM_LBUTTONDOWN: {
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				window.mousePressCallback(window.userdata, 0);
				break;
			}
			case WM_LBUTTONUP: {
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				window.mouseReleaseCallback(window.userdata, 0);
				break;
			}
			case WM_RBUTTONDOWN: {
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				window.mousePressCallback(window.userdata, 1);
				break;
			}
			case WM_RBUTTONUP: {
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				window.mouseReleaseCallback(window.userdata, 1);
				break;
			}
			case WM_MOUSEWHEEL: { // TODO: Make axis
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				window.mouseWheelCallback(window.userdata, 0.0f, GET_WHEEL_DELTA_WPARAM(wParam) / static_cast<float32>(WHEEL_DELTA));
				break;
			}
			case WM_MOUSEHWHEEL: { // TODO: Make axis
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				window.mouseWheelCallback(window.userdata, GET_WHEEL_DELTA_WPARAM(wParam) / static_cast<float32>(WHEEL_DELTA), 0.0f);
				break;
			}
			case WM_MOUSELEAVE: {
				auto& window = *reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				window.mouseInWindow = false;
				window.mouseLeaveCallback(window.userdata);
				break;
			}
			default:
				return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
	
		return 0;
	}
}
