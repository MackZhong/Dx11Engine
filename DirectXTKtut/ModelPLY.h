#pragma once
#include "Model.h"

class ModelPLY : public Model
{
public:
	ModelPLY();
	ModelPLY(const ModelPLY&) = default;
	~ModelPLY();

	// Loads a model from memory
	static std::unique_ptr<Model> __cdecl CreateFromPLY(_In_ ID3D11Device* d3dDevice, _In_reads_bytes_(dataSize) const uint8_t* meshData, _In_ size_t dataSize,
		_In_opt_ std::shared_ptr<IEffect> ieffect = nullptr, bool ccw = false, bool pmalpha = false);
	// Loads a model from a .PLY file
	static std::unique_ptr<Model> __cdecl CreateFromPLY(_In_ ID3D11Device* d3dDevice, _In_z_ const wchar_t* szFileName,
		_In_opt_ std::shared_ptr<IEffect> ieffect = nullptr, bool ccw = false, bool pmalpha = false);
	// Loads a model from a .PAK file
	static std::unique_ptr<Model> __cdecl CreateFromPAK(_In_ ID3D11Device* d3dDevice, _In_z_ const wchar_t* szFileName, _In_z_ const char* entity,
		_In_opt_ std::shared_ptr<IEffect> ieffect = nullptr, bool ccw = false, bool pmalpha = false);
};

struct PlyMeshBlock {
	PlyMeshBlock() = default;
	PlyMeshBlock(const PlyMeshBlock&) = default;

	struct Mesh {
		UINT32 unknownFlags;
		UINT32 facesOffset;
		UINT32 facesTriangles;
		UINT32 matType;
		UINT32 color;
	};
	const Mesh* mesh;
	struct SkinIndex {
		UINT16 skin2;
		USHORT* indexs;
	};
	const SkinIndex *skinIndex;
	std::wstring name;
};

struct PlyVertBlock {
	struct Vert {
		UINT32 numVertices;
		UINT16 vertexStride;
		UINT16 u1;
	}vert;
	const byte data[1];
};

struct PlyIndxBlock {
	UINT32 numIndices;
	const UINT16 data[1];
};

struct PlyMaterial
{
	DirectX::XMFLOAT4   Ambient;
	DirectX::XMFLOAT4   Diffuse;
	DirectX::XMFLOAT4   Specular;
	float               SpecularPower;
	DirectX::XMFLOAT4   Emissive;
	DirectX::XMFLOAT4X4 UVTransform;
};

struct MaterialRecordPLY
{
	const PlyMaterial*   pMaterial;
	std::wstring                    name;
	std::wstring                    pixelShader;
	std::wstring                    texture[4];
	std::shared_ptr<IEffect>        effect;
	ComPtr<ID3D11InputLayout>       il;

	MaterialRecordPLY() DIRECTX_NOEXCEPT : pMaterial(nullptr) {}
};

struct PlyMaterialBump
{
	DirectX::XMFLOAT4   Diffuse;
	std::string bump;
	bool blend;
};