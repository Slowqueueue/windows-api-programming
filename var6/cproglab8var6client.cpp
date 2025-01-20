#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#pragma comment(lib,"Ws2_32.lib")
#define PORT 666
#define SERVERADDR "127.0.0.1"

DWORD main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");
	char buff[1014];
	char servaddr[80] = { 0 };
	char bytes[3] = { 0 };
	int n, len;
	printf("UDP DEMO Client\nType quit to quit\nEnter changeMAX/Filename.txt\n");
	// Шаг 1 - иницилизация библиотеки Winsocks
	if (WSAStartup(0x202, (WSADATA*)&buff[0]))
	{
		printf("WSAStartup error: %d\n", WSAGetLastError());
		return -1;
	}
	// Шаг 2 - открытие сокета
	SOCKET my_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (my_sock == INVALID_SOCKET)
	{
		printf("socket() error: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	// Шаг 3 - обмен сообщений с сервером
	HOSTENT* hst;
	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	if (argc > 1) strcpy(servaddr, argv[1]);
	else strcpy(servaddr, SERVERADDR);
	// определение IP-адреса узла
	if (inet_addr(servaddr) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(servaddr);
	else
	{
		if (hst = gethostbyname(servaddr))
			dest_addr.sin_addr.s_addr = ((unsigned long**)hst->h_addr_list)[0][0];
		else
		{
			printf("Unknown host: %d\n", WSAGetLastError());
			closesocket(my_sock);
			WSACleanup();
			return -1;
		}
	}
	printf("Send data to %s\n", servaddr);
	while (1)
	{
		// Выводим приглашение для ввода команды
		printf("cmd>");
		// чтение введенного сообщения в buff
		fgets(&buff[0], sizeof(buff) - 1, stdin);
		len = strlen(buff);
		if (strlen(buff) == 1) {
			printf("Error:Empty line\n");
			continue;
		}
		// проверка на quit
		if (!strcmp(&buff[0], "quit\n")) {
			n = sendto(my_sock, buff, len, 0, \
				(sockaddr*)&dest_addr, sizeof(dest_addr));
			break;
		}
		// Передача датаграмы - имени файла и символа
		n = sendto(my_sock, buff, len, 0, \
			(sockaddr*)&dest_addr, sizeof(dest_addr));
		printf("Sent %d bytes\n", n);
		// структура sockaddr_in и ее размер
		sockaddr_in server_addr;
		int dest_addr_size = sizeof(dest_addr);
		// Прием датаграмы
		ZeroMemory(buff, sizeof(buff));
		n = recvfrom(my_sock, &buff[0], sizeof(buff) - 1, 0, \
			(sockaddr*)&dest_addr, &(dest_addr_size));
		printf("Get %d bytes from server\n", n);
		if (n == SOCKET_ERROR)
		{
			printf("recvfrom() error: %d\n", WSAGetLastError());
			closesocket(my_sock);
			WSACleanup();
			return -1;
		}

		buff[n - 1] = 0;
		// Вывод принятого с сервера сообщения на экран
		printf("Server => Client:%s\n", &buff[0]);
	} // end while

	// Шаг последний - выход
	closesocket(my_sock);
	WSACleanup();
	return 0;
}