#include <iostream>
#include <cassert>
#include <WinSock2.h>
#include <vector>
#include "card.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define ASSERT assert

struct GameTable
{
	int Id;			//��ϷID
	char name[10];  
	int playerNum;   //��ǰ�������
	Card card;  
};


static const int c_iPort = 10001;
unsigned int totalGameId = 0;//һ������������Ϸ��������gameID����
vector<GameTable> gameList;

//����ָ����ϷId����Ϸ
GameTable* SearchGame(int gameId)
{
	for(int i=0;i<gameList.size();i++)
	{
		if(gameId == gameList[i].Id)
		{
			return &gameList[i];
		}
	}

	return NULL;
}

//���������߳�
DWORD WINAPI SocketProc(LPVOID ipParameter)
{
    SOCKET skAccept  = (SOCKET)ipParameter;
	int iRet;
	const int c_iBufLen = 512;
	char recvBuf[c_iBufLen + 1] = {'\0'};
	char sendBuf[c_iBufLen + 16 + 1] = {'\0'} ;
	char sendBuf2[128] = {'\0'} ;
	int MyGameId = 0;  //���������Ϸ��Ψһ��ʾ
	GameTable *joinGame = NULL;


	if(gameList.empty())//��ǰ��Ϸ�б�Ϊ��
	{
		send(skAccept, "NULL", strlen("NULL"), 0);

		iRet = recv(skAccept, recvBuf, c_iBufLen, 0);//��ȡ����Ϸ��

		//��������²����ߵ�����
		if(recvBuf[0]== '\0')
		{
			closesocket(skAccept);  
			cout<< "sycn error with client. close."<<endl;
			return 0;
		}
		cout<<endl<<"Create new game: "<<recvBuf<<endl;

		GameTable myGame;
		strcpy(myGame.name, recvBuf);
		myGame.playerNum = 1;
		myGame.Id = 0;
		gameList.push_back(myGame);
		MyGameId = myGame.Id;
		totalGameId++;
	}
	else //select or create
	{
		int num;
		sprintf(sendBuf, "%2X", gameList.size());
		send(skAccept, sendBuf, 2, 0);
		iRet = recv(skAccept, recvBuf, c_iBufLen, 0);//��ȡ�û�����Ϸ

		//�ͻ������жϣ���������²����ߵ�����
		if(recvBuf[0] == '\0')
		{
			closesocket(skAccept);
			cout<< "sycn error with client. close."<<endl;
			return 0;
		}

		if(strncmp(recvBuf,"create",6)==0) //create
		{
			cout<<endl<<"Create new game: "<<recvBuf+8<<endl;

			GameTable myGame;
			strcpy(myGame.name, recvBuf + 8);
			myGame.playerNum = 1;
			myGame.Id = totalGameId;
			gameList.push_back(myGame);
			totalGameId++;	
			MyGameId = myGame.Id;
		}
		else  //join
		{
			sscanf(recvBuf,"%D",&num);

			joinGame = SearchGame(gameList[num-1].Id);
			if(joinGame == NULL)
				return 0;

			if(joinGame->playerNum<4)
			{
				joinGame->playerNum += 1;
				MyGameId = joinGame->Id;
				cout<<joinGame->name<<": player join"<<", now player num "<<joinGame->playerNum<<endl;
			}
			else //��ѡ��Ϸ�������
			{
				GameTable myGame;
				strcpy(myGame.name, "default");
				myGame.playerNum = 1;
				myGame.Id = totalGameId;
				totalGameId++;
				MyGameId = myGame.Id;
				gameList.push_back(myGame);
				cout<<"create game default"<<endl;
			}
		}
	}

	for (;;)
	{
		//���տͻ�����Ϣ
		iRet = recv(skAccept, recvBuf, c_iBufLen, 0);// ���տͻ��˷��͵���Ϣ�� ����ͻ��˲�������Ϣ�����̻߳��������˴�
		if(iRet == 0)//�ͻ������ŵĹر��˴�����
		{
			cout<<endl <<	"Connection " << skAccept << " shutdown." << endl;

			joinGame = SearchGame(MyGameId);
			if(joinGame!=NULL)
			{
				joinGame->playerNum -= 1;
				cout<<joinGame->name<<": player quit, now player num "<<joinGame->playerNum<<endl;
				if(joinGame->playerNum == 0 )
				{	
					for(int i=0;i<gameList.size();i++)
					{
						if(joinGame->Id == gameList[i].Id)
						{
							cout<<"delete game "<<gameList[i].name<<endl;
							gameList.erase(gameList.begin()+i);						
							break;
						}
					}
				}
			}

			break;
		}
		else if(SOCKET_ERROR == iRet)
		{
			cout <<endl<< "Connection " << skAccept << " recv error." << endl;
			
			joinGame = SearchGame(MyGameId);
			if(joinGame!=NULL)
			{
				joinGame->playerNum -= 1;
				cout<<joinGame->name<<": player quit, now player num "<<joinGame->playerNum<<endl;
				if(joinGame->playerNum == 0 )
				{	
					for(int i=0;i<gameList.size();i++)
					{
						if(joinGame->Id == gameList[i].Id)
						{
							cout<<"delete game "<<gameList[i].name<<endl;
							gameList.erase(gameList.begin()+i);							
							break;
						}
					}
				}
			}
			break;
		}
		recvBuf[iRet] = '\0';

		if(0 == strcmp(recvBuf, "deal"))
		{
			joinGame = SearchGame(MyGameId);
			joinGame->card.Shuffle(); //��ϴ��
			joinGame->card.Deal(sendBuf);

			//��ͻ��˷�����Ϣ
			iRet = send(skAccept, sendBuf, strlen(sendBuf), 0);	 // �ͻ������û���㹻�Ļ�����������Ϣ�����̻߳��������˴�   
			if(SOCKET_ERROR == iRet)
			{
				cout <<endl<< "Connection " << skAccept << "send error." << endl;

				joinGame = SearchGame(MyGameId);
				if(joinGame!=NULL)
				{
					joinGame->playerNum -= 1;
					cout<<joinGame->name<<": player quit, now player num "<<joinGame->playerNum<<endl;
					if(joinGame->playerNum == 0 )
					{	
						for(int i=0;i<gameList.size();i++)
						{
							if(joinGame->Id == gameList[i].Id)
							{
								cout<<"delete game "<<gameList[i].name<<endl;
								gameList.erase(gameList.begin()+i);
								break;
							}
						}
					}
				}
				break;
			}

			joinGame = SearchGame(MyGameId);
			joinGame->card.ShowType(sendBuf2);

			iRet = send(skAccept, sendBuf2, strlen(sendBuf), 0);	 // �ͻ������û���㹻�Ļ�����������Ϣ�����̻߳��������˴�   
			if(SOCKET_ERROR == iRet)
			{
				cout <<endl<< "Connection " << skAccept << "send error." << endl;
				
				joinGame = SearchGame(MyGameId);
				if(joinGame!=NULL)
				{
					joinGame->playerNum -= 1;
					cout<<joinGame->name<<": player quit, now player num "<<joinGame->playerNum<<endl;
					if(joinGame->playerNum == 0 )
					{	
						for(int i=0;i<gameList.size();i++)
						{
							if(joinGame->Id == gameList[i].Id)
							{
								cout<<"delete game "<<gameList[i].name<<endl;
								gameList.erase(gameList.begin()+i);								
								break;
							}
						}
					}
				}
				break;
			}
		}
	}

	//�رո��׽��ֶ˿�
	iRet = shutdown(skAccept, SD_SEND);
	while (recv(skAccept, recvBuf, c_iBufLen, 0) > 0);
	ASSERT(SOCKET_ERROR != iRet);

	//������׽��ֵ���Դ
	iRet = closesocket(skAccept);
	ASSERT(SOCKET_ERROR != iRet);

    return 0;  
}



