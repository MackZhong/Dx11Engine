#include "EnginePCH.h"
#include "SimpleRendering.h"


SimpleRendering::SimpleRendering()
{
}


SimpleRendering::~SimpleRendering()
{
}

LPCWSTR SimpleRendering::GetClass() const
{
	return L"DirectXTK_Tutorial_Class";
}

LPCWSTR SimpleRendering::GetTitle() const
{
	return L"Simple Rendering Tutorial";
}

void SimpleRendering::OnUpdate(DX::StepTimer const& timer)
{
	// TODO:;
}

void SimpleRendering::OnRender()
{
	m_d3dContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	m_d3dContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
	m_d3dContext->RSSetState(m_states->CullNone());

	m_effect->Apply(m_d3dContext.Get());

	m_d3dContext->IASetInputLayout(m_inputLayout.Get());

	m_batch->Begin();

	//VertexPositionColor v1(Vector3(0.f, 0.5f, 0.5f), Colors::Yellow);
	//VertexPositionColor v2(Vector3(0.5f, -0.5f, 0.5f), Colors::Yellow);
	//VertexPositionColor v3(Vector3(-0.5f, -0.5f, 0.5f), Colors::Yellow);
	VertexPositionColor v1(Vector3(400.f, 150.f, 0.f), Colors::Yellow);
	VertexPositionColor v2(Vector3(600.f, 450.f, 0.f), Colors::Blue);
	VertexPositionColor v3(Vector3(200.f, 450.f, 0.f), Colors::Green);

	m_batch->DrawTriangle(v1, v2, v3);

	m_batch->End();
}

void SimpleRendering::OnActivated()
{
	// TODO:;
}

void SimpleRendering::OnDeactivated()
{
	// TODO:;
}

void SimpleRendering::OnSuspending()
{
	// TODO:;
}

void SimpleRendering::OnResuming()
{
	// TODO:;
}

void SimpleRendering::OnWindowSizeChanged(int width, int height)
{
	// TODO:;
}

void SimpleRendering::CreateDeviceDependentResource()
{
	m_states = std::make_unique<CommonStates>(m_d3dDevice.Get());

	m_effect = std::make_unique<BasicEffect>(m_d3dDevice.Get());
	m_effect->SetVertexColorEnabled(true);

	void const* shaderByteCode;
	size_t byteCodeLength;

	m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	DX::ThrowIfFailed(
		m_d3dDevice->CreateInputLayout(VertexPositionColor::InputElements,
			VertexPositionColor::InputElementCount,
			shaderByteCode, byteCodeLength,
			m_inputLayout.ReleaseAndGetAddressOf()));

	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_d3dContext.Get());
}

void SimpleRendering::CreateWindowDependentResource(UINT backBufferWidth, UINT backBufferHeight)
{
	// TODO:
	Matrix proj = Matrix::CreateScale(2.f / float(backBufferWidth),
		-2.f / float(backBufferHeight), 1.f)
		* Matrix::CreateTranslation(-1.f, 1.f, 0.f);
	m_effect->SetProjection(proj);
}

void SimpleRendering::OnDeviceLost()
{
	m_states.reset();
	m_effect.reset();
	m_batch.reset();
	m_inputLayout.Reset();
}
