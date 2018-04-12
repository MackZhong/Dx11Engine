#include "EnginePCH.h"
#include "DR_ModelRendering.h"


DR_ModelRendering::DR_ModelRendering()
{
}


DR_ModelRendering::~DR_ModelRendering()
{
}

void DR_ModelRendering::OnRender(ID3D11DeviceContext1 * context)
{
	m_model->Draw(context, *m_states, m_world, m_view, m_proj);
}

void DR_ModelRendering::OnDeviceDependentResources(ID3D11Device * device)
{
	auto path = std::make_unique<wchar_t[]>(MAX_PATH);
	GetCurrentDirectoryW(MAX_PATH, path.get());
	SetCurrentDirectoryW(L"assets");
	m_model = Model::CreateFromCMO(device, L"cup.cmo", *m_fxFactory);
	SetCurrentDirectoryW(path.get());
}

void DR_ModelRendering::OnWindowSizeDependentResources(int width, int height)
{
	//auto r = m_deviceResources->GetOutputSize();
}

void DR_ModelRendering::OnDeviceLost()
{
	m_model.reset();
}

void DR_ModelRendering::OnUpdate(DX::StepTimer const& timer)
{
	float time = float(timer.GetTotalSeconds());

	m_world = Matrix::CreateRotationZ(cosf(time) * 2.f);
}
