//
// Game_DR.cpp
//

#include "EnginePCH.h"
#include "Game_DR.h"

extern void ExitGame_DR();

using namespace DirectX;

using Microsoft::WRL::ComPtr;


// Windows procedure
LRESULT CALLBACK GameWindowProc_DR(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	static bool s_in_sizemove = false;
	static bool s_in_suspend = false;
	static bool s_minimized = false;
	static bool s_fullscreen = false;
	// TODO: Set s_fullscreen to true if defaulting to fullscreen.

	auto game = reinterpret_cast<Game_DR*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse::ProcessMessage(message, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(message, wParam, lParam);
		break;

	case WM_PAINT:
		if (s_in_sizemove && game)
		{
			game->Tick();
		}
		else
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_MOVE:
		if (game)
		{
			game->OnWindowMoved();
		}
		break;

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
		{
			if (!s_minimized)
			{
				s_minimized = true;
				if (!s_in_suspend && game)
					game->Suspending();
				s_in_suspend = true;
			}
		}
		else if (s_minimized)
		{
			s_minimized = false;
			if (s_in_suspend && game)
				game->Resuming();
			s_in_suspend = false;
		}
		else if (!s_in_sizemove && game)
		{
			game->ChangeWindowSize(LOWORD(lParam), HIWORD(lParam));
		}
		break;

	case WM_ENTERSIZEMOVE:
		s_in_sizemove = true;
		break;

	case WM_EXITSIZEMOVE:
		s_in_sizemove = false;
		if (game)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);

			game->ChangeWindowSize(rc.right - rc.left, rc.bottom - rc.top);
		}
		break;

	case WM_GETMINMAXINFO:
	{
		auto info = reinterpret_cast<MINMAXINFO*>(lParam);
		info->ptMinTrackSize.x = 320;
		info->ptMinTrackSize.y = 200;
	}
	break;

	case WM_ACTIVATEAPP:
		if (game)
		{
			if (wParam)
			{
				game->Activated();
			}
			else
			{
				game->Deactivated();
			}
		}
		Keyboard::ProcessMessage(message, wParam, lParam);
		Mouse::ProcessMessage(message, wParam, lParam);
		break;

	case WM_POWERBROADCAST:
		switch (wParam)
		{
		case PBT_APMQUERYSUSPEND:
			if (!s_in_suspend && game)
				game->Suspending();
			s_in_suspend = true;
			return TRUE;

		case PBT_APMRESUMESUSPEND:
			if (!s_minimized)
			{
				if (s_in_suspend && game)
					game->Resuming();
				s_in_suspend = false;
			}
			return TRUE;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_MENUCHAR:
		// A menu is active and the user presses a key that does not correspond
		// to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
		return MAKELRESULT(0, MNC_CLOSE);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

int Game_DR::Run(HINSTANCE hInstance) {
	if (!XMVerifyCPUSupport())
		return 1;

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))
		return 2;

	auto path = std::make_unique<wchar_t[]>(MAX_PATH);
	GetCurrentDirectoryW(MAX_PATH, path.get());
	wchar_t iconFile[] = L"directx.ico";
	wchar_t enginePath[] = L"Engine\\";
	if (PathFileExists(iconFile))
	{
		PathCombineW(path.get(), path.get(), iconFile);
	}
	else if (PathFileExists(enginePath)) {
		PathCombineW(path.get(), path.get(), enginePath);
		if (PathFileExistsW(path.get())) {
			PathCombineW(path.get(), path.get(), iconFile);
		}
	}
	// Register class and create window
	{
		// Register class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = GameWindowProc_DR;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = (HICON)LoadImageW(NULL, path.get(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = this->GetClass();
		wcex.hIconSm = wcex.hIcon;
		if (!RegisterClassEx(&wcex))
			return 1;

		// Create window
		int w, h;
		this->GetDefaultSize(w, h);

		RECT rc;
		rc.top = 0;
		rc.left = 0;
		rc.right = static_cast<LONG>(w);
		rc.bottom = static_cast<LONG>(h);

		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		HWND hwnd = CreateWindowEx(0, this->GetClass(), this->GetTitle(), WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
			nullptr);
		// TODO: Change to CreateWindowEx(WS_EX_TOPMOST, L"Dx11Tut01WindowClass", L"Dx11Tut01-Sprites", WS_POPUP,
		// to default to fullscreen.

		if (!hwnd)
			return 1;

		ShowWindow(hwnd, SW_SHOW);
		// TODO: Change nCmdShow to SW_SHOWMAXIMIZED to default to fullscreen.

		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		GetClientRect(hwnd, &rc);

		this->Initialize(hwnd, rc.right - rc.left, rc.bottom - rc.top);
	}

	// Main message loop
	MSG msg = {};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			this->Tick();
		}
	}

	//g_game.reset();

	CoUninitialize();

	return (int)msg.wParam;
}

