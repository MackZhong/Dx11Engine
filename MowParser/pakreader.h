#pragma once


#pragma pack(push, 2)
__declspec(align(2)) struct _ZipLocalFileHeader
{
	unsigned short version;
	unsigned short bitflags;
	unsigned short comp_method;
	unsigned short lastModFileTime;
	unsigned short lastModFileDate;
	unsigned long crc_32;
	unsigned long comp_size;
	unsigned long uncompr_size;
	unsigned short fname_len;
	unsigned short extra_field_len;
};

__declspec(align(2)) struct _ZipCDFHeader
{
	unsigned short version_madeby;
	unsigned short version_needed;
	unsigned short bitflags;
	unsigned short comp_method;
	unsigned short lastModFileTime;
	unsigned short lastModFileDate;
	unsigned long crc_32;
	unsigned long comp_size;
	unsigned long uncompr_size;
	unsigned short fname_len;
	unsigned short extra_field_len;
	unsigned short fcomment_len;
	unsigned short disk_num_start;
	unsigned short internal_fattribute;
	unsigned long external_fattribute;
	unsigned long relative_offset;
};

__declspec(align(2)) struct _ZipEOCD	//Offset Bytes Description
{
	//0	4	End of central directory signature = 0x06054b50
	unsigned short numOfDisk;					//4	2	Number of this disk
	unsigned short diskCDStart;					//6	2	Disk where central directory starts
	unsigned short numOfCD;						//8	2	Number of central directory records on this disk
	unsigned short numTotalCD;					//10 2	Total number of central directory records
	unsigned long sizeCD;						//12 4	Size of central directory(bytes)
	unsigned long offsetCDStart;				//16 4	Offset of start of central directory, relative to start of archive
	unsigned short lenComment;					//20 2	Comment length(n)
										//22 n	Comment
};
#pragma pack(pop)

class PakFileReader
{
public:
	struct FilePosition {
		unsigned long offset;
		unsigned long size;
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
	PakFileReader(const wchar_t* szFile);

	int m_File;

	void AddFilePosition(std::string filename, unsigned long offset, unsigned long size) {
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
