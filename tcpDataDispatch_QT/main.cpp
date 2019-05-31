#include <QtCore/QCoreApplication>
#include <QtCore/QDataStream>
#include <QtCore/QByteArray>
#include <QtGui/QImage>
#include <QtCore/QBuffer>
#include <windows.h>
#include <WinSock.h>
#include <stdio.h>
#include <time.h>
#include "FIFO.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 8000
#define HUD_IMAGE 102
#define SCENE_IMAGE 101
int g_LR = 0;
int g_type = 0;

FIFO *pFifo;

//ƽ�Ի���
HANDLE EVSShareMemory;
char *evsShareMem;

//�۾�д����
HANDLE GlassSMWrite;
char *glassSMWrite;

//�۾�������
HANDLE GlassSMRead;
char *glassSMRead;

int *i_status;
int *i_status2;
int *i_status3;

typedef struct myPara
{
	SOCKET Socket;
	SOCKADDR_IN Addr;
}MyPara;

typedef struct
{
	int ready_status; //�����Ƿ�׼���� 1.׼����
	int width; //ͼ����i
	int height;
	int pos_x;//ͼ������Xƫ��
	int pos_y;//ͼ������Yƫ��
	int fov; //�����ӳ�
	int vov;//�����ӳ�
	int nType;  //0ͼƬ1����2ͼƬ+����
}CaptureInfo;

struct glassPtureInfo
{
	int ready_status; //�����Ƿ�׼���� 1.׼����
	int width; //ͼ����i
	int height;
	int pos_x;//ͼ������Xƫ��
	int pos_y;//ͼ������Yƫ��
	int fov; //�����ӳ�
	int vov;//�����ӳ�
	int nType;  //0ͼƬ1����2ͼƬ+����
};

SOCKET network_init()
{
	//�����׽��ֿ�
	WSADATA wsaData;
	int iRet = 0;
	iRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRet != 0)
	{
		printf("WSAStartup(MAKEWORD(2, 2), &wsaData) execute failed!");
		return -1;
	}
	if (2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion))
	{
		WSACleanup();
		printf("WSADATA version is not correct!");
		return -1;
	}

	//�����׽���
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		printf("serverSocket = socket(AF_INET, SOCK_STREAM, 0) execute failed!\n");
		return -1;
	}

	//��ʼ����������ַ�����
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(PORT);

	//��
	iRet = bind(serverSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (iRet == SOCKET_ERROR)
	{
		printf("bind(serverSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) execute failed!");
		return -1;
	}

	//����
	iRet = listen(serverSocket, 5);
	if (iRet == SOCKET_ERROR)
	{
		printf("listen(serverSocket, 10) execute failed!");
		return -1;
	}

	return serverSocket;
}