Game_DR::Game_DR()
{
	m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game_DR::Initialize(HWND window, int width, int height, bool fixedtimer)
{
	m_deviceResources->SetWindow(window, width, height);

	m_deviceResources->CreateDeviceResources();
	CreateDeviceDependentResources(m_deviceResources->GetD3DDevice());

	m_deviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources(width, height);

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	if (fixedtimer) {
		m_timer.SetFixedTimeStep(true);
		m_timer.SetTargetElapsedSeconds(1.0 / 60);
	}
}

#pragma region Frame Update
// Executes the basic game loop.
void Game_DR::Tick()
{
	m_timer.Tick([&]()
	{
		Update(m_timer);
	});

	Render();
}

// Updates the world.
void Game_DR::Update(DX::StepTimer const& timer)
{

	// TODO: Add your game logic here.
	OnUpdate(timer);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game_DR::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return;
	}

	Clear();

	m_deviceResources->PIXBeginEvent(L"Render");
	auto context = m_deviceResources->GetD3DDeviceContext();

	// TODO: Add your rendering code here.
	OnRender(context);

	m_deviceResources->PIXEndEvent();

	// Show the new frame.
	m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game_DR::Clear()
{
	m_deviceResources->PIXBeginEvent(L"Clear");

	// Clear the views.
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTarget = m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_deviceResources->GetDepthStencilView();

	context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);

	// Set the viewport.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game_DR::Activated()
{
	// TODO: Game_DR is becoming active window.
	OnActivated();
}

void Game_DR::Deactivated()
{
	// TODO: Game_DR is becoming background window.
	OnDeactivated();
}

void Game_DR::Suspending()
{
	// TODO: Game_DR is being power-suspended (or minimized).
	OnSuspending();
}

void Game_DR::Resuming()
{
	m_timer.ResetElapsedTime();

	// TODO: Game_DR is being power-resumed (or returning from minimize).
	OnResuming();
}

void Game_DR::OnWindowMoved()
{
	auto r = m_deviceResources->GetOutputSize();
	m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game_DR::ChangeWindowSize(int width, int height)
{
	if (!m_deviceResources->WindowSizeChanged(width, height))
		return;

	CreateWindowSizeDependentResources(width, height);

	// TODO: Game_DR window is being resized.
	OnWindowSizeChanged(width, height);
}

// Properties
void Game_DR::GetDefaultSize(int& width, int& height) const
{
	// TODO: Change to desired default window size (note minimum size is 320x200).
	width = 800;
	height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game_DR::CreateDeviceDependentResources(ID3D11Device * device)
{
	m_states = std::make_unique<CommonStates>(device);

	m_fxFactory = std::make_unique<EffectFactory>(device);

	m_world = Matrix::Identity;

	// TODO: Initialize device dependent objects here (independent of window size).
	OnDeviceDependentResources(device);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game_DR::CreateWindowSizeDependentResources(int width, int height)
{
	m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f),
		Vector3::Zero, Vector3::UnitY);
	m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
		float(width) / float(height), 0.1f, 10.f);

	// TODO: Initialize windows-size dependent objects here.
	OnWindowSizeDependentResources(width, height);
}

void Game_DR::OnDeviceLost()
{
	// TODO: Add Direct3D resource cleanup here.
	m_states.reset();
	m_fxFactory.reset();
}

void Game_DR::OnDeviceRestored()
{
	CreateDeviceDependentResources(m_deviceResources->GetD3DDevice());

	auto r = m_deviceResources->GetOutputSize();
	OnWindowSizeDependentResources(r.right, r.bottom);
}
#pragma endregion


// Exit helper
void ExitGame_DR()
{
	PostQuitMessage(0);
}