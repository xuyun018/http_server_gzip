// server.cpp : Defines the entry point for the console application.
//

#include <WinSock2.h>

#include <stdio.h>

#include "zlib131/zlib.h"
//---------------------------------------------------------------------------
#pragma comment(lib, "ws2_32.lib")
//---------------------------------------------------------------------------
#define MAX_LISTEN_NUM 5
#define RECV_BUF_SIZE 8192
#define LISTEN_PORT 2010
//---------------------------------------------------------------------------
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
//---------------------------------------------------------------------------
unsigned char *ReadFileBuffer(const WCHAR *filename, unsigned int *filesize)
{
	HANDLE hfile;
	DWORD numberofbytes;
	unsigned char *result = NULL;

	hfile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile != INVALID_HANDLE_VALUE)
	{
		*filesize = GetFileSize(hfile, NULL);

		result = (unsigned char *)MALLOC(*filesize);
		if (result != NULL)
		{
			ReadFile(hfile, result, *filesize, &numberofbytes, NULL);
		}

		CloseHandle(hfile);
	}
	return(result);
}
BOOL WriteFileBuffer(const TCHAR *filename, unsigned char *filebuffer, unsigned int filesize)
{
	HANDLE hfile;
	LARGE_INTEGER li;
	DWORD numberofbytes;
	BOOL result = FALSE;

	hfile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hfile != INVALID_HANDLE_VALUE)
	{
		//li.QuadPart = 0;
		//SetFilePointerEx(hfile, li, NULL, FILE_END);

		WriteFile(hfile, filebuffer, filesize, &numberofbytes, NULL);

		CloseHandle(hfile);
	}
	return(result);
}

DWORD WINAPI socket_thread_proc(LPVOID param)
{
	SOCKET fd = (SOCKET)param;
	char buf[8192];

	// 随便接收到什么数据
	recv(fd, buf, sizeof(buf), 0);

	unsigned char *dst;
	unsigned char *src;
	unsigned int len;
	unsigned int size;

	src = ReadFileBuffer(L"1.bmp", &size);
	if (src)
	{
		len = size + size;
		dst = (unsigned char *)MALLOC(len);
		if (dst)
		{
			gzFile outfile = gzopen();

			len = 0;
			gzwrite(outfile, dst, &len, src, size);
			gzclose(outfile, dst, &len);

			sprintf(buf, "HTTP/1.1 200 OK\r\n"
				"Connection: keep-alive\r\n"
				"Server: Microsoft-IIS/8.5\r\n"
				"Content-Type: image/bmp\r\n"
				"Content-Encoding: gzip\r\n"
				"Content-Length: %d\r\n"
				"\r\n", len);

			send(fd, buf, strlen(buf), 0);
			send(fd, (const char *)dst, len, 0);

			FREE(dst);
		}

		FREE(src);
	}

	closesocket(fd);

	return(0);
}

int wmain(int argc, WCHAR *argv[])
{
	SOCKET fd0 = 0;
	SOCKET fd = 0;
	struct sockaddr_in hostaddr;
	struct sockaddr_in clientaddr;
	int socklen = sizeof(clientaddr);
	char recvbuf[RECV_BUF_SIZE];
	int retlen;
	int port = 9000;

	if (argc > 1)
	{
		port = _wtoi(argv[1]);
	}

	WSADATA wsad;

	WSAStartup(MAKEWORD(2, 2), &wsad);

	memset((void *)&hostaddr, 0, sizeof(hostaddr));
	memset((void *)&clientaddr, 0, sizeof(clientaddr));

	hostaddr.sin_family = AF_INET;
	hostaddr.sin_port = htons(port);
	hostaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	fd0 = socket(AF_INET, SOCK_STREAM, 0);
	if (fd0 != INVALID_SOCKET)
	{
		if (bind(fd0, (struct sockaddr *)&hostaddr, sizeof(hostaddr)) == 0)
		{
			if (listen(fd0, MAX_LISTEN_NUM) == 0)
			{
				printf("Listening %d\r\n", port);

				while (1)
				{
					fd = accept(fd0, (struct sockaddr *)&clientaddr, &socklen);
					//
					if (fd != INVALID_SOCKET)
					{
						printf("accept %lld\r\n", fd);

						CreateThread(NULL, 0, socket_thread_proc, (LPVOID)fd, 0, NULL);
					}
				}
			}
		}

		closesocket(fd0);
	}

	WSACleanup();

	return 0;
}

