#include <string.h>
#include <iostream>
#include "SocketClass.h"

bool SocketClass::IsInitialize = false;

SocketClass::SocketClass()
{
	if (IsInitialize)
	{
		std::cout << "already exist socket class instance" << std::endl;
		exit(1);
	}

	bool res = init();
	if (!res)
	{
		std::cout << "init failed" << std::endl;
		release();
		exit(1);
	}

	IsInitialize = true;
}

SocketClass::~SocketClass()
{
	release();
	IsInitialize = false;
}

void SocketClass::run()
{
    int SelectResult = 0;
    fd_set cpy_reads = {};
    struct timeval timeout = {};
    ssize_t StringLength = 0;
    char str[BUF_SIZE] = { 0, };

    // file descriptor 배열 초기화
    FD_ZERO(&cpy_reads);

    // time-out 설정
    timeout.tv_sec = 1;         // seconds
    timeout.tv_usec = 0;        // micro-seconds

    while (1)
    {
        cpy_reads = m_OriginRegisteredFileDescriptors;

        // 등록된 file descriptor 중 변화가 있는 것이 있는 지 확인
        SelectResult = select(m_MaxFileDescriptorCount, &cpy_reads, 0, 0, &timeout);
        if (-1 == SelectResult)
        {
            std::cout << "select() error" << std::endl;
            break;
        }

        // time-out이 아닌 경우
        if (SelectResult > 0)
        {
            // 등록된 모든 file descriptor 확인
            for (int i = 3; i < m_MaxFileDescriptorCount; i++)
            {
                if (FD_ISSET(i, &cpy_reads))
                {
                    // server socket에 이벤트가 발생한 경우, 연결 수락
                    if (i == m_ServerSocket)
                        AcceptClient();
                    else
                    {
                        memset(str, 0, sizeof(str));

                        // client로부터 수신된 데이터가 있으면 가져오기
                        StringLength = read(i, str, BUF_SIZE);
                        if (-1 == StringLength)
                        {
                            std::cout << "read() error" << std::endl;
                            break;
                        }

                        // 수신된 데이터가 없는 경우, 연결 해제
                        if (0 == StringLength)
                            CloseClient(i);
                        // 수신한 데이터를 다른 client들에게 전송
                        else
                            SendMessage(i, str);
                    }
                }
            }
        }
    }
}

bool SocketClass::init()
{
	int FunctionResult = 0;
	socklen_t AddressSize = 0;
	struct sockaddr_in ServerAddress = {};

	// server socket 생성
	m_ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == m_ServerSocket)
	{
		std::cout << "socket() failed" << std::endl;
		return false;
	}

	// 서버 정보 입력
	AddressSize = sizeof(m_ServerSocket);
	memset(&ServerAddress, 0, AddressSize);
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerAddress.sin_port = htons(34567);

	// socket과 서버 정보를 bind
	FunctionResult = bind(m_ServerSocket, reinterpret_cast<struct sockaddr*>(&ServerAddress), sizeof(ServerAddress));
	if (-1 == FunctionResult)
	{
		std::cout << "bind() failed" << std::endl;
		return false;
	}

	// socket을 연결 상태로 전환
	FunctionResult = listen(m_ServerSocket, 5);
	if (-1 == FunctionResult)
	{
		std::cout << "listen() failed" << std::endl;
		return false;
	}

    // file descriptor 배열 초기화
    FD_ZERO(&m_OriginRegisteredFileDescriptors);

    // server socket을 file descriptor 배열에 등록
    FD_SET(m_ServerSocket, &m_OriginRegisteredFileDescriptors);

    // file descriptor의 최대 개수 설정
    m_MaxFileDescriptorCount = m_ServerSocket + 1;

	return true;
}

void SocketClass::release()
{
    for (int i = 0; i < CONNECT_ABLE; ++i)
    {
        if (m_ClientSockets[i])
            close(m_ClientSockets[i]);

        m_ClientNickName[i].reset();
    }

    if (m_ServerSocket)
        close(m_ServerSocket);
}

