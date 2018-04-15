#include "EnginePCH.h"
#include "PakFileReader.h"

const ULONG READ_BLOCK = 1024 * 1024;
PakFileReader::PakFileReaderMapType PakFileReader::m_ReaderMap;

PakFileReader::PakFileReader(LPCWSTR szFile)
	:m_File(INVALID_HANDLE_VALUE)
{
	if (!PathFileExistsW(szFile)) {
		throw "File not found";
	}

	DWORD dwErr;

	m_File = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == m_File) {
		dwErr = GetLastError();
		OutputDebugStringW(L"CreateFile failed.\n");
		throw "Failed to open file";
	}

	if (!GetFileSizeEx(m_File, &m_Size)) {
		dwErr = GetLastError();
		OutputDebugStringW(L"GetFileSizeEx failed.\n");
		throw "Failed to get file size";
	}

	ULONG readBlock = READ_BLOCK;
	if (readBlock > m_Size.LowPart && m_Size.HighPart == 0) {
		readBlock = m_Size.LowPart;
	}
	DWORD readedBytes;
	auto buf = std::make_unique<byte[]>(readBlock);
	SetFilePointer(m_File, 0 - READ_BLOCK, nullptr, FILE_END);
	if (!ReadFile(m_File, buf.get(), readBlock, &readedBytes, NULL)) {
		dwErr = GetLastError();
		OutputDebugStringW(L"ReadFile failed.\n");
		throw "Failed to read file";
	}
	const _ZipEOCD* pEOCD{ nullptr };
	while (readedBytes--) {
		if (*(PUINT32)(buf.get() + readedBytes) == ZIP_SIGNATURE_END_OF_CENTRAL_DIRECTORY) {
			pEOCD = reinterpret_cast<const _ZipEOCD*>(buf.get() + readedBytes + 4);
			break;
		}
	}
	if (!pEOCD) {
		throw "Can't find EOCD";
	}

	ULONG cdfOffset = pEOCD->offsetCDStart;
	LONG cdfSize = pEOCD->sizeCD;

	cdfOffset = SetFilePointer(m_File, cdfOffset, nullptr, FILE_BEGIN);
	ULONG lastLfOffset = (ULONG)-1;
	std::string fileName;
	while (cdfSize > 0 && cdfOffset < m_Size.LowPart) {
		//auto buf2 = std::make_unique<byte[]>(cdfSize);
		DWORD signature;
		ReadFile(m_File, &signature, 4, &readedBytes, NULL);
		cdfSize -= readedBytes;
		if (signature != ZIP_SIGNATURE_CENTRAL_DIRECTORY_FILE_HEADER) {
			throw "Invalid Central Directory File Header position";
		}
		_ZipCDFHeader cdfHeader;
		if (!ReadFile(m_File, &cdfHeader, sizeof(cdfHeader), &readedBytes, NULL)) {
			dwErr = GetLastError();
			OutputDebugStringW(L"Read Central Directory File Header failed.\n");
			throw "Failed to read Central Directory File Header";
		}
		cdfSize -= readedBytes;
		ReadFile(m_File, buf.get(), cdfHeader.fname_len, &readedBytes, NULL);
		cdfSize -= readedBytes;
		if (lastLfOffset != (ULONG)-1 && !fileName.empty()) {
			AddFilePosition(fileName, lastLfOffset, cdfHeader.relative_offset - lastLfOffset);
		}
		lastLfOffset = cdfHeader.relative_offset;
		fileName.assign(buf.get(), buf.get() + cdfHeader.fname_len);
		SetFilePointer(m_File, cdfHeader.extra_field_len + cdfHeader.fcomment_len, 0, FILE_CURRENT);
		cdfSize -= cdfHeader.extra_field_len + cdfHeader.fcomment_len;
		cdfOffset = SetFilePointer(m_File, 0, nullptr, FILE_CURRENT);
	}
	if (lastLfOffset != (ULONG)-1 && !fileName.empty()) {
		AddFilePosition(fileName, lastLfOffset, pEOCD->offsetCDStart - lastLfOffset);
	}

	CloseHandle(m_File);
	m_File = INVALID_HANDLE_VALUE;
}

std::shared_ptr<PakFileReader> PakFileReader::GetReader(const wchar_t* fileName)
{
	if (m_ReaderMap.find(fileName) == m_ReaderMap.end()) {
		m_ReaderMap[fileName] = std::shared_ptr<PakFileReader>(new PakFileReader(fileName));
	}

	return m_ReaderMap[fileName];
}

