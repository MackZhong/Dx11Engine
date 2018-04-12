#include "EnginePCH.h"
#include "RenderingModel.h"
#include "Helper.h"

//#include "BinaryReader.h"

using namespace DX;

//std::unique_ptr<Model> CreateFromPLY(ID3D11Device* d3dDevice, const wchar_t* szFileName,
//	std::shared_ptr<IEffect> ieffect, bool ccw, bool pmalpha)
//{
//	size_t dataSize = 0;
//	std::unique_ptr<uint8_t[]> data;
//	HRESULT hr = BinaryReader::ReadEntireFile(szFileName, data, &dataSize);
//	if (FAILED(hr))
//	{
//		DebugTrace("CreateFromPLY failed (%08X) loading '%ls'\n", hr, szFileName);
//		throw std::exception("CreateFromVBO");
//	}
//
//	auto model = CreateFromPLY(d3dDevice, data.get(), dataSize, ieffect, ccw, pmalpha);
//
//	model->name = szFileName;
//
//	return model;
//}

namespace
{
	//--------------------------------------------------------------------------------------
	// Shared VB input element description
	INIT_ONCE g_InitOnce = INIT_ONCE_STATIC_INIT;
	std::shared_ptr<std::vector<D3D11_INPUT_ELEMENT_DESC>> g_vbdecl;

	BOOL CALLBACK InitializeDecl(PINIT_ONCE initOnce, PVOID Parameter, PVOID *lpContext)
	{
		UNREFERENCED_PARAMETER(initOnce);
		UNREFERENCED_PARAMETER(Parameter);
		UNREFERENCED_PARAMETER(lpContext);

		// TODO: dynamic config vertex buffer layout
		g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
			VertexPositionNormalTexture::InputElements,
			VertexPositionNormalTexture::InputElements + VertexPositionNormalTexture::InputElementCount);
		//g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
		//	VertexPositionNormalColorTexture::InputElements,
		//	VertexPositionNormalColorTexture::InputElements + VertexPositionNormalColorTexture::InputElementCount);

		return TRUE;
	}
}


