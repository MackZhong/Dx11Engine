#include "EnginePCH.h"
#include "DR_ModelRendering.h"
#include "ModelPLY.h"
#include "MaterialPly.h"

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
	std::shared_ptr<MaterialPlySimple> material = MaterialPlySimple::CreateFromPak(device,
		L"D:/Games/Men of War Assault Squad 2/resource/entity/e1.pak", "flora/bush/alder_little/01-material.mtl");

	//auto path = std::make_unique<wchar_t[]>(MAX_PATH);
	//GetCurrentDirectoryW(MAX_PATH, path.get());E:/Games/Men of War Assault Squad
	//SetCurrentDirectoryW(L"assets");
	//m_model = Model::CreateFromCMO(device, L"cup.cmo", *m_fxFactory);
	//SetCurrentDirectoryW(path.get());
	m_model = ModelPLY::CreateFromPAK(device, L"E:/Games/Men of War Assault Squad/resource/entity/e1.pak",
		"humanskin/ger-rifleman/skin.ply");
	//"-vehicle/airborne/a6m_m21_86/cockpit.ply");
	if (nullptr == m_model) {
		throw "Load model failed.";
	}
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
