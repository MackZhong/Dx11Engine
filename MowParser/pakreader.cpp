#include "stdafx.h"
#include "pakreader.h"

const unsigned int READ_BLOCK = 1024 * 1024;
PakFileReader::PakFileReaderMapType PakFileReader::m_ReaderMap;

extern void ShowError(const char* msg);

void ErrorMsg(int en) {
	std::stringstream stm;
	switch (errno) {
	case EACCES:
		stm << "Tried to open a read-only file for writing, file's sharing mode does not allow the specified operations, or the given path is a directory.";
		break;
	case EEXIST:
		stm << "_O_CREAT and _O_EXCL flags specified, but filename already exists.";
		break;
	case EINVAL:
		stm << "Invalid oflag or pmode argument.";
		break;
	case EMFILE:
		stm << "No more file descriptors are available (too many files are open).";
		break;
	case ENOENT:
		stm << "File or path not found.";
		break;
		//case:
		//	stm << "" ;
		//	break;
		//case:
		//	stm << "" ;
		//	break;
	default:
		stm << "Unknown errno " << en << ".";
		break;
	}

	ShowError(stm.str().c_str());
}

PakFileReader::PakFileReader(const wchar_t* szFile)
	:m_File(-1)
{
	errno_t err = ::_wsopen_s(&m_File, L"", _O_RDONLY, _SH_DENYNO, _S_IREAD);
	if (m_File < 0) {
		ErrorMsg(err);
		throw "Failed to open file.";
	}

	__int64 fileSize = _lseeki64(m_File, 0, SEEK_END);
	uint32_t readBlock = (uint32_t)__min(READ_BLOCK, fileSize);

	_lseeki64(m_File, 0 - readBlock, SEEK_END);

	auto buf = std::make_unique<char[]>(readBlock);
	int readedBytes = ::_read(m_File, buf.get(), readBlock);
	if (readedBytes < 0) {
		ErrorMsg(errno);
		throw "ReadFile failed.";
	}

	const _ZipEOCD* pEOCD{ nullptr };
	while (readedBytes--) {
		if (*(const unsigned int*)(buf.get() + readedBytes) == ZIP_SIGNATURE_END_OF_CENTRAL_DIRECTORY) {
			pEOCD = reinterpret_cast<const _ZipEOCD*>(buf.get() + readedBytes + 4);
			break;
		}
	}
	if (!pEOCD) {
		throw "Can't find EOCD";
	}

	long cdfSize = pEOCD->sizeCD;

	__int64 cdfOffset = _lseeki64(m_File, pEOCD->offsetCDStart, SEEK_SET);
	unsigned long lastLfOffset = (unsigned long)-1;

	std::string fileName;
	while (cdfSize > 0 && cdfOffset < fileSize) {
		//auto buf2 = std::make_unique<byte[]>(cdfSize);
		unsigned long signature;
		readedBytes = _read(m_File, &signature, 4);
		cdfSize -= readedBytes;
		if (signature != ZIP_SIGNATURE_CENTRAL_DIRECTORY_FILE_HEADER) {
			throw "Invalid Central Directory File Header position";
		}

		_ZipCDFHeader cdfHeader;
		readedBytes = _read(m_File, &cdfHeader, sizeof(cdfHeader));
		if (readedBytes < 0) {
			ErrorMsg(errno);
			throw "Failed to read Central Directory File Header";
		}
		cdfSize -= readedBytes;

		readedBytes = _read(m_File, buf.get(), cdfHeader.fname_len);
		cdfSize -= readedBytes;

		if (lastLfOffset != (unsigned long)-1 && !fileName.empty()) {
			AddFilePosition(fileName, lastLfOffset, cdfHeader.relative_offset - lastLfOffset);
		}

		lastLfOffset = cdfHeader.relative_offset;
		fileName.assign(buf.get(), buf.get() + cdfHeader.fname_len);

		cdfOffset = _lseeki64(m_File, cdfHeader.extra_field_len + cdfHeader.fcomment_len, SEEK_CUR);
		cdfSize -= cdfHeader.extra_field_len + cdfHeader.fcomment_len;
	}
	if (lastLfOffset != (unsigned long)-1 && !fileName.empty()) {
		AddFilePosition(fileName, lastLfOffset, pEOCD->offsetCDStart - lastLfOffset);
	}

	_close(m_File);
	m_File = -1;
}

std::shared_ptr<PakFileReader> PakFileReader::GetReader(const wchar_t* fileName)
{
	if (m_ReaderMap.find(fileName) == m_ReaderMap.end()) {
		m_ReaderMap[fileName] = std::shared_ptr<PakFileReader>(new PakFileReader(fileName));
	}

	return m_ReaderMap[fileName];
}

