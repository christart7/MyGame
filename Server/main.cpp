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
	int Id;			//游戏ID
	char name[10];  
	int playerNum;   //当前玩家数量
	Card card;  
};


static const int c_iPort = 10001;
unsigned int totalGameId = 0;//一共创建过的游戏数，用于gameID分配
vector<GameTable> gameList;

//查找指定游戏Id的游戏
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

//创建处理线程
DWORD WINAPI SocketProc(LPVOID ipParameter)
{
    SOCKET skAccept  = (SOCKET)ipParameter;
	int iRet;
	const int c_iBufLen = 512;
	char recvBuf[c_iBufLen + 1] = {'\0'};
	char sendBuf[c_iBufLen + 16 + 1] = {'\0'} ;
	char sendBuf2[128] = {'\0'} ;
	int MyGameId = 0;  //玩家所在游戏的唯一标示
	GameTable *joinGame = NULL;


	if(gameList.empty())//当前游戏列表为空
	{
		send(skAccept, "NULL", strlen("NULL"), 0);

		iRet = recv(skAccept, recvBuf, c_iBufLen, 0);//获取新游戏名

		//正常情况下不会走到下面
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
		iRet = recv(skAccept, recvBuf, c_iBufLen, 0);//获取用户的游戏

		//客户端有判断，正常情况下不会走到下面
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
			else //所选游戏玩家已满
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
		//接收客户端消息
		iRet = recv(skAccept, recvBuf, c_iBufLen, 0);// 接收客户端发送的信息， 如果客户端不发送信息，则线程会阻塞到此处
		if(iRet == 0)//客户端优雅的关闭了此连接
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
			joinGame->card.Shuffle(); //先洗牌
			joinGame->card.Deal(sendBuf);

			//向客户端发送消息
			iRet = send(skAccept, sendBuf, strlen(sendBuf), 0);	 // 客户端如果没有足够的缓冲区接受信息，则线程会阻塞到此处   
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

			iRet = send(skAccept, sendBuf2, strlen(sendBuf), 0);	 // 客户端如果没有足够的缓冲区接受信息，则线程会阻塞到此处   
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

	//关闭该套接字端口
	iRet = shutdown(skAccept, SD_SEND);
	while (recv(skAccept, recvBuf, c_iBufLen, 0) > 0);
	ASSERT(SOCKET_ERROR != iRet);

	//清理该套接字的资源
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

	//初始化Winsocket
	WSADATA data;
	ZeroMemory(&data, sizeof(WSADATA));
	iRet = WSAStartup(MAKEWORD(2, 0), &data);
	ASSERT(SOCKET_ERROR != iRet);

	//建立服务器端程序的监听套接字
	SOCKET skListen = INVALID_SOCKET;
	skListen = socket(AF_INET, SOCK_STREAM, 0);
	ASSERT(INVALID_SOCKET != skListen);

	 //初始化监听套接字地址信息，网络格式
	sockaddr_in adrServ;
	ZeroMemory(&adrServ, sizeof(sockaddr_in));
	adrServ.sin_family	= AF_INET;	 //初始化地址格式
	adrServ.sin_port	= htons(c_iPort);   //网络字节顺序和主机字节顺序相反，使用htons将主机字节顺序转换成网络字节顺序
	adrServ.sin_addr.s_addr	 = INADDR_ANY;	//任意IP

	//绑定监听套接字到本地
	iRet = bind(skListen, (sockaddr*)&adrServ, sizeof(sockaddr_in));
	ASSERT(SOCKET_ERROR != iRet);

	//使用套接字进行监听
	iRet = listen(skListen, SOMAXCONN);
	ASSERT(SOCKET_ERROR != iRet);

	cout<<"server is running..."<<endl;

	for (;;)
	{
		//客户端向服务器端发送连接请求，服务器端接受客户端的连接
		SOCKET skAccept = INVALID_SOCKET;
		sockaddr_in adrClit;
		ZeroMemory(&adrClit, sizeof(sockaddr_in));
		int iLen = sizeof(sockaddr_in);
		skAccept = accept(skListen, (sockaddr*)&adrClit, &iLen);// 如果没有客户端程序请求连接，服务端程序会一直阻塞在这里等待连接
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

	// 关闭该套接字的连接   
	iRet = shutdown(skListen, SD_SEND);
	ASSERT(SOCKET_ERROR != iRet);

	// 清理该套接字的资源   
	iRet = closesocket(skListen);   
	ASSERT(SOCKET_ERROR != iRet); 

	// 清理Winsocket资源,<span style="white-space: pre;">对Winsocket和ws2_32.dll的清理
	iRet = WSACleanup();   
	ASSERT(SOCKET_ERROR != iRet);

	system("pause");
	return 0;
}
