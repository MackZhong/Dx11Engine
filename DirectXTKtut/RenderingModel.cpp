#include "EnginePCH.h"
#include "RenderingModel.h"


RenderingModel::RenderingModel()
{
}


RenderingModel::~RenderingModel()
{
}

LPCWSTR RenderingModel::GetClass() const
{
	return L"RenderingModelClass";
}

LPCWSTR RenderingModel::GetTitle() const
{
	return L"Rendering Model";
}

void RenderingModel::OnUpdate(DX::StepTimer const& timer)
{
	float time = float(timer.GetTotalSeconds());

	m_world = Matrix::CreateRotationZ(cosf(time) * 2.f);
}

void RenderingModel::OnRender()
{
	m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
}

void RenderingModel::OnActivated()
{
	// TODO:;
}

void RenderingModel::OnDeactivated()
{
	// TODO:;
}

void RenderingModel::OnSuspending()
{
	// TODO:;
}

void RenderingModel::OnResuming()
{
	// TODO:;
}

void RenderingModel::OnWindowSizeChanged(int width, int height)
{
	// TODO:;
}

void RenderingModel::CreateDeviceDependentResource()
{
	m_states = std::make_unique<CommonStates>(m_d3dDevice.Get());

	//m_fxFactory = std::make_unique<EffectFactory>(m_d3dDevice.Get());
	m_fxFactory = std::make_unique<DGSLEffectFactory>(m_d3dDevice.Get());

	auto path = std::make_unique<wchar_t[]>(MAX_PATH);
	GetCurrentDirectoryW(MAX_PATH, path.get());
	SetCurrentDirectoryW(L"assets");
	m_model = Model::CreateFromCMO(m_d3dDevice.Get(), L"cup.cmo", *m_fxFactory);
	SetCurrentDirectoryW(path.get());
	m_model->UpdateEffects([](IEffect* effect) {
		auto lights = dynamic_cast<IEffectLights*>(effect);
		if (lights)
		{
			lights->SetLightingEnabled(true);
			lights->SetPerPixelLighting(true);
			lights->SetLightEnabled(0, true);
			lights->SetLightDiffuseColor(0, Colors::Gold);
			lights->SetLightEnabled(1, false);
			lights->SetLightEnabled(2, false);
		}

		auto fog = dynamic_cast<IEffectFog*>(effect);
		if (fog)
		{
			fog->SetFogEnabled(true);
			fog->SetFogColor(Colors::CornflowerBlue);
			fog->SetFogStart(3.f);
			fog->SetFogEnd(4.f);
		}
	});

	m_world = Matrix::Identity;
}

void RenderingModel::CreateWindowDependentResource(UINT backBufferWidth, UINT backBufferHeight)
{
	m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f),
		Vector3::Zero, Vector3::UnitY);
	m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
		float(backBufferWidth) / float(backBufferHeight), 0.1f, 10.f);
}

void RenderingModel::OnDeviceLost()
{
	m_states.reset();
	m_fxFactory.reset();
	m_model.reset();
}
