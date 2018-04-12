#pragma once
#include "Game.h"


class RenderingModel :
	public Game
{
public:
	RenderingModel();
	~RenderingModel();
private:
	virtual LPCWSTR GetClass() const override;


	virtual LPCWSTR GetTitle() const override;

protected:
	virtual void OnUpdate(DX::StepTimer const& timer) override;


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
	DirectX::SimpleMath::Matrix m_world;

	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::IEffectFactory> m_fxFactory;
	std::unique_ptr<DirectX::Model> m_model;
};

