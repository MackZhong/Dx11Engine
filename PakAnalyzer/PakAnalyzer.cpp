// PakAnalyzer.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>

#include "ostream.hpp"

using namespace std;
using namespace text::csv;
namespace csv = text::csv;


// Helper class for safely use HANDLE
class SafeHandle {
private:
	HANDLE m_Handle;
public:
	SafeHandle(HANDLE h) :m_Handle(h) {};
	~SafeHandle() { CloseHandle(m_Handle); m_Handle = NULL; }
	operator HANDLE() { return m_Handle; }
};


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

int main()
{
	wchar_t pakFile[] = L"E:/Games/Men of War Assault Squad/resource/entity/c1.pak";
	if (!PathFileExistsW(pakFile)) {
		std::wcerr << "The file \"" << pakFile << "\" not exists." << std::endl;
		return 1;
	}
	std::wstring resultFile(PathFindFileNameW(pakFile));
	resultFile += L".csv";

	//using  CsvLine = vector<string>;
	//using ParseResult = vector<CsvLine>;

	//ParseResult pr;

	//int g_columns = 10;

	//if (PathFileExistsW(resultFile.c_str())) {
	//	std::ifstream fs(resultFile);
	//	text::csv::csv_istream csvInput(fs);
	//}

	ofstream ofs(resultFile);
	csv_ostream ocsv(ofs);

	// header
	ocsv << "Name"	// 1
		<< "Offset"	// 2
		<< "Min"	// 3
		<< "Max"	// 4
		<< "Skins Count"	// 5
		<< "Skins Name"		// 6
		<< "Unknown 1"		// 7
		<< "Unknown 2"		// 8
		<< "Faces/Triangles"	// 9
		<< "Material Type"	// 10
		<< "Unknown 3"		// 11
		<< "Material"		// 12
		<< "Unknown 4(x,c,i)"	// 13
		<< "Skins Index"	// 14
		<< "Vertices Count"	// 15
		<< "Vertex Stride"	// 16
		<< "Unknown 5"		// 17
		<< "Vertex Bytes"	// 18
		<< "Indices Count"	// 19
		<< "Index Bytes"	// 20
		<< csv::endl;

	SafeHandle hFile = CreateFileW(pakFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		OutputDebugStringW(L"CreateFile failed.\n");
		wcerr << L"CreateFile failed.\n" << std::endl;
		return -1;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize)) {
		OutputDebugStringW(L"GetFileSizeEx failed.\n");
		wcerr << L"GetFileSizeEx failed." << std::endl;
		return -2;
	}

	struct _FileBlock {
		_ZipLocalFileHeader header;
		DWORD offset;
	};

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

			if (fileBlock->header.extra_field_len) {
				SetFilePointer(hFile, fileBlock->header.extra_field_len, 0, FILE_CURRENT);
			}
			fileBlock->offset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
			if (fileBlock->header.comp_size) {
				if (strncmp(".ply", PathFindExtensionA(fileName.get()), 4)) {
					SetFilePointer(hFile, fileBlock->header.comp_size, 0, FILE_CURRENT);
					break;
				}
				ocsv << fileName.get();	// 1,"Name"
				OutputDebugStringA(fileName.get());

				char buf[256] = { 0 };
				int nPos = 0;
				sprintf_s(buf, "0X%08X", fileBlock->offset);
				ocsv << buf;	// 2,"Offset"

				auto dataPtr = std::make_unique<byte[]>(fileBlock->header.comp_size);
				ReadFile(hFile, dataPtr.get(), fileBlock->header.comp_size, &readedBytes, NULL);
				const byte* meshData = dataPtr.get();

				// File Header
				if (*(PUINT64)meshData != *(PUINT64)"EPLYBNDS") {
					OutputDebugStringW(L"Invalid ply file.\n");
					cout << "\"" << fileName.get() << "\" is invalid ply file." << std::endl;
					SetFilePointer(hFile, fileBlock->header.comp_size, 0, FILE_CURRENT);
					break;
				}
				meshData += 8;

				float x = *(float*)meshData;
				meshData += 4;
				float y = *(float*)meshData;
				meshData += 4;
				float z = *(float*)meshData;
				meshData += 4;
				sprintf_s(buf, "%f,%f,%f", x, y, z);
				ocsv << buf;	// 3,"Min"

				x = *(float*)meshData;
				meshData += 4;
				y = *(float*)meshData;
				meshData += 4;
				z = *(float*)meshData;
				meshData += 4;
				sprintf_s(buf, "%f,%f,%f", x, y, z);
				ocsv << buf;	// 4,"Max"

				//std::vector<UINT16> m_SkinIndices;

				UINT32 numVertices = 0;
				UINT16 vertexStride = 0;
				UINT32 numIndices = 0;
				UINT32 u1;
				UINT32 skinsCount = 0;

				const UINT32 SKIN = 0x4e494b53; // "SKIN";
				const UINT32 MESH = 0x4853454d; // "MESH";
				const UINT32 VERT = 0x54524556; // "VERT";
				const UINT32 INDX = 0x58444e49; // "INDX";
				while ((size_t)(meshData - dataPtr.get()) < readedBytes - 4) {
					UINT32 magicK = *(PUINT32)meshData;
					meshData += 4;
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
						skinsCount = *(PUINT32)meshData;
						meshData += 4;
						ocsv << (long)skinsCount;	// 5,"Skins Count"
						OutputDebugStringW(L"Skins: ");
						nPos = 0;
						while (skinsCount--) {
							std::string skin((LPCSTR)meshData + 1, meshData[0]);
							meshData += 1 + meshData[0];
							nPos += sprintf_s(buf + nPos, 255 - nPos, "%s,", skin.c_str());
						}
						buf[nPos] = 0;
						ocsv << buf;	// 6, "Skins Name"
						OutputDebugStringA(buf);
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
						u1 = *(PUINT32)meshData;
						sprintf_s(buf, "0X%08X: 0x%04x, %i", (UINT32)(meshData - dataPtr.get()), u1, u1);
						meshData += 4;	// some unknown data
						ocsv << buf;	// 7, "Unknown 1"
						u1 = *(PUINT32)meshData;
						sprintf_s(buf, "0X%08X: 0x%04x, %i", (UINT32)(meshData - dataPtr.get()), u1, u1);
						meshData += 4;	// some unknown data
						ocsv << buf;	// 8, "Unknown 2"

						UINT32 m_Faces = *(PUINT32)meshData;
						meshData += 4;
						ocsv << (long)m_Faces;	// 9, "Faces/Triangles"

						UINT32 matType = *(PUINT32)meshData;
						meshData += 4;
						sprintf_s(buf, "0X%08X", matType);
						ocsv << buf;	// 10, "Material Type"

						// 0x0644, 0x0604, 0x0404, 0x0704, 0x0744, 0x0C14
						const UINT32 MAT_404 = 0x0404;
						const UINT32 MAT_604 = 0x0604;
						const UINT32 MAT_644 = 0x0644;
						const UINT32 MAT_704 = 0x0704;
						const UINT32 MAT_744 = 0x0744;
						const UINT32 MAT_C14 = 0x0C14;
						const UINT32 MAT_C54 = 0x0C54;
						const UINT32 MAT_F14 = 0x0F14;
						switch (matType) {
						case MAT_404:
						case MAT_C14:
						case MAT_C54:
							break;
						default:
							u1 = *(PUINT32)meshData;
							sprintf_s(buf, "0X%08X: 0x%04x, %i", (UINT32)(meshData - dataPtr.get()), u1, u1);
							meshData += 4;	// some unknown data
							ocsv << buf;	// 11, "Unknown 3"
							break;
						}
						std::string material((LPCSTR)meshData + 1, meshData[0]);
						meshData += 1 + meshData[0];
						ocsv << material;	// 12, "Material"

						//if (MAT_C14 == matType) {
						//	nPos = sprintf(buf, "0X%08X: %x,%x,%x ",
						//		meshData - dataPtr.get(), meshData[0], meshData[1], meshData[2]);
						//	nPos = sprintf_s(buf + nPos, 255 - nPos, "%c,%c,%c ",
						//		meshData[0], meshData[1], meshData[2]);
						//	nPos = sprintf_s(buf + nPos, 255 - nPos, "%i,%i,%i",
						//		meshData[0], meshData[1], meshData[2]);
						//	meshData += 3;
						//	ocsv << buf;	// 13, "Unknown 4(x,c,i)"
						//}
						UINT16 skins = *(PUINT16)meshData;
						meshData += 2;
						ocsv << skins;	// 13, "Skins+2"
						nPos = sprintf_s(buf, "Skin index: ");
						for (UINT32 p = 0; p < skinsCount; p++) {
							nPos += sprintf_s(buf + nPos, 255 - nPos, "%i", meshData[0]);
							meshData++;
						}
						buf[nPos] = 0;
						ocsv << buf;	// 14, "Skins Index"
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
						numVertices = *(PUINT32)meshData;
						meshData += 4;
						ocsv << (long)numVertices;	// 15, "Vertices Count"

						vertexStride = *(PUINT16)meshData;
						meshData += 2;
						ocsv << (long)vertexStride;	// 16, "Vertex Stride"

						u1 = *(PUINT16)meshData;
						sprintf_s(buf, "0X%08X: 0x%02x, %i", (UINT32)(meshData - dataPtr.get()), u1, u1);
						meshData += 2;	// some unknown data
						ocsv << buf;	// 17, "Unknown 5"

						ULONG verticesBytes = numVertices * vertexStride;
						sprintf_s(buf, "0x%08x, %i", verticesBytes, verticesBytes);
						ocsv << buf;	// 18, "Vertex Bytes"

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
						numIndices = *(PUINT32)meshData;
						meshData += 4;
						ocsv << (long)numIndices;	// 19, "Indices Count"

						ULONG indicesBytes = numIndices * sizeof(WORD);
						sprintf_s(buf, "0x%08x, %i", indicesBytes, indicesBytes);
						ocsv << buf;	// 20, "Index Bytes"

						meshData += indicesBytes;
					}
					break;
					default:
						sprintf_s(buf, "Invalid ply block in 0X%08X\n", (UINT32)(meshData - dataPtr.get()));
						OutputDebugStringA(buf);
					}
				}
				ocsv << csv::endl;
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

	return 0;
}

