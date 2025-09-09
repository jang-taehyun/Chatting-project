#include <iostream>
#include <thread>
#include "SocketClass.h"

bool IsFinish = false;

void InputData(SocketClass* sock);
void ReceiveData(SocketClass* sock);

int main()
{
	std::unique_ptr<SocketClass> sock = std::make_unique<SocketClass>();
	
	if (sock->ConnetServer())
	{
		std::thread t1(InputData, sock.get());
		std::thread t2(ReceiveData, sock.get());

		t1.join();
		t2.join();
	}

	return 0;
}

void InputData(SocketClass* sock)
{
	char buf[BUF_SIZE] = { 0, };

	std::cout << "Insert message(q or Q to quit)" << std::endl;

	while (!IsFinish)
	{
		// 문자열 입력
		std::cin.getline(buf, BUF_SIZE);

		// 종료 조건에 해당되는지 검사
		if (!strcmp(buf, "q") || !strcmp(buf, "Q"))
		{
			IsFinish = true;
			break;
		}

		// 서버로 데이터 전송
		sock->SendDataToServer(buf);

		// 초기화
		memset(buf, 0, sizeof(buf));
	}
}

void ReceiveData(SocketClass* sock)
{
	fd_set originRead = {}, cpyRead = {};
	TIMEVAL timeout = {};
	int SelectResult = 0;
	char buf[BUF_SIZE] = { 0, };

	//  초기화
	FD_ZERO(&originRead);

	// client socket의 socket 등록
	FD_SET(sock->GetSocket(), &originRead);

	// time-out 설정
	timeout.tv_sec = 0;
	timeout.tv_usec = 10;

	while (!IsFinish)
	{
		cpyRead = originRead;

		// 이벤트가 발생했는지 확인
		SelectResult = select(0, &cpyRead, 0, 0, &timeout);
		if (SOCKET_ERROR == SelectResult)
		{
			std::cout << "select() error" << std::endl;
			IsFinish = true;
			break;
		}

		if (SelectResult > 0)
		{
			for (UINT i = 0; i < cpyRead.fd_count; ++i)
			{
				// socket에 이벤트가 발생했다면, 닉네임과 메세지 출력
				if (FD_ISSET(cpyRead.fd_array[i], &cpyRead) && cpyRead.fd_array[i] == sock->GetSocket())
				{
					// 닉네임 가져와서 출력
					if (!sock->ReceiveDataFromServer(buf))
					{
						IsFinish = false;
						break;
					}
					std::cout << buf << ": ";
					memset(buf, 0, sizeof(buf));

					// 메세지 가져와서 출력
					if (!sock->ReceiveDataFromServer(buf))
					{
						IsFinish = false;
						break;
					}
					std::cout << buf << std::endl;
					memset(buf, 0, sizeof(buf));
				}
			}
		}
	}
}