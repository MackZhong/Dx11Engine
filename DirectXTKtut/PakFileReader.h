#pragma once
class PakFileReader
{
public:
	struct FilePosition {
		ULONG offset;
		ULONG size;
	};

	std::shared_ptr<FilePosition> FindFilePosition(std::string filename) {
		FilePositionMapType::iterator m = m_FileMap.find(filename);
		if (m == m_FileMap.end()) {
			return nullptr;
		}
		return std::make_shared<FilePosition>(m->second);
	}

	static std::shared_ptr<PakFileReader> GetReader(const wchar_t* fileName);
	using PakFileReaderMapType = std::map<std::wstring, std::shared_ptr<PakFileReader>>;
	static PakFileReaderMapType m_ReaderMap;
	using FilePositionMapType = std::map<std::string, FilePosition>;
	FilePositionMapType m_FileMap;

protected:
	PakFileReader(LPCWSTR szFile);

	DX::SafeHandle m_File;
	LARGE_INTEGER m_Size;

	void AddFilePosition(std::string filename, ULONG offset, ULONG size) {
		FilePositionMapType::iterator m = m_FileMap.find(filename);
		if (m == m_FileMap.end()) {
			m_FileMap[filename] = FilePosition{ offset, size };
		}
	}
};

enum ZIP_SIGNATURE {
	ZIP_SIGNATURE_LOCAL_FILE_HEADER = 0x04034b50,
	ZIP_SIGNATURE_DATA_DESCRIPTOR = 0x08074b50,
	ZIP_SIGNATURE_CENTRAL_DIRECTORY_FILE_HEADER = 0x02014b50,
	ZIP_SIGNATURE_END_OF_CENTRAL_DIRECTORY = 0x06054b50
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
