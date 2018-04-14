#include "EnginePCH.h"
#include "ModelPLY.h"


ModelPLY::ModelPLY()
{
}


ModelPLY::~ModelPLY()
{
}

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
		UINT16 stride = *(PUINT16)Parameter;
		UINT32 vertexSize = 0;
		switch (stride) {
		case 12:
			g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
				VertexPosition::InputElements,
				VertexPosition::InputElements + VertexPosition::InputElementCount);
			vertexSize = sizeof(VertexPosition);
			break;
		case 20:
			g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
				VertexPositionTexture::InputElements,
				VertexPositionTexture::InputElements + VertexPositionTexture::InputElementCount);
			vertexSize = sizeof(VertexPositionTexture);
			break;
		case 24:
			g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
				VertexPositionNormal::InputElements,
				VertexPositionNormal::InputElements + VertexPositionNormal::InputElementCount);
			vertexSize = sizeof(VertexPositionNormal);
			break;
		case 28:
			g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
				VertexPositionColor::InputElements,
				VertexPositionColor::InputElements + VertexPositionColor::InputElementCount);
			vertexSize = sizeof(VertexPositionColor);
			break;
			//case 28:
			//	g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
			//		VertexPositionDualTexture::InputElements,
			//		VertexPositionDualTexture::InputElements + VertexPositionDualTexture::InputElementCount);
			//vertexSize = sizeof(VertexPositionDualTexture);
			//	break;
		case 32:
			g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
				VertexPositionNormalTexture::InputElements,
				VertexPositionNormalTexture::InputElements + VertexPositionNormalTexture::InputElementCount);
			vertexSize = sizeof(VertexPositionNormalTexture);
			break;
		case 36:
			g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
				VertexPositionColorTexture::InputElements,
				VertexPositionColorTexture::InputElements + VertexPositionColorTexture::InputElementCount);
			vertexSize = sizeof(VertexPositionColorTexture);
			break;
		case 40:
			g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
				VertexPositionNormalColor::InputElements,
				VertexPositionNormalColor::InputElements + VertexPositionNormalColor::InputElementCount);
			vertexSize = sizeof(VertexPositionNormalColor);
			break;
		case 48:
			g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
				VertexPositionNormalColorTexture::InputElements,
				VertexPositionNormalColorTexture::InputElements + VertexPositionNormalColorTexture::InputElementCount);
			vertexSize = sizeof(VertexPositionNormalColorTexture);
			break;
		case 52:
			g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
				VertexPositionNormalTangentColorTexture::InputElements,
				VertexPositionNormalTangentColorTexture::InputElements + VertexPositionNormalTangentColorTexture::InputElementCount);
			vertexSize = sizeof(VertexPositionNormalTangentColorTexture);
			break;
		case 60:
			g_vbdecl = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
				VertexPositionNormalTangentColorTextureSkinning::InputElements,
				VertexPositionNormalTangentColorTextureSkinning::InputElements + VertexPositionNormalTangentColorTextureSkinning::InputElementCount);
			vertexSize = sizeof(VertexPositionNormalTangentColorTextureSkinning);
			break;
		default:
			return FALSE;
		}

		if (lpContext) {
			memcpy(*lpContext, &vertexSize, sizeof(UINT32));
		}

		return TRUE;
	}
}

