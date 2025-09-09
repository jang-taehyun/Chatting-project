#ifndef SOCKET_CLASS
#define SOCKET_CLASS

#define BUF_SIZE 1024
#define CONNECT_ABLE 40

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#include <memory>

class SocketClass
{
public:
	SocketClass();
	~SocketClass();

	void run();

private:
	bool init();
	void release();

	void AcceptClient();
	void CloseClient(int ClientSocket);

	void SendMessage(int sender, char* message) const;

private:
	static bool IsInitialize;

	int m_ServerSocket = 0;

	int m_ClientSockets[CONNECT_ABLE] = { 0, };
	struct sockaddr_in m_ClientAddress[CONNECT_ABLE] = {};
	std::unique_ptr<char[]> m_ClientNickName[CONNECT_ABLE] = { nullptr, };

	fd_set m_OriginRegisteredFileDescriptors;

	int m_MaxFileDescriptorCount = 0;
};

#endif // !SOCKET_CLASS