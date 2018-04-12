//
// Game_DR.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game_DR : public DX::IDeviceNotify
{
private:
	virtual LPCWSTR GetClass() const = 0;
	virtual LPCWSTR GetTitle() const = 0;

	virtual void OnUpdate(DX::StepTimer const& timer) {};
	virtual void OnRender(ID3D11DeviceContext1 * context) = 0;
	virtual void OnActivated() { };
	virtual void OnDeactivated() { };
	virtual void OnSuspending() { };
	virtual void OnResuming() { };
	virtual void OnWindowSizeChanged(int width, int height) { };
	virtual void OnDeviceDependentResources(ID3D11Device* device) {};
	virtual void OnWindowSizeDependentResources(int width, int height) {};

protected:

public:

	Game_DR();
	int Run(HINSTANCE hInstance);

	// Initialization and management
	void Initialize(HWND window, int width, int height, bool fixedtimer = false);

	// Basic game loop
	void Tick();

	// IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;

	// Messages
	void Activated();
	void Deactivated();
	void Suspending();
	void Resuming();
	void OnWindowMoved();
	void ChangeWindowSize(int width, int height);

	// Properties
	void GetDefaultSize(int& width, int& height) const;

private:

	void Update(DX::StepTimer const& timer);
	void Render();

	void Clear();

	void CreateDeviceDependentResources(ID3D11Device * device);
	void CreateWindowSizeDependentResources(int width, int height);

	// Device resources.
	std::unique_ptr<DX::DeviceResources>    m_deviceResources;

	// Rendering loop timer.
	DX::StepTimer                           m_timer;

protected:
	DirectX::SimpleMath::Matrix m_world;
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_proj;

	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::IEffectFactory> m_fxFactory;
};