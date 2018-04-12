#pragma once
#include "Game.h"

class MySprites :
	public Game
{
public:
	MySprites();
	~MySprites();

protected:

	virtual void OnUpdate(float elapsedTime) override;


	virtual void OnRender() override;

	virtual void OnActivated() override;
	virtual void OnDeactivated() override;
	virtual void OnSuspending() override;


	virtual void OnResuming() override;


	virtual void OnWindowSizeChanged(int width, int height) override;


	virtual void CreateDeviceDependentResource() override;


	virtual void CreateWindowDependentResource(UINT width, UINT height) override;


	virtual void OnDeviceLost() override;

private:
	// Resource
	ComPtr<ID3D11ShaderResourceView> m_texture;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	DirectX::SimpleMath::Vector2 m_screenPos;
	DirectX::SimpleMath::Vector2 m_origin;

	virtual LPCWSTR GetClass() const override { return  L"Dx11WindowClass"; }
	virtual LPCWSTR GetTitle() const override { return L"Dx11Tut01-Sprites"; }

};

