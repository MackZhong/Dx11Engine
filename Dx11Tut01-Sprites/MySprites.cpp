#include "pch.h"
#include "MySprites.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

MySprites::MySprites()
{
}


MySprites::~MySprites()
{
}

// Updates the world.
void MySprites::OnUpdate(float elapsedTime)
{
	// TODO: Add your game logic here.
}

// Draws the scene.
void MySprites::OnRender()
{
	// TODO: Add your rendering code here.
	m_spriteBatch->Begin();

	m_spriteBatch->Draw(m_texture.Get(), m_screenPos, nullptr, Colors::White,
		0.f, m_origin);

	m_spriteBatch->End();
}

// Message handlers
void MySprites::OnActivated()
{
	// TODO: Game is becoming active window.
}

void MySprites::OnDeactivated()
{
	// TODO: Game is becoming background window.
}

void MySprites::OnSuspending()
{
	// TODO: Game is being power-suspended (or minimized).
}

void MySprites::OnResuming()
{
	// TODO: Game is being power-resumed (or returning from minimize).
}

void MySprites::OnWindowSizeChanged(int width, int height)
{
	// TODO: Game window is being resized.
}

void MySprites::CreateDeviceDependentResource()
{
	// TODO: Initialize device dependent objects here (independent of window size).

	m_spriteBatch = std::make_unique<SpriteBatch>(m_d3dContext.Get());

	auto pathBuf = std::make_unique<wchar_t[]>(MAX_PATH);
	GetCurrentDirectoryW(MAX_PATH, pathBuf.get());
	//MessageBoxW(NULL, pathBuf.get(), L"CurrentDirectory", MB_OK);
	OutputDebugStringW(pathBuf.get());
	OutputDebugStringW(L"\n");

	ComPtr<ID3D11Resource> resource;
	DX::ThrowIfFailed(
		CreateWICTextureFromFile(m_d3dDevice.Get(), L"cat.png",
			resource.GetAddressOf(),
			m_texture.ReleaseAndGetAddressOf()));

	ComPtr<ID3D11Texture2D> cat;
	DX::ThrowIfFailed(resource.As(&cat));

	CD3D11_TEXTURE2D_DESC catDesc;
	cat->GetDesc(&catDesc);

	m_origin.x = float(catDesc.Width / 2);
	m_origin.y = float(catDesc.Height / 2);
}

void MySprites::CreateWindowDependentResource(UINT width, UINT height)
{
	// TODO: Initialize windows-size dependent objects here.
	
	m_screenPos.x = width / 2.f;
	m_screenPos.y = height / 2.f;
}

void MySprites::OnDeviceLost()
{
	// TODO: Add Direct3D resource cleanup here.

	m_texture.Reset();
	m_spriteBatch.reset();
}
