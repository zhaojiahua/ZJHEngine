// Copyright (C) RenZhai.2022.All Rights Reserved.
#include "simple_net_drive_tcp.h"
#include "../../../public/simple_c_log/simple_c_log.h"
#include "../simple_net_connetion/simple_connetion_tcp.h"
#include "../../../public/simple_channel/simple_protocols_definition.h"
#include "../../../public/simple_channel/simple_net_protocols.h"

HANDLE FSimpleTCPNetDrive::CompletionPortHandle = nullptr;

FSimpleTCPNetDrive::FSimpleTCPNetDrive(ESimpleDriveType InDriveType)
	:DriveType(InDriveType)
{
	MainConnetion = new FSimpleTCPConnetion();

	if (InDriveType == ESimpleDriveType::DRIVETYPE_LISTEN)
	{
		MainConnetion->SetConnetionState(ESimpleConnetionState::JOIN);
		MainConnetion->SetConnetionType(ESimpleConnetionType::CONNETION_MAIN_LISTEN);
	}
}

void HandShake(FSimpleConnetion* InLink)
{
	if (!InLink)
	{
		return;
	}

	FSimpleBunchHead Head = *(FSimpleBunchHead*)InLink->GetIOData().Buffer;
	if (Head.ParamNum == 0)
	{
		memset(InLink->GetIOData().Buffer, 0, 1024);
		InLink->GetIOData().Len = 0;
	}

	if (FSimpleChannel* Channel = InLink->GetMainChannel())
	{
		if (InLink->GetDriveType() == ESimpleDriveType::DRIVETYPE_LISTEN)
		{
			switch (Head.Protocols)
			{
				case SP_Hello:
				{
					std::string VersionRemote;
					SIMPLE_PROTOCOLS_RECEIVE(SP_Hello, VersionRemote);

					if (VersionRemote == "1.0.1")
					{
						SIMPLE_PROTOCOLS_SEND(SP_Challenge);

						log_log("Server:[Challenge] %s", InLink->GetAddrString().c_str());
					}

					break;
				}
				case SP_Login:
				{
					std::vector<int> Channels;
					SIMPLE_PROTOCOLS_RECEIVE(SP_Login, Channels)
					if (Channels.size() == 10)
					{
						InLink->SetConnetionState(ESimpleConnetionState::LOGIN);

						auto ChannelLists = InLink->GetChannels();
						
						int i = 0;
						for (auto &Tmp :*ChannelLists)
						{
							Tmp.SetGuid(Channels[i]);
							i++;
						}

						SIMPLE_PROTOCOLS_SEND(SP_Welcom);

						log_log("Server:[Welcom] %s", InLink->GetAddrString().c_str());
					}

					break;
				}
				case SP_Join:
				{
					InLink->SetConnetionState(ESimpleConnetionState::JOIN);
					InLink->ResetHeartBeat();

					log_success("Server:[Join] %s", InLink->GetAddrString().c_str());

					break;
				}
			}
		}
		else//�ͻ���
		{
			switch (Head.Protocols)
			{
				case SP_Challenge:
				{
					std::vector<int> Channels;
					InLink->GetChannelActiveID(Channels);
					SIMPLE_PROTOCOLS_SEND(SP_Login, Channels);
					InLink->SetConnetionState(ESimpleConnetionState::LOGIN);

					log_log("Client:[Login] :%s", InLink->GetAddrString().c_str());
				}
				case SP_Welcom:
				{
					InLink->SetConnetionState(ESimpleConnetionState::JOIN);

					SIMPLE_PROTOCOLS_SEND(SP_Join);

					//��������
					InLink->StartSendHeartBeat();

					log_log("Client:[Join] :%s", InLink->GetAddrString().c_str());
				}
			}
		}	
	}
}