void SocketClass::AcceptClient()
{
    int CilentIdx = -1;
    int ClientSocket = 0;
    socklen_t AddressSize = sizeof(m_ClientAddress[0]);
    ssize_t StringLength = 0;
    char str[BUF_SIZE] = { 0, };

    // client가 연결할 수 있는지 확인
    for (int i = 0; i < CONNECT_ABLE; i++)
    {
        if (!m_ClientSockets[i])
        {
            CilentIdx = i;
            break;
        }
    }

    // 연결할 수 없다면, 무시
    if (-1 == CilentIdx)
    {
        std::cout << "Cannot connect this client" << std::endl;
        return;
    }

    // 연결 요청 수락
    ClientSocket = accept(m_ServerSocket, reinterpret_cast<struct sockaddr*>(&m_ClientAddress[CilentIdx]), &AddressSize);
    if (-1 == ClientSocket)
    {
        std::cout << "accept() error" << std::endl;
        return;
    }

    // client socket을 file descriptor 배열에 등록
    m_ClientSockets[CilentIdx] = ClientSocket;
    FD_SET(ClientSocket, &m_OriginRegisteredFileDescriptors);

    // file descriptor의 최대값 갱신
    if (ClientSocket >= m_MaxFileDescriptorCount)
        m_MaxFileDescriptorCount = ClientSocket + 1;

    // client로부터 닉네임 가져오기
    while (!StringLength)
    {
        StringLength = read(ClientSocket, str, BUF_SIZE);
        if (-1 == StringLength)
        {
            std::cout << "failed to get client nickname" << std::endl;
            std::cout << "read() error" << std::endl;
            return;
        }
    }

    // 가져온 닉네임 저장
    m_ClientNickName[CilentIdx] = std::make_unique<char[]>(strlen(str));
    strcpy(m_ClientNickName[CilentIdx].get(), str);

    std::cout << "connected client : " << ClientSocket << std::endl;
    std::cout << "connected client nickname : " << m_ClientNickName[CilentIdx].get() << std::endl;
}

void SocketClass::CloseClient(int ClientSocket)
{
    // 연결을 해제할 client 찾기
    for (int i = 0; i < CONNECT_ABLE; i++)
    {
        if (m_ClientSockets[i] == ClientSocket)
        {
            std::cout << "closed client nickname : " << m_ClientNickName[i].get() << std::endl;

            m_ClientSockets[i] = 0;

            memset(&m_ClientAddress[i], 0, sizeof(m_ClientAddress[i]));

            m_ClientNickName[i].reset();
            m_ClientNickName[i] = nullptr;

            break;
        }
    }

    // file descriptor 배열에서 client socket을 해제
    FD_CLR(ClientSocket, &m_OriginRegisteredFileDescriptors);

    // socket 반환
    close(ClientSocket);
}

void SocketClass::SendMessage(int sender, char* message) const
{
    ssize_t StringLength = 0;
    int SenderNicknameIdx = 0;

    // 보내는 사람의 닉네임 찾기
    for (int i = 0; i < CONNECT_ABLE; i++)
    {
        if (m_ClientSockets[i] == sender)
        {
            SenderNicknameIdx = i;
            break;
        }
    }

    // 보내는 사람을 제외한 모든 사람들에게 메세지 전송
    for (int i = 0; i < CONNECT_ABLE; i++)
    {
        if (m_ClientSockets[i] && m_ClientSockets[i] != sender)
        {
            StringLength = write(m_ClientSockets[i], m_ClientNickName[SenderNicknameIdx].get(), strlen(m_ClientNickName[SenderNicknameIdx].get()));
            if (-1 == StringLength)
            {
                std::cout << "failed send nickname to other client(file descriptor : " << m_ClientSockets[i] << std::endl;
                std::cout << "write() error" << std::endl;
            }

            StringLength = write(m_ClientSockets[i], message, strlen(message));
            if (-1 == StringLength)
            {
                std::cout << "failed send nickname to other client(file descriptor : " << m_ClientSockets[i] << std::endl;
                std::cout << "write() error" << std::endl;
            }
        }
    }
}