std::unique_ptr<DirectX::Model> __cdecl ModelPLY::CreateFromPLY(_In_ ID3D11Device* d3dDevice, _In_reads_bytes_(dataSize) const uint8_t* meshData, _In_ size_t dataSize, _In_opt_ std::shared_ptr<IEffect> ieffect /*= nullptr*/, bool ccw /*= false*/, bool pmalpha /*= false*/)
{
	const uint8_t* pBufBase = meshData;

	if (!d3dDevice || !meshData)
		throw std::exception("Device and meshData cannot be null");

	// File Header
	if (*(PUINT64)meshData != *(PUINT64)"EPLYBNDS") {
		OutputDebugStringW(L"Invalid ply file.\n");
		return nullptr;
	}
	meshData += 8;

	XMFLOAT3 position1 = *(XMFLOAT3*)meshData;
	meshData += 12;
	XMFLOAT3 position2 = *(XMFLOAT3*)meshData;
	meshData += 12;

	std::list<std::string> skinNames;
	//std::list<UINT16> skinIndices;
	//UINT32 facesTriangles;
	//std::string materialName;

	//UINT32 numVertices = 0;
	//UINT16 vertexStride = 0;
	//UINT32 numIndices = 0;
	int skinBlockCount = 0;
	int meshBlocksCount = 0;
	UINT32 skinsCount = 0;
	//UINT32 meshCount = 0;
	ULONG indicesBytes = 0;
	ULONG verticesBytes = 0;

	const UINT32 SKIN = 0x4e494b53; // "SKIN";
	const UINT32 MESH = 0x4853454d; // "MESH";
	const UINT32 VERT = 0x54524556; // "VERT";
	const UINT32 INDX = 0x58444e49; // "INDX";
	const UINT32 ADJA = 0x414a4441; // "ADJA";
	const UINT32 SHDW = 0x57444853; // "SHDW";

	using PlyMeshBlockVector = std::vector<std::shared_ptr<struct PlyMeshBlock>>;
	PlyMeshBlockVector meshBlocks;

	const PlyVertBlock* vertBlock{ nullptr };
	const PlyIndxBlock* indxBlock{ nullptr };

	while ((size_t)(meshData - pBufBase) < dataSize - 4) {
		UINT32 magicK = *(PUINT32)meshData;
		meshData += 4;
		switch (magicK) {
		case SKIN:
		{
			skinBlockCount++;
			if (skinBlockCount > 1) {
				throw "Too many skin blocks.";
			}
			//skins, = struct.unpack("<I", f.read(4))
			//	print("Number of skins: %i at %s" % (skins, hex(f.tell())))
			//	for i in range(0, skins) :
			//		skin_name_length, = struct.unpack("B", f.read(1))
			//		print("Skin name length:", hex(skin_name_length))
			//		skin_name = f.read(skin_name_length)
			//		print("Skin name:", skin_name)
			skinsCount = *(PUINT32)meshData;
			meshData += 4;
			OutputDebugStringW(L"Skins: ");
			for (UINT32 s = 0; s < skinsCount; s++) {
				skinNames.emplace_back((LPCSTR)meshData + 1, meshData[0]);
				//std::string skin((LPCSTR)meshData + 1, meshData[0]);
				meshData += 1 + meshData[0];
				//OutputDebugStringA(skin.c_str());
				//OutputDebugStringA(" ");
				//skinNames.push_back(skin);
			}
			OutputDebugStringW(L".\n");
		}
		break;
		case MESH:
		{
			meshBlocksCount++;
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
			//UINT32 unknownFlags = *(PUINT32)meshData;	// Unknwon flags
			//meshData += 4;
			//UINT32 facesOffset = *(PUINT32)meshData;	// x3 is the offset of indices
			//meshData += 4;
			//facesTriangles = *(PUINT32)meshData;
			//meshData += 4;
			//UINT32 matType = *(PUINT32)meshData;
			//meshData += 4;
			auto meshBlock = std::make_shared<struct PlyMeshBlock>();
			meshBlock->mesh = reinterpret_cast<const PlyMeshBlock::Mesh*>(meshData);
			meshData += 16;

			switch (meshBlock->mesh->matType) {
			case 0x0c14:
				break;
			case 0x0005:
			case 0x0401:
			case 0x0404:
			case 0x0405:
			case 0x0406:
			case 0x040c:
			case 0x0444:
			case 0x0445:
			case 0x0504:
			case 0x0505:
			case 0x0506:
			case 0x0544:
			case 0x0c15:
			case 0x0c54:
			case 0x0c55:
			case 0x0d14:
				// simple material
				break;
			case 0x0604:
			case 0x0605:
			case 0x0644:
			case 0x0645:
			case 0x0704:
			case 0x0705:
			case 0x0744:
			case 0x0745:
			case 0x0e14:
			case 0x0f14:
			case 0x0f15:
			case 0x0f54:
			case 0x0f55:
				// bump material
				meshData += 4;
				break;
			default:
				meshData += 4;
				break;
			}
			//std::string material((LPCSTR)meshData + 1, meshData[0]);
			//materialName = material;
			meshBlock->name.assign((LPCSTR)meshData + 1, (LPCSTR)meshData + 1 + meshData[0]); // = std::string((LPCSTR)meshData + 1, meshData[0]);
			meshData += 1 + meshData[0];

			if (0x0C14 == meshBlock->mesh->matType) {
				meshData += 3;
			}
			// IMPORT TODO: skins index
			if (skinBlockCount) {
				meshBlock->skinIndex = reinterpret_cast<const PlyMeshBlock::SkinIndex*>(meshData);
				meshData += 1 + *(PUINT16)meshData[0];
			}
			else {
				meshBlock->skinIndex = nullptr;
			}
			meshBlocks.push_back(meshBlock);
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
			//numVertices = *(PUINT32)meshData;
			//meshData += 4;
			//vertexStride = *(PUINT16)meshData;
			//meshData += 2;

			//UINT16 u1 = *(PUINT16)meshData;
			//meshData += 2;
			vertBlock = reinterpret_cast<const PlyVertBlock*>(meshData);
			meshData += 8;

			verticesBytes = vertBlock->vert.numVertices * vertBlock->vert.vertexStride;
			meshData += verticesBytes;
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
			//numIndices = *(PUINT32)meshData;

			indxBlock = reinterpret_cast<const PlyIndxBlock*>(meshData);
			meshData += 4;

			indicesBytes = indxBlock->numIndices * sizeof(WORD);
			meshData += indicesBytes;
		}
		break;
		default:
			OutputDebugStringW(L"Invalid ply block.\n");
			return nullptr;
		}
	}


	PUINT32 pVertSize = new UINT32;
	if (!InitOnceExecuteOnce(&g_InitOnce, InitializeDecl, (LPVOID)&(vertBlock->vert.vertexStride), (LPVOID*)&pVertSize))
		throw std::exception("Vertex InputElement Layout initialization failed");
	assert(*pVertSize == vertBlock->vert.vertexStride);

	ComPtr<ID3D11Buffer> vb;
	// Create vertex buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = verticesBytes;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA initData = { vertBlock->data };
		DX::ThrowIfFailed(
			d3dDevice->CreateBuffer(&desc, &initData, vb.GetAddressOf())
		);

		DirectX::SetDebugObjectName(vb.Get(), "ModelPLY");
	}

	// Create index buffer
	ComPtr<ID3D11Buffer> ib;
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = indicesBytes;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		D3D11_SUBRESOURCE_DATA initData = { indxBlock->data };
		DX::ThrowIfFailed(
			d3dDevice->CreateBuffer(&desc, &initData, ib.GetAddressOf())
		);

		DirectX::SetDebugObjectName(ib.Get(), "ModelPLY");
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
			d3dDevice->CreateInputLayout(g_vbdecl->data(),
			(UINT)g_vbdecl->size(),
				shaderByteCode, byteCodeLength,
				il.GetAddressOf()));

		DirectX::SetDebugObjectName(il.Get(), "ModelPLY");
	}

	auto mesh = std::make_shared<ModelMesh>();
	mesh->ccw = ccw;
	mesh->pmalpha = pmalpha;
	BoundingBox::CreateFromPoints(mesh->boundingBox,
		XMVectorSet(position1.x, position1.y, position1.z, 1.0f),
		XMVectorSet(position2.x, position2.y, position2.z, 1.0f));
	BoundingSphere::CreateFromBoundingBox(mesh->boundingSphere, mesh->boundingBox);

	UINT partIndexCount = 0;
	for (PlyMeshBlockVector::iterator m = meshBlocks.begin(); m != meshBlocks.end(); m++) {
		MaterialRecordPLY materials;
		materials.name = (*m)->name;


		auto part = new ModelMeshPart();
		part->indexCount = (*m)->mesh->facesTriangles * 3;
		partIndexCount += part->indexCount;
		part->startIndex = (*m)->mesh->facesOffset * 3;
		part->vertexStride = vertBlock->vert.vertexStride;
		part->inputLayout = il;
		part->indexBuffer = ib;
		part->vertexBuffer = vb;
		part->effect = ieffect;
		part->vbDecl = g_vbdecl;

		mesh->meshParts.emplace_back(part);
	}
	assert(indxBlock->numIndices == partIndexCount);

	std::unique_ptr<Model> model(new Model());
	model->meshes.emplace_back(mesh);

	return model;
}