std::unique_ptr<Model> CreateFromPLY(ID3D11Device* d3dDevice, const uint8_t* pBuf, size_t dataSize,
	std::shared_ptr<IEffect> ieffect, bool ccw, bool pmalpha)
{
	const uint8_t* pBufBase = pBuf;

	if (!InitOnceExecuteOnce(&g_InitOnce, InitializeDecl, nullptr, nullptr))
		throw std::exception("One-time initialization failed");

	if (!d3dDevice || !pBuf)
		throw std::exception("Device and meshData cannot be null");

	// File Header
	if (*(PUINT64)pBuf != *(PUINT64)"EPLYBNDS") {
		OutputDebugStringW(L"Invalid ply file.\n");
		return nullptr;
	}
	pBuf += 8;

	XMFLOAT3 position1 = *(XMFLOAT3*)pBuf;
	pBuf += 12;
	XMFLOAT3 position2 = *(XMFLOAT3*)pBuf;
	pBuf += 12;

	std::list<std::string> m_Skins;
	std::list<UINT16> m_SkinIndices;
	UINT32 m_Faces;
	std::string m_MaterialName;

	UINT32 numVertices = 0;
	UINT32 numIndices = 0;
	ComPtr<ID3D11Buffer> vb;
	ComPtr<ID3D11Buffer> ib;

	const UINT32 SKIN = 0x4e494b53; // "SKIN";
	const UINT32 MESH = 0x4853454d; // "MESH";
	const UINT32 VERT = 0x54524556; // "VERT";
	const UINT32 INDX = 0x58444e49; // "INDX";
	while ((size_t)(pBuf - pBufBase) < dataSize - 4) {
		UINT32 magicK = *(PUINT32)pBuf;
		pBuf += 4;
		switch (magicK) {
		case SKIN:
		{
			//skins, = struct.unpack("<I", f.read(4))
			//	print("Number of skins: %i at %s" % (skins, hex(f.tell())))
			//	for i in range(0, skins) :
			//		skin_name_length, = struct.unpack("B", f.read(1))
			//		print("Skin name length:", hex(skin_name_length))
			//		skin_name = f.read(skin_name_length)
			//		print("Skin name:", skin_name)
			UINT32 skinsCount = *(PUINT32)pBuf;
			pBuf += 4;
			OutputDebugStringW(L"Skins: ");
			while (skinsCount--) {
				std::string skin((LPCSTR)pBuf + 1, pBuf[0]);
				pBuf += 1 + pBuf[0];
				OutputDebugStringA(skin.c_str());
				OutputDebugStringA(" ");
				m_Skins.push_back(skin);
			}
			OutputDebugStringW(L".\n");
		}
		break;
		case MESH:
		{
			//# read 
			//	f.read(0x8)
			//	triangles, = struct.unpack("<I", f.read(4))
			//	print("Number of triangles:", triangles)
			//	self.material_info, = struct.unpack("<I", f.read(4))
			//	print("Material info:", hex(self.material_info))
			//	if self.material_info in SUPPORTED_FORMAT :
			//if self.material_info == 0x0404 :
			//	pass
			//	elif self.material_info == 0x0C14 :
			//	pass
			//else :
			//	vert = f.read(0x4)
			//	else:
			//raise Exception("Unsupported material type")
			//	material_name_length, = struct.unpack("B", f.read(1))
			//	print("Material name length:", hex(material_name_length))
			//	material_file = f.read(material_name_length)
			//	print("Material file:", material_file)
			//	# read some more unknown data
			//	if self.material_info == 0x0C14:
			//f.read(3)
			pBuf += 8;	// some unknown data
			m_Faces = *(PUINT32)pBuf;
			pBuf += 4;
			UINT32 matType = *(PUINT32)pBuf;
			pBuf += 4;
			// 0x0644, 0x0604, 0x0404, 0x0704, 0x0744, 0x0C14
			const UINT32 MAT_404 = 0x0404;
			const UINT32 MAT_604 = 0x0604;
			const UINT32 MAT_644 = 0x0644;
			const UINT32 MAT_704 = 0x0704;
			const UINT32 MAT_744 = 0x0744;
			const UINT32 MAT_C14 = 0x0C14;
			const UINT32 MAT_F14 = 0x0F14;
			switch (matType) {
			case MAT_404:
			case MAT_C14:
				break;
			default:
				UINT32 v = *(PUINT32)pBuf;
				pBuf += 4;
				break;
			}
			std::string material((LPCSTR)pBuf + 1, pBuf[0]);
			pBuf += 1 + pBuf[0];
			m_MaterialName = material;

			if (MAT_C14 == matType) {
				pBuf += 3;
			}
		}
		break;
		case VERT:
		{
			//verts, = struct.unpack("<I", f.read(4))
			//                 print("Number of verts: %i at %s" % (verts, hex(f.tell())))
			//                 vertex_description, = struct.unpack("<I", f.read(4))
			//                 print("Vertex description:", hex(vertex_description))
			//                 for i in range(0, verts):
			//                     if vertex_description == 0x00010024:
			//                         vx,vy,vz,nx,ny,nz,U,V = struct.unpack("ffffff4xff", f.read(36))
			//                     elif vertex_description == 0x00070020:
			//                         vx,vy,vz,nx,ny,nz,U,V = struct.unpack("ffffffff", f.read(32))
			//                     elif vertex_description == 0x00070028:
			//                         vx,vy,vz,nx,ny,nz,U,V = struct.unpack("ffffffff8x", f.read(40))
			//                     elif vertex_description == 0x00070030:
			//                         vx,vy,vz,nx,ny,nz,U,V = struct.unpack("ffffffff16x", f.read(48))
			//                     else:
			//                         raise Exception("Unknown format: %s" % hex(vertex_description))
			//                     if verbose:
			//                         print("Vertex %i: " % i,vx,vy,vz)
			//                     self.positions.append((vx,vy,vz))
			//                     self.normals.append((nx,ny,nz))
			//                     if not self.translate_uv_y:
			//                         self.UVs.append((U,V))
			//                     else:
			//                         self.UVs.append((U,V+1.0))
			//                 print("Vertex info ends at:",hex(f.tell()))
			numVertices = *(PUINT32)pBuf;
			pBuf += 4;
			UINT16 m_VertexStride = *(PUINT16)pBuf;
			//assert(sizeof(VertexPositionNormalColorTexture) == m_VertexStride);
			pBuf += 2;

			UINT32 u1 = *(PUINT16)pBuf;
			pBuf += 2;
			ULONG verticesBytes = numVertices * m_VertexStride;
			auto verts = reinterpret_cast<const VertexPositionNormalTexture*>(pBuf);
			//if (32 == m_VertexStride)
			//	verts = reinterpret_cast<const VertexPositionNormalTexture*>(pBuf);
			//else if (48 == m_VertexStride)
			//	verts = reinterpret_cast<const VertexPositionNormalColorTexture*>(pBuf);


			// Create vertex buffer
			{
				D3D11_BUFFER_DESC desc = {};
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.ByteWidth = static_cast<UINT>(verticesBytes);
				desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				D3D11_SUBRESOURCE_DATA initData = {};
				initData.pSysMem = pBuf;
				DX::ThrowIfFailed(
					d3dDevice->CreateBuffer(&desc, &initData, vb.GetAddressOf())
				);

				DirectX::SetDebugObjectName(vb.Get(), "ModelPLY");
			}
			pBuf += verticesBytes;
		}
		break;
		case INDX:
		{
			//idx_count, = struct.unpack("<I", f.read(4))
			//	print("Indeces:", idx_count)
			//	for i in range(0, idx_count / 3) :
			//		i0, i1, i2 = struct.unpack("<HHH", f.read(6))
			//		if verbose :
			//			print("Face %i:" % i, i0, i1, i2)
			//			if self.material_info == 0x0744 :
			//				self.indeces.append((i2, i1, i0))
			//			else :
			//				self.indeces.append((i0, i1, i2))
			//				print("Indces end at", hex(f.tell() - 1))
			numIndices = *(PUINT32)pBuf;
			pBuf += 4;
			ULONG indicesBytes = numIndices * sizeof(WORD);
			auto indices = reinterpret_cast<const WORD*>(pBuf);

			// Create index buffer
			{
				D3D11_BUFFER_DESC desc = {};
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.ByteWidth = static_cast<UINT>(indicesBytes);
				desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				D3D11_SUBRESOURCE_DATA initData = {};
				initData.pSysMem = indices;
				DX::ThrowIfFailed(
					d3dDevice->CreateBuffer(&desc, &initData, ib.GetAddressOf())
				);

				DirectX::SetDebugObjectName(ib.Get(), "ModelPLY");
			}
			pBuf += indicesBytes;
		}
		break;
		default:
			OutputDebugStringW(L"Invalid ply block.\n");
			return nullptr;
		}
	}

	// Create input layout and effect
	if (!ieffect)
	{
		auto effect = std::make_shared<BasicEffect>(d3dDevice);
		effect->EnableDefaultLighting();
		effect->SetLightingEnabled(true);

		ieffect = effect;
	}

	ComPtr<ID3D11InputLayout> il;
	{
		void const* shaderByteCode;
		size_t byteCodeLength;

		ieffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

		DX::ThrowIfFailed(
			d3dDevice->CreateInputLayout(VertexPositionNormalColorTexture::InputElements,
				VertexPositionNormalColorTexture::InputElementCount,
				shaderByteCode, byteCodeLength,
				il.GetAddressOf()));

		DirectX::SetDebugObjectName(il.Get(), "ModelPLY");
	}

	auto part = new ModelMeshPart();
	part->indexCount = numIndices;
	part->startIndex = 0;
	part->vertexStride = static_cast<UINT>(sizeof(VertexPositionNormalColorTexture));
	part->inputLayout = il;
	part->indexBuffer = ib;
	part->vertexBuffer = vb;
	part->effect = ieffect;
	part->vbDecl = g_vbdecl;

	auto mesh = std::make_shared<ModelMesh>();
	mesh->ccw = ccw;
	mesh->pmalpha = pmalpha;
	BoundingBox::CreateFromPoints(mesh->boundingBox,
		XMVectorSet(position1.x, position1.y, position1.z, 1.0f),
		XMVectorSet(position2.x, position2.y, position2.z, 1.0f));
	BoundingSphere::CreateFromBoundingBox(mesh->boundingSphere, mesh->boundingBox);
	//BoundingSphere::CreateFromPoints(mesh->boundingSphere, numVertices, &position1, sizeof(VertexPositionNormalColorTexture));
	//BoundingBox::CreateFromPoints(mesh->boundingBox, numVertices, &position1, sizeof(VertexPositionNormalColorTexture));
	mesh->meshParts.emplace_back(part);

	std::unique_ptr<Model> model(new Model());
	model->meshes.emplace_back(mesh);

	return model;
}