void ShareBufferInit()
{
	//ƽ��
	EVSShareMemory = CreateFileMappingA((HANDLE)0xffffffffffffffff, NULL, PAGE_READWRITE, 0, 4 * 1280 * 1024 * 3 + 128, "IRImageInfo3");
	evsShareMem = (char *)MapViewOfFile(EVSShareMemory, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
	//�۾�д
	GlassSMWrite = CreateFileMappingA((HANDLE)0xffffffffffffffff, NULL, PAGE_READWRITE, 0, 4 * 1280 * 1024 * 3 + 128, "IRImageInfo1");
	glassSMWrite = (char *)MapViewOfFile(GlassSMWrite, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
	//�۾���
	GlassSMRead = CreateFileMappingA((HANDLE)0xffffffffffffffff, NULL, PAGE_READWRITE, 0, 4 * 1280 * 1024 * 3 + 128, "IRImageInfo2");
	glassSMRead = (char *)MapViewOfFile(GlassSMRead, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
}


void DecodeImage(QByteArray *pData)
{
	QImage image = QImage::fromData(*pData, "jpg");

	// ���������ͼƬ
	if (g_type == SCENE_IMAGE)
	{
		i_status2 = (int*)(&(glassSMWrite[0]));
		*i_status2 = 0;

		QBuffer buffer;
		buffer.open(QBuffer::ReadWrite);
		image.save(&buffer, "bmp");

		QDataStream out(&buffer);
		out << image;
		int size = buffer.size();
		const char *from = buffer.data().data();
		glassPtureInfo cap;
		if (*i_status2 == 0)
		{
			cap.ready_status = 1;
			cap.width = image.width();
			cap.height = image.height();
			cap.pos_x = 0;
			cap.pos_y = 0;
			cap.fov = 30 * 10;
			cap.vov = 22.5 * 10;
			cap.nType = g_LR;
			memcpy(glassSMWrite, &cap, sizeof(glassPtureInfo));
			memcpy(&(glassSMWrite[sizeof(glassPtureInfo)]), from, size);
		}
		
	}
	else if (g_type == HUD_IMAGE)
	{
		i_status = (int*)(&(evsShareMem[0]));
		*i_status = 0;

		// �����ݼ��ص������ڴ���
		QBuffer buffer;
		buffer.open(QBuffer::ReadWrite);
		image.save(&buffer, "bmp");
		QDataStream out(&buffer);
		out << image;
		int size = buffer.size();
		const char *from = buffer.data().data();
		CaptureInfo cap;
		if (*i_status == 0)
		{
			cap.ready_status = 1;
			cap.width = image.width();
			cap.height = image.height();
			cap.pos_x = 0;
			cap.pos_y = 0;
			cap.fov = 30 * 10;
			cap.vov = 22.5 * 10;
			cap.nType = g_LR;
			memcpy(evsShareMem, &cap, sizeof(CaptureInfo));
			memcpy(evsShareMem + sizeof(CaptureInfo), from, size);
			//*i_status = 1;
		}
	}
}

void ShowFrameRate()
{
	static unsigned int ImageCnt = 0;
	static time_t startTime = time(NULL);
	time_t lastTime = time(NULL);
	ImageCnt += 1;
	printf("ImageCount: %d\tFrameRate: %f\r", ImageCnt, (float)ImageCnt / (lastTime-startTime));
}

void ParsePacket(char *gBuffer, int sz)
{
	int left = 0;
	int curPoint = 0;
	int state = 0;
	qint64 length = 0;
	static int bound = 0;
	static unsigned char buf[5 * 1024 * 1024] = { 0 };

	memcpy(buf + bound, gBuffer, sz);
	bound += sz;
	left = bound;

	while (left)
	{
		switch (state)
		{
		case 0: // look for 0xaa, 0x55
			if (buf[curPoint] == 0xAA) {
				state = 1;
			}
			left -= 1;
			curPoint += 1;
			break;

		case 1:
			if (buf[curPoint] == 0x55){
				//printf("Got Head\n");
				state = 2;
			}else{
				state = 0;
			}
			left -= 1;
			curPoint += 1;
			break;

		case 2:
			if (buf[curPoint] == 0x7F){
				state = 3;
			}else{
				state = 0;
			}
			left -= 1;
			curPoint += 1;
			break;

		case 3:
			if (buf[curPoint] == 0){
				g_LR = 0;
				state = 4;
			}
			else if (buf[curPoint] == 1){
				g_LR = 1;
				state = 4;
			}
			else {
				printf("\nUnknown Type Case 3\n");
				state = 0;
			}

			left -= 1;
			curPoint += 1;
			break;

		case 4:
			if (buf[curPoint] == HUD_IMAGE){
				g_type = HUD_IMAGE;
				state = 5;
			}
			else if (buf[curPoint] == SCENE_IMAGE){
				g_type = SCENE_IMAGE;
				state = 5;
			}
			else {
				printf("\nUnknown Type Case 4\n");
				state = 0;
			}

			left -= 1;
			curPoint += 1;
			break;

		case 5: // look for length
			if (left >= 8) {
				QByteArray tmp((char*)(buf + curPoint), 8);
				QDataStream in(tmp);
				in >> length;
				state = 6;
				left -= 8;
				curPoint += 8;
				//printf("image length: %d\n", length);
			}
			else{
				left = 0;
			}
			break;

		case 6:
			if (left >= length) {
				QByteArray *pImage = new QByteArray();
				pImage->resize(length);
				memcpy(pImage->data(), buf + curPoint, length);
				left -= length;
				curPoint += length;

				pFifo->FIFO_In(&pImage, 1);

				memcpy(buf, buf + curPoint, left);
				state = 0;
				curPoint = 0;
				bound = left;
				//printf("Got a Image\n");
				ShowFrameRate();
			}
			else{// ʣ�೤�Ȳ���
				//printf("left length is not enough\n");
				left = 0;
			}
			break;
		}

	}
}

DWORD WINAPI ParseThread(const LPVOID param)
{
	MyPara *para = (MyPara*)param;
	SOCKET sock = para->Socket;
	char gBuffer[16 * 1024] = { 0 };
	
	printf("Connect success\n");
	while (1)
	{
		int result = recv(sock, gBuffer, 16 * 1024, 0);
		if (result != -1){
			ParsePacket(gBuffer, result);
		}
		else if (result == -1)
		{
			printf("\nNetwork: Client Disconnected\n");
			return -1;
		}
	}
}

DWORD WINAPI DecodeThread(const LPVOID param)
{
	QByteArray *pImage = NULL;

	while (1)
	{
		int cnt = pFifo->FIFO_Out(&pImage, 1);
		if (cnt != 0) {
			DecodeImage(pImage);
			delete pImage;
		}
	}

	return 0;
}

DWORD WINAPI ThreadMain(const LPVOID param)
{
	MyPara *para = (MyPara*)param;
	SOCKET listenSock = para->Socket;
	SOCKADDR_IN clientAddr;
	int len = sizeof(SOCKADDR);
	HANDLE hThread = CreateThread(NULL, 0, DecodeThread, para, 0, NULL);
	while (1)
	{
		printf("Waiting for connection...\n");
		SOCKET connSocket = accept(listenSock, (SOCKADDR*)&clientAddr, &len);
		if (connSocket == INVALID_SOCKET)
		{
			printf("accept(serverSocket, (SOCKADDR*)&clientAddr, &len) execute failed!");
			return -1;
		}
		
		MyPara *para = new MyPara;
		para->Socket = connSocket;
		para->Addr = clientAddr;

		HANDLE hThread = CreateThread(NULL, 0, ParseThread, para, 0, NULL);
	}
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	
	pFifo = new FIFO();
	pFifo->FIFO_Init(16, sizeof(QByteArray*));
	ShareBufferInit();
	
	SOCKET listenSock = network_init();
	if (listenSock == -1) {
		return 1;
	}

	SOCKADDR_IN clientAddr;
	int len = sizeof(SOCKADDR);
	MyPara *para = new MyPara;
	para->Socket = listenSock;

	HANDLE hThread = CreateThread(NULL, 0, ThreadMain, para, 0, NULL);

	return a.exec();
}