unsigned int __stdcall Run(void* Content)
{
	for (;;)
	{
 		DWORD IOSize = -1;
		LPOVERLAPPED lpOverlapped = NULL;
		FSimpleConnetion* InLink = NULL;

		//win32 �� win64
		bool Ret = GetQueuedCompletionStatus(FSimpleTCPNetDrive::CompletionPortHandle, &IOSize, (PULONG_PTR)&InLink, &lpOverlapped, INFINITE);
		if (InLink == NULL && lpOverlapped == NULL)
		{
			break;
		}

		if (Ret)
		{
			if (IOSize == 0)
			{
				//ListRemove(Client);
				continue;
			}

			FSimpleIOData* pData = CONTAINING_RECORD(lpOverlapped,FSimpleIOData, Overlapped);
			switch (pData->Type)
			{
				case 0://���� �ͻ��˷��͵���Ϣ
				{
					InLink->GetIOData().Len = IOSize;
					InLink->GetIOData().Buffer[IOSize] = '\0';
					if (InLink->GetConnetionState() == ESimpleConnetionState::JOIN)
					{
						//ҵ���߼�
						InLink->Analysis();
					}
					else
					{
						//
						HandShake(InLink);
					}

					break;
				}
				case 1://����
				{	
					//printf(InLink->GetIOData().Buffer);
					//InLink->GetIOData().Len = 0;
					//if (!InLink->Recv())
					//{
					//	InLink->Send();
					//	//ListRemove(Client);
					//}

					//break;
				}
			}
		}
		else
		{
			DWORD Msg = GetLastError();
			if (Msg == WAIT_TIMEOUT)
			{
				continue;
			}
			else if (lpOverlapped != NULL)
			{
				//ListRemove(Client);
			}
			else
			{
				break;
			}
		}
	}

	return 0;
}

bool FSimpleTCPNetDrive::Init()
{
	if (DriveType == ESimpleDriveType::DRIVETYPE_LISTEN)
	{
		//��ɶ˿�
		if ((CompletionPortHandle = CreateIoCompletionPort(
			INVALID_HANDLE_VALUE,//�򿪵��ļ����
			NULL,//
			0,
			0)) == NULL)
		{
			//GetLastError();
			log_error("������ɶ˿�ʧ�� ~~ \n");
			return false;
		}

		//�����߳���Ŀ
		for (int i = 0; i < 2 * 2; i++)
		{
			hThreadHandle[i] = (HANDLE)_beginthreadex(
				NULL,// ��ȫ���ԣ� ΪNULLʱ��ʾĬ�ϰ�ȫ��
				0,// �̵߳Ķ�ջ��С�� һ��Ĭ��Ϊ0
				Run, // ��Ҫ������̺߳���
				CompletionPortHandle, // �̺߳����Ĳ����� ��һ��void*���ͣ� ���ݶ������ʱ�ýṹ��
				0,// ���̵߳ĳ�ʼ״̬��0��ʾ����ִ�У�CREATE_SUSPENDED��ʾ����֮�����
				NULL);   // ���������߳�ID
		}
	}

	int Ret = 0;
	if ((Ret = WSAStartup(MAKEWORD(2, 1), &WsaData)) != 0)
	{
		log_error("��ʼ����ʧ�� WSAStartup ~~ \n");
		return false;
	}

	//ִ��Connetion��ʼ��
	MainConnetion->Init();
	MainConnetion->SetDriveType(DriveType);

	if (DriveType == ESimpleDriveType::DRIVETYPE_LISTEN)
	{
		//����Socket
		MainConnetion->GetSocket() = INVALID_SOCKET;
		if ((MainConnetion->GetSocket() = WSASocket(
			AF_INET,//
			SOCK_STREAM, //TCP  SOCK_DGRAM
			0, //Э��
			NULL,//
			0,
			WSA_FLAG_OVERLAPPED//
		)) == INVALID_SOCKET)
		{
			WSACleanup();
			log_error("��������Socketʧ�� WSAStartup ~~ \n");
			return false;
		}

		//��������õ�ַ
		MainConnetion->GetConnetionAddr().sin_family = AF_INET;//IPV4������Э����
		MainConnetion->GetConnetionAddr().sin_addr.S_un.S_addr = htonl(INADDR_ANY);//0.0.0.0 ���Ե�ַ��
		MainConnetion->GetConnetionAddr().sin_port = htons(98592);

		if (bind(MainConnetion->GetSocket(), (SOCKADDR*)&MainConnetion->GetConnetionAddr(), sizeof(MainConnetion->GetConnetionAddr())) == SOCKET_ERROR)
		{
			closesocket(MainConnetion->GetSocket());
			WSACleanup();
			log_error("��������ʧ�� ~~ \n");
			return false;
		}

		if (listen(MainConnetion->GetSocket(), SOMAXCONN))
		{
			closesocket(MainConnetion->GetSocket());
			WSACleanup();
			log_error("�������ʧ�� ~~ \n");
			return false;
		}

		//��ʼ������ͨ��
		for (int i = 0 ;i < 2000;i++)
		{
			Connetions.insert(std::make_pair(i, new FSimpleTCPConnetion()));
			Connetions[i]->Init();
			Connetions[i]->SetDriveType(DriveType);
		}
	}
	else
	{
		MainConnetion->GetSocket() = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if (MainConnetion->GetSocket() == INVALID_SOCKET)
		{
			WSACleanup();
			log_error("��������Socketʧ�� WSAStartup ~~ \n");
			return false;
		}

		//�ͻ������õ�ַ
		MainConnetion->GetConnetionAddr().sin_family = AF_INET;//IPV4������Э����
		MainConnetion->GetConnetionAddr().sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//0.0.0.0 ���Ե�ַ��
		MainConnetion->GetConnetionAddr().sin_port = htons(98592);

		if (connect(
			MainConnetion->GetSocket(),
			(SOCKADDR*)&MainConnetion->GetConnetionAddr(),
			sizeof(MainConnetion->GetConnetionAddr())) == SOCKET_ERROR)
		{
			closesocket(MainConnetion->GetSocket());
			WSACleanup();
			log_error("�ͻ�������ʧ�� ~~ \n");
			return false;
		}

		//�������������֤
		if (FSimpleChannel* Channel = MainConnetion->GetMainChannel())
		{
			std::string v = "1.0.1";
			SIMPLE_PROTOCOLS_SEND(SP_Hello, v);

			log_log("Client send [Hello] to server[addr : %s]~~ \n", MainConnetion->GetAddrString().c_str());
		}
	}

	//���÷�����
	SetNonblocking();

	return true;
}

