#pragma once

#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>

#define BUF_SIZE 1024

class SocketClass
{
public:
	SocketClass();
	~SocketClass();

	bool ConnetServer();
	bool SendDataToServer(const char* str) const;
	bool ReceiveDataFromServer(char* str) const;

	SOCKET GetSocket() const { return m_ConnectSocket; }

private:
	bool init();
	void release();

private:
	static bool IsInitialize;

	SOCKET m_ConnectSocket = 0;
};