//
// Game.h
//

#pragma once

#include "StepTimer.h"

using Microsoft::WRL::ComPtr;

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
	friend LRESULT GameWindowProc(HWND, UINT, WPARAM, LPARAM);
private:
	virtual LPCWSTR GetClass() const = 0;
	virtual LPCWSTR GetTitle() const = 0;
public:
	Game();
	int Run(HINSTANCE hInstance);

	// Properties
	void GetDefaultSize(int& width, int& height) const {
		// TODO: Change to desired default window size (note minimum size is 320x200).
		width = 800;
		height = 600;
	};

protected:
	// Initialization and management
	void Initialize(HWND window, int width, int height);
	// Basic game loop
	void Tick();
	void Update(DX::StepTimer const& timer);
	// Updates the world.
	virtual void OnUpdate(DX::StepTimer const& timer) = 0;
	void Render();
	// Draws the scene.
	virtual void OnRender() = 0;
	// Helper method to clear the back buffers.
	void Clear();
	// Presents the back buffer contents to the screen.
	void Present();

	// Messages
	virtual void OnActivated() = 0;
	virtual void OnDeactivated() = 0;
	virtual void OnSuspending() = 0;
	void Resuming();
	// Game is being power-resumed (or returning from minimize).
	virtual void OnResuming() = 0;
	void ChangeWindowSize(int width, int height);
	// Game window is being resized.
	virtual void OnWindowSizeChanged(int width, int height) = 0;

	// resource management

	void CreateDevice();
	// These are the resources that depend on the device.
	// Initialize device dependent objects here (independent of window size).
	virtual void CreateDeviceDependentResource() = 0;
	void CreateResources();
	// Allocate all memory resources that change on a window SizeChanged event.
	// Initialize windows-size dependent objects here.
	virtual void CreateWindowDependentResource(UINT width, UINT height) = 0;

	void DeviceLost();
	// 	Direct3D resource cleanup here.
	virtual void OnDeviceLost() = 0;

protected:

	// Device resources.
	HWND                                            m_window;
	int                                             m_outputWidth;
	int                                             m_outputHeight;

	D3D_FEATURE_LEVEL                               m_featureLevel;
	ComPtr<ID3D11Device1>           m_d3dDevice;
	ComPtr<ID3D11DeviceContext1>    m_d3dContext;

	ComPtr<IDXGISwapChain1>         m_swapChain;
	ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
	ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

	// Rendering loop timer.
	DX::StepTimer                                   m_timer;

	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;
	DirectX::SimpleMath::Matrix m_proj;
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Vector3 m_cameraPos;
	float m_pitch;
	float m_yaw;
};