std::unique_ptr<Model> LoadEntityFromPak(ID3D11Device* d3dDevice, LPCWSTR pakFile, LPCSTR entity) {
	enum ZIP_SIGNATURE {
		LocalFileHeader = 0x04034b50,
		DataDescriptor = 0x08074b50,
		CentralDirectoryFileHeader = 0x02014b50,
		EOCD = 0x06054b50
	};
#pragma pack( show)
#pragma pack(push, 2)
#pragma pack( show)
	__declspec(align(2)) struct _ZipLocalFileHeader
	{
		WORD version;
		WORD bitflags;
		WORD comp_method;
		WORD lastModFileTime;
		WORD lastModFileDate;
		DWORD crc_32;
		DWORD comp_size;
		DWORD uncompr_size;
		WORD fname_len;
		WORD extra_field_len;
	};
	__declspec(align(2)) struct _ZipCDFHeader
	{
		WORD version_madeby;
		WORD version_needed;
		WORD bitflags;
		WORD comp_method;
		WORD lastModFileTime;
		WORD lastModFileDate;
		DWORD crc_32;
		DWORD comp_size;
		DWORD uncompr_size;
		WORD fname_len;
		WORD extra_field_len;
		WORD fcomment_len;
		WORD disk_num_start;
		WORD internal_fattribute;
		DWORD external_fattribute;
		DWORD relative_offset;
	};
	__declspec(align(2)) struct _ZipEOCD	//Offset Bytes Description
	{
		//0	4	End of central directory signature = 0x06054b50
		WORD numOfDisk;					//4	2	Number of this disk
		WORD diskCDStart;					//6	2	Disk where central directory starts
		WORD numOfCD;						//8	2	Number of central directory records on this disk
		WORD numTotalCD;					//10 2	Total number of central directory records
		DWORD sizeCD;						//12 4	Size of central directory(bytes)
		DWORD offsetCDStart;				//16 4	Offset of start of central directory, relative to start of archive
		WORD lenComment;					//20 2	Comment length(n)
											//22 n	Comment
	};
#pragma pack(pop)
#pragma pack(show)

	SafeHandle hFile = CreateFileW(pakFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		OutputDebugStringW(L"CreateFile failed.\n");
		return nullptr;
	}
	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize)) {
		OutputDebugStringW(L"GetFileSizeEx failed.\n");
		return nullptr;
	}

	struct _FileBlock {
		_ZipLocalFileHeader header;
		DWORD offset;
	};
	typedef std::map < std::string, std::shared_ptr<_FileBlock>> FileBlockMapType;
	FileBlockMapType fileBlockMap;

	DWORD signature;
	DWORD readedBytes;
	while (ReadFile(hFile, &signature, 4, &readedBytes, NULL)) {
		switch (signature) {
		case LocalFileHeader:
		{
			auto fileBlock = std::make_shared<_FileBlock>();
			DWORD stToRead = sizeof(_ZipLocalFileHeader);
			if (!ReadFile(hFile, &fileBlock->header, stToRead, &readedBytes, NULL)) {
				OutputDebugStringW(L"ReadFile for Local file header failed.\n");
				break;
			}
			//char* fileName = new char[fileBlock->header.fname_len + 1]{ 0 };
			auto fileName = std::make_unique<char[]>(fileBlock->header.fname_len + 1);
			ReadFile(hFile, fileName.get(), fileBlock->header.fname_len, &readedBytes, NULL);
			fileBlockMap[fileName.get()] = fileBlock;
			//delete[] fileName; fileName = nullptr;

			if (fileBlock->header.extra_field_len) {
				SetFilePointer(hFile, fileBlock->header.extra_field_len, 0, FILE_CURRENT);
			}
			fileBlock->offset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
			if (fileBlock->header.comp_size) {
				if (StrCmpNA(entity, fileName.get(), fileBlock->header.fname_len) == 0) {
					auto dataPtr = std::make_unique<byte[]>(fileBlock->header.comp_size);
					ReadFile(hFile, dataPtr.get(), fileBlock->header.comp_size, &readedBytes, NULL);
					return CreateFromPLY(d3dDevice, dataPtr.get(), fileBlock->header.comp_size,
						nullptr, false, false);
				}
				SetFilePointer(hFile, fileBlock->header.comp_size, 0, FILE_CURRENT);
			}
		}
		break;
		case DataDescriptor:
			OutputDebugStringW(L"Data descriptor not supported.\n");
			break;
		case CentralDirectoryFileHeader:
		{
			_ZipCDFHeader cdfHeader;
			if (!ReadFile(hFile, &cdfHeader, sizeof(cdfHeader), &readedBytes, NULL)) {
				OutputDebugStringW(L"ReadFile for Central directory file header failed.\n");
				break;
			}
			char* fileName = new char[cdfHeader.fname_len + 1]{ 0 };
			ReadFile(hFile, fileName, cdfHeader.fname_len, &readedBytes, NULL);
			delete[] fileName; fileName = nullptr;

			if (cdfHeader.extra_field_len) {
				SetFilePointer(hFile, cdfHeader.extra_field_len, 0, FILE_CURRENT);
			}

			if (cdfHeader.fcomment_len) {
				SetFilePointer(hFile, cdfHeader.fcomment_len, 0, FILE_CURRENT);
			}
		}
		break;
		case EOCD:
		{
			_ZipEOCD eocdBlock;
			DWORD stToRead = sizeof(eocdBlock);
			if (!ReadFile(hFile, &eocdBlock, stToRead, &readedBytes, NULL)) {
				OutputDebugStringW(L"ReadFile for End of central directory record failed.\n");
				break;
			}

			if (eocdBlock.lenComment) {
				SetFilePointer(hFile, eocdBlock.lenComment, 0, FILE_CURRENT);
			}
		}
		break;
		default:
			OutputDebugStringW(L"Unknown zip block.\n");
			break;
		}
		DWORD pos = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
		if (pos >= fileSize.LowPart) {
			OutputDebugStringW(L"File Ended.\n");
			break;;
		}
	}

	FileBlockMapType::iterator finded = fileBlockMap.find(entity);
	if (finded == fileBlockMap.end())
	{
		return nullptr;
	}

	return nullptr;
}


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

	m_world = Matrix::CreateRotationZ(cosf(time) * 0.2f);
}

