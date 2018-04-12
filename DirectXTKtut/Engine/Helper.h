#pragma once
#include <handleapi.h>

class SafeHandle {
	HANDLE m_Handle;
public:
	SafeHandle(HANDLE h) :m_Handle(h) {};
	~SafeHandle() { CloseHandle(m_Handle); m_Handle = NULL; }
	operator HANDLE() { return m_Handle; }
};