#pragma once
using namespace std;

class MaterialPlySimple
{
	std::string name;
	string diffuse;
	string blend;	// none,test
	bool tile{ false };
	bool alphatocoverage{ false };

public:
	MaterialPlySimple();
	~MaterialPlySimple();

	static std::shared_ptr<MaterialPlySimple> CreateFromPak(ID3D11Device* device, LPCWSTR szPakFile, LPCSTR szMatFile);
private:
	static std::shared_ptr<MaterialPlySimple> CreateFromMTL(ID3D11Device* d3dDevice, const uint8_t* meshData, size_t dataSize);
};

class MaterialPlyBump :
	public MaterialPlySimple
{
	string specular;
	UINT32 color;

public:
	MaterialPlyBump();
	~MaterialPlyBump();
};