std::unique_ptr<DirectX::Model> __cdecl ModelPLY::CreateFromPLY(_In_ ID3D11Device* d3dDevice, _In_z_ const wchar_t* szFileName, _In_opt_ std::shared_ptr<IEffect> ieffect /*= nullptr*/, bool ccw /*= false*/, bool pmalpha /*= false*/)
{
	DX::SafeHandle hFile = CreateFileW(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		OutputDebugStringW(L"CreateFile failed.\n");
		return nullptr;
	}
	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize)) {
		OutputDebugStringW(L"GetFileSizeEx failed.\n");
		return nullptr;
	}
	auto dataPtr = std::make_unique<byte[]>(fileSize.LowPart);
	DWORD readedBytes;
	ReadFile(hFile, dataPtr.get(), fileSize.LowPart, &readedBytes, NULL);

	auto model = CreateFromPLY(d3dDevice, dataPtr.get(), readedBytes, ieffect, ccw, pmalpha);

	model->name = szFileName;
	return model;
}

std::unique_ptr<DirectX::Model> __cdecl ModelPLY::CreateFromPAK(_In_ ID3D11Device* d3dDevice, _In_z_ const wchar_t* szFileName, _In_z_ const char* entity, _In_opt_ std::shared_ptr<IEffect> ieffect /*= nullptr*/, bool ccw /*= false*/, bool pmalpha /*= false*/)
{
	enum ZIP_SIGNATURE {
		LocalFileHeader = 0x04034b50,
		DataDescriptor = 0x08074b50,
		CentralDirectoryFileHeader = 0x02014b50,
		EOCD = 0x06054b50
	};
#pragma pack(push, 2)
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

	DX::SafeHandle hFile = CreateFileW(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
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
					// find it, it's plain, not compressed.
					auto model = CreateFromPLY(d3dDevice, dataPtr.get(), fileBlock->header.comp_size,
						ieffect, ccw, pmalpha);
					if (model == nullptr)
						throw "Load model fail!";
					model->name.assign(entity, entity + strlen(entity));
					//return model;
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

			if (cdfHeader.extra_field_len) {
				SetFilePointer(hFile, cdfHeader.extra_field_len, 0, FILE_CURRENT);
			}

			if (cdfHeader.fcomment_len) {
				SetFilePointer(hFile, cdfHeader.fcomment_len, 0, FILE_CURRENT);
			}
			delete[] fileName; fileName = nullptr;
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
