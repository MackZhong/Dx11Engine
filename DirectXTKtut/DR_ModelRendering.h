#pragma once
#include "Game_DR.h"

class DR_ModelRendering :
	public Game_DR
{
public:
	DR_ModelRendering();
	~DR_ModelRendering();

private:
	virtual LPCWSTR GetClass() const override { return L"DR_DirectX11WindowClass"; }


	virtual LPCWSTR GetTitle() const override { return L"Device Resource Model Rendering"; }


	virtual void OnRender(ID3D11DeviceContext1 * context) override;
	std::unique_ptr<DirectX::Model> m_model;

	virtual void OnDeviceDependentResources(ID3D11Device * device);


	virtual void OnWindowSizeDependentResources(int width, int height) override;

	virtual void OnDeviceLost();

	virtual void OnUpdate(DX::StepTimer const& timer) override;

};