int main(int argc, char* argv[])
{
	int iRet = SOCKET_ERROR;
	HANDLE hThread =  NULL;
	const int c_iBufLen = 512;
	char recvBuf[c_iBufLen + 1] = {'\0'};

	//��ʼ��Winsocket
	WSADATA data;
	ZeroMemory(&data, sizeof(WSADATA));
	iRet = WSAStartup(MAKEWORD(2, 0), &data);
	ASSERT(SOCKET_ERROR != iRet);

	//�����������˳���ļ����׽���
	SOCKET skListen = INVALID_SOCKET;
	skListen = socket(AF_INET, SOCK_STREAM, 0);
	ASSERT(INVALID_SOCKET != skListen);

	 //��ʼ�������׽��ֵ�ַ��Ϣ�������ʽ
	sockaddr_in adrServ;
	ZeroMemory(&adrServ, sizeof(sockaddr_in));
	adrServ.sin_family	= AF_INET;	 //��ʼ����ַ��ʽ
	adrServ.sin_port	= htons(c_iPort);   //�����ֽ�˳��������ֽ�˳���෴��ʹ��htons�������ֽ�˳��ת���������ֽ�˳��
	adrServ.sin_addr.s_addr	 = INADDR_ANY;	//����IP

	//�󶨼����׽��ֵ�����
	iRet = bind(skListen, (sockaddr*)&adrServ, sizeof(sockaddr_in));
	ASSERT(SOCKET_ERROR != iRet);

	//ʹ���׽��ֽ��м���
	iRet = listen(skListen, SOMAXCONN);
	ASSERT(SOCKET_ERROR != iRet);

	cout<<"server is running..."<<endl;

	for (;;)
	{
		//�ͻ�����������˷����������󣬷������˽��ܿͻ��˵�����
		SOCKET skAccept = INVALID_SOCKET;
		sockaddr_in adrClit;
		ZeroMemory(&adrClit, sizeof(sockaddr_in));
		int iLen = sizeof(sockaddr_in);
		skAccept = accept(skListen, (sockaddr*)&adrClit, &iLen);// ���û�пͻ��˳����������ӣ�����˳����һֱ����������ȴ�����
		ASSERT(INVALID_SOCKET != skAccept);
		cout << "New connection " << skAccept << ", c_ip: " << inet_ntoa(adrClit.sin_addr) << ", c_port: " << ntohs(adrClit.sin_port) << endl;

		hThread  = CreateThread(NULL,0,SocketProc,(LPVOID)skAccept,0,NULL);
		if(hThread == NULL)
		{
			cout<<"creat thread failed"<<endl;
			break;
		}
		CloseHandle(hThread);
	}

	// �رո��׽��ֵ�����   
	iRet = shutdown(skListen, SD_SEND);
	ASSERT(SOCKET_ERROR != iRet);

	// ������׽��ֵ���Դ   
	iRet = closesocket(skListen);   
	ASSERT(SOCKET_ERROR != iRet); 

	// ����Winsocket��Դ,<span style="white-space: pre;">��Winsocket��ws2_32.dll������
	iRet = WSACleanup();   
	ASSERT(SOCKET_ERROR != iRet);

	system("pause");
	return 0;
}
