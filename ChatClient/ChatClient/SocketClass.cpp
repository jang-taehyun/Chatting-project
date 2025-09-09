#include <iostream>
#include <assert.h>
#include "info.h"
#include "SocketClass.h"

bool SocketClass::IsInitialize = false;

SocketClass::SocketClass()
{
	assert(!IsInitialize);

	bool res = init();
	assert(res);

	IsInitialize = true;
}

SocketClass::~SocketClass()
{
	release();
	IsInitialize = false;
}

bool SocketClass::init()
{
	int FunctionResult = 0;
	WSADATA wsaData = {};
	
	// winsock 라이브러리 초기화
	FunctionResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (FunctionResult)
	{
		std::cout << "WSAStartup() false" << std::endl;
		release();

		return false;
	}
	
	// TCP 소켓 생성
	m_ConnectSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_ConnectSocket)
	{
		std::cout << "socket() error" << std::endl;
		release();
		return false;
	}	

	return true;
}

void SocketClass::release()
{
	if (m_ConnectSocket)
	{
		closesocket(m_ConnectSocket);
		m_ConnectSocket = 0;
	}

	WSACleanup();
}

bool SocketClass::ConnetServer()
{
	int FunctionResult = 0;
	SOCKADDR_IN ServerAddress = {};
	char nickname[BUF_SIZE] = { 0, };
	int StringLength = 0;

	// 닉네임 설정
	std::cout << "닉네임을 입력해주세요 : ";
	std::cin.getline(nickname, BUF_SIZE);

	// 서버 정보(IP, Port) 입력
	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = inet_addr(IPAddress);
	ServerAddress.sin_port = htons(PortNumber);

	// 서버에 연결 요청
	FunctionResult = connect(m_ConnectSocket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress));
	if (SOCKET_ERROR == FunctionResult)
	{
		std::cout << "connect() error" << std::endl;
		release();
		return false;
	}

	// 서버에 닉네임 전송
	StringLength = send(m_ConnectSocket, nickname, static_cast<UINT>(strlen(nickname)), 0);
	if (SOCKET_ERROR == StringLength)
	{
		std::cout << "닉네임 전송 실패" << std::endl;
		std::cout << "send() error" << std::endl;
		return false;
	}

	return true;
}

bool SocketClass::SendDataToServer(const char* str) const
{
	int StringLength = 0;

	// 서버에 데이터 전송
	StringLength = send(m_ConnectSocket, str, static_cast<UINT>(strlen(str)), 0);
	if (SOCKET_ERROR == StringLength)
	{
		std::cout << "send() error" << std::endl;
		return false;
	}

	return true;
}

bool SocketClass::ReceiveDataFromServer(char* str) const
{
	int StringLength = 0;
	char buf[BUF_SIZE] = { 0, };

	// buffer로부터 수신한 데이터 가져오기
	StringLength = recv(m_ConnectSocket, str, BUF_SIZE, 0);
	if (SOCKET_ERROR == StringLength)
	{
		std::cout << "recv() error" << std::endl;
		return false;
	}

	return true;
}