void RenderingModel::OnRender()
{
	//m_d3dContext->RSSetState(m_states->CullCounterClockwise());
	m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, false);
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

	m_model = LoadEntityFromPak(m_d3dDevice.Get(), L"E:/Games/Men of War Assault Squad/resource/entity/e2.pak",
		"-vehicle/cannon/88mm_u_boat/turret.ply");
	//"-vehicle/airborne/a6m_m21_86/cockpit.ply");

//auto path = std::make_unique<wchar_t[]>(MAX_PATH);
//GetCurrentDirectoryW(MAX_PATH, path.get());
//SetCurrentDirectoryW(L"assets");
//m_model = Model::CreateFromCMO(m_d3dDevice.Get(), L"cup.cmo", *m_fxFactory);
//SetCurrentDirectoryW(path.get());

//m_model->UpdateEffects([](IEffect* effect) {
//	auto lights = dynamic_cast<IEffectLights*>(effect);
//	if (lights)
//	{
//		lights->SetLightingEnabled(true);
//		lights->SetPerPixelLighting(true);
//		lights->SetLightEnabled(0, true);
//		lights->SetLightDiffuseColor(0, Colors::Gold);
//		lights->SetLightEnabled(1, false);
//		lights->SetLightEnabled(2, false);
//	}

//	auto fog = dynamic_cast<IEffectFog*>(effect);
//	if (fog)
//	{
//		fog->SetFogEnabled(true);
//		fog->SetFogColor(Colors::CornflowerBlue);
//		fog->SetFogStart(3.f);
//		fog->SetFogEnd(4.f);
//	}
//});

	m_world = Matrix::Identity;
}

void RenderingModel::CreateWindowDependentResource(UINT backBufferWidth, UINT backBufferHeight)
{
	//m_view = Matrix::CreateLookAt(Vector3(12.f, 12.f, 12.f),
	//	Vector3::Zero, Vector3::UnitY);
	//m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 2.f,
	//	float(backBufferWidth) / float(backBufferHeight), 0.1f, 10.f);
}

void RenderingModel::OnDeviceLost()
{
	m_states.reset();
	m_fxFactory.reset();
	m_model.reset();
}
