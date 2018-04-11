////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "graphicsclass.h"


GraphicsClass::GraphicsClass()
{
	m_Model = 0;
}

GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}

GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	// Initialize the Direct3D object.
	result = m_D3D.Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Set the initial position of the camera.
	m_Camera.SetPosition(0.0f, 0.0f, -10.0f);

	// Create the model object.
	m_Model = new ModelClass;
	if (!m_Model)
	{
		return false;
	}

	// Initialize the model object.
	result = m_Model->Initialize(m_D3D.GetDevice(), L"data/cube.txt", L"data/seafloor.dds");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// Initialize the light shader object.
	result = m_LightShader.Initialize(m_D3D.GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	// Initialize the light object.
	m_Light.SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light.SetDirection(0.0f, 0.0f, 1.0f);

	return true;
}

void GraphicsClass::Shutdown()
{
	// Release the model object.
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	return;
}

bool GraphicsClass::Frame()
{
	bool result;
	static float rotation = 0.0f;

	// Update the rotation variable each frame.
	rotation += (float)XM_PI * 0.00001f;
	if (rotation > 360.0f)
	{
		rotation -= 360.0f;
	}

	// Render the graphics scene.
	result = Render(rotation);
	if (!result)
	{
		return false;
	}

	return true;
}

bool GraphicsClass::Render(float rotation)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	// Clear the buffers to begin the scene.
	m_D3D.BeginScene(0.0f, 0.0f, 1.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera.Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	viewMatrix = m_Camera.GetViewMatrix();
	worldMatrix = m_D3D.GetWorldMatrix();
	projectionMatrix = m_D3D.GetProjectionMatrix();

	// Rotate the world matrix by the rotation value so that the triangle will spin.
	//D3DXMatrixRotationY(&worldMatrix, rotation);
	worldMatrix = XMMatrixRotationY(rotation);

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_Model->Render(m_D3D.GetDeviceContext());

	// Render the model using the light shader.
	result = m_LightShader.Render(m_D3D.GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model->GetTexture(), m_Light.GetDirection(), m_Light.GetDiffuseColor());

	if (!result)
	{
		return false;
	}

	// Present the rendered scene to the screen.
	m_D3D.EndScene();

	return true;
}