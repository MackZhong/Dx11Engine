#include "EnginePCH.h"
#include "MaterialPly.h"
#include "PakFileReader.h"
#include "SdlParser.h"


MaterialPlySimple::MaterialPlySimple()
{
}


MaterialPlySimple::~MaterialPlySimple()
{
}


std::shared_ptr<MaterialPlySimple> MaterialPlySimple::CreateFromPak(ID3D11Device* d3dDevice, LPCWSTR szPakFile, LPCSTR szMatFile)
{
	auto fileReader = PakFileReader::GetReader(szPakFile);
	auto pos = fileReader->FindFilePosition(szMatFile);
	if (!pos)
		return nullptr;

	DWORD dwErr;
	DX::SafeHandle hFile = CreateFileW(szPakFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		dwErr = GetLastError();
		OutputDebugStringW(L"CreateFile failed.\n");
		throw "Open file error";
		//return nullptr;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize) || fileSize.LowPart < (pos->offset + pos->size)) {
		dwErr = GetLastError();
		OutputDebugStringW(L"GetFileSizeEx failed or file size error.\n");
		throw "GetFileSizeEx failed or file size error";
		//return nullptr;
	}

	DWORD readedBytes;
	for (auto m = fileReader->m_FileMap.begin(); m != fileReader->m_FileMap.end(); m++) {
		if (StrCmpNA(".mtl", PathFindExtensionA((*m).first.c_str()), 4))
			continue;

		OutputDebugStringA((*m).first.c_str());
		OutputDebugStringA("\n");

		auto pos = fileReader->FindFilePosition((*m).first);
		if (!pos)
			break;;

		SetFilePointer(hFile, pos->offset, nullptr, FILE_BEGIN);

		auto dataPtr = std::make_unique<byte[]>(pos->size);
		if (!ReadFile(hFile, dataPtr.get(), pos->size, &readedBytes, NULL)) {
			dwErr = GetLastError();
			OutputDebugStringW(L"ReadFile for Local file failed.\n");
			throw "ReadFile for Local file failed";
		}
		if (*(PUINT32)dataPtr.get() != ZIP_SIGNATURE_LOCAL_FILE_HEADER) {
			OutputDebugStringW(L"Invalid Local File Header position.\n");
			throw "Invalid Local File Header position";
		}
		const _ZipLocalFileHeader* pLF = reinterpret_cast<_ZipLocalFileHeader*>(dataPtr.get() + 4);

		ULONG sizeHeader = 4 + sizeof(_ZipLocalFileHeader) + pLF->fname_len + pLF->extra_field_len;
		readedBytes -= sizeHeader;
		std::shared_ptr<MaterialPlySimple> material = CreateFromMTL(d3dDevice, dataPtr.get() + sizeHeader, readedBytes);
		material->name = szMatFile;
	}




	//DWORD dwErr;
	//DX::SafeHandle hFile = CreateFileW(szPakFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		dwErr = GetLastError();
		OutputDebugStringW(L"CreateFile failed.\n");
		throw "Open file error";
		//return nullptr;
	}

	//LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize) || fileSize.LowPart < (pos->offset + pos->size)) {
		dwErr = GetLastError();
		OutputDebugStringW(L"GetFileSizeEx failed or file size error.\n");
		throw "GetFileSizeEx failed or file size error";
		//return nullptr;
	}

	//DWORD readedBytes;
	SetFilePointer(hFile, pos->offset, nullptr, FILE_BEGIN);

	auto dataPtr = std::make_unique<byte[]>(pos->size);
	if (!ReadFile(hFile, dataPtr.get(), pos->size, &readedBytes, NULL)) {
		dwErr = GetLastError();
		OutputDebugStringW(L"ReadFile for Local file failed.\n");
		throw "ReadFile for Local file failed";
	}
	if (*(PUINT32)dataPtr.get() != ZIP_SIGNATURE_LOCAL_FILE_HEADER) {
		OutputDebugStringW(L"Invalid Local File Header position.\n");
		throw "Invalid Local File Header position";
	}
	const _ZipLocalFileHeader* pLF = reinterpret_cast<_ZipLocalFileHeader*>(dataPtr.get() + 4);

	ULONG sizeHeader = 4 + sizeof(_ZipLocalFileHeader) + pLF->fname_len + pLF->extra_field_len;
	readedBytes -= sizeHeader;
	std::shared_ptr<MaterialPlySimple> material = CreateFromMTL(d3dDevice, dataPtr.get() + sizeHeader, readedBytes);
	material->name = szMatFile;

	return material;
}

std::shared_ptr<MaterialPlySimple> MaterialPlySimple::CreateFromMTL(ID3D11Device* d3dDevice, const uint8_t* meshData, size_t dataSize)
{
	SdlParser parser;
	parser.Analyse(meshData, dataSize);

	return std::make_shared<MaterialPlySimple>();
}

MaterialPlyBump::MaterialPlyBump()
{
}


MaterialPlyBump::~MaterialPlyBump()
{
}