void FSimpleTCPNetDrive::Tick(double InTimeInterval)
{
	Super::Tick(InTimeInterval);

	if (DriveType == ESimpleDriveType::DRIVETYPE_LISTEN)
	{	
		//��ͨ��tick
		MainConnetion->Tick(InTimeInterval);

		//�����ܼ��
		for (auto &Tmp :Connetions)
		{
			if (Tmp.second->GetConnetionState() != ESimpleConnetionState::FREE)
			{
				Tmp.second->Recv();
			}
			else if (Tmp.second->GetConnetionState() != ESimpleConnetionState::JOIN)
			{
				Tmp.second->Tick(InTimeInterval);
			}
		}

		//iocp��Ͷ��
		SOCKET ClientAccept = INVALID_SOCKET;
		SOCKADDR_IN ClientAddr;
		int ClientAddrLen = sizeof(ClientAddr);

		if ((ClientAccept = WSAAccept(//��������
			MainConnetion->GetSocket(),
			(SOCKADDR*)&ClientAddr,
			&ClientAddrLen,
			NULL,
			0)) == SOCKET_ERROR)
		{
			//log_error("���ܿͻ���Ͷ��ʧ��");
			return;
		}

		//�����õ����õ�
		if (FSimpleConnetion* FreeConnetion = GetFreeConnetion())
		{
			//����ɶ˿�
			if (CreateIoCompletionPort(
				(HANDLE)ClientAccept,
				CompletionPortHandle,
				(DWORD)FreeConnetion, 0) == NULL)
			{
				log_error("�ͻ��˰󶨶˿�ʧ��");
				return;
			}

			//��Socket �� �����ַ
			FreeConnetion->GetSocket() = ClientAccept;
			FreeConnetion->GetConnetionAddr() = ClientAddr;

			if (!FreeConnetion->Recv())
			{
				//ListRemove(InClient);
				log_error("�ͻ��˽���ʧ��");
				return;
			}
			else
			{
				FreeConnetion->SetConnetionState(ESimpleConnetionState::VERSION_VERIFICATION);
			}
		}
	}
	else
	{
		MainConnetion->Recv();

		//������
		//log_success("�ͻ��˽��ܳɹ� %s", MainConnetion->GetIOData().Buffer);
		if (MainConnetion->GetConnetionState() != ESimpleConnetionState::JOIN)
		{
			HandShake(MainConnetion);
		}
		else
		{
			MainConnetion->Tick(InTimeInterval);

			//���ϲ�ҵ��Ľ���
			MainConnetion->Analysis();
		}
	}
}

void FSimpleTCPNetDrive::SetNonblocking()
{
	unsigned long UL = 1;
	int Ret = ioctlsocket(MainConnetion->GetSocket(), FIONBIO, &UL);
	if (Ret == SOCKET_ERROR)
	{
		log_error("Set Non-blocking ʧ��");
	}
}
