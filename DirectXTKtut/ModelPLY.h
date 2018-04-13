#pragma once
#include "Model.h"

class ModelPLY : public Model
{
public:
	ModelPLY();
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

