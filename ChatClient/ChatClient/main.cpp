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
		// ���ڿ� �Է�
		std::cin.getline(buf, BUF_SIZE);

		// ���� ���ǿ� �ش�Ǵ��� �˻�
		if (!strcmp(buf, "q") || !strcmp(buf, "Q"))
		{
			IsFinish = true;
			break;
		}

		// ������ ������ ����
		sock->SendDataToServer(buf);

		// �ʱ�ȭ
		memset(buf, 0, sizeof(buf));
	}
}

void ReceiveData(SocketClass* sock)
{
	fd_set originRead = {}, cpyRead = {};
	TIMEVAL timeout = {};
	int SelectResult = 0;
	char buf[BUF_SIZE] = { 0, };

	//  �ʱ�ȭ
	FD_ZERO(&originRead);

	// client socket�� socket ���
	FD_SET(sock->GetSocket(), &originRead);

	// time-out ����
	timeout.tv_sec = 0;
	timeout.tv_usec = 10;

	while (!IsFinish)
	{
		cpyRead = originRead;

		// �̺�Ʈ�� �߻��ߴ��� Ȯ��
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
				// socket�� �̺�Ʈ�� �߻��ߴٸ�, �г��Ӱ� �޼��� ���
				if (FD_ISSET(cpyRead.fd_array[i], &cpyRead) && cpyRead.fd_array[i] == sock->GetSocket())
				{
					// �г��� �����ͼ� ���
					if (!sock->ReceiveDataFromServer(buf))
					{
						IsFinish = false;
						break;
					}
					std::cout << buf << ": ";
					memset(buf, 0, sizeof(buf));

					// �޼��� �����ͼ� ���
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