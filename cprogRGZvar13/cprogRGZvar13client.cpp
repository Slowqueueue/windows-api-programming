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
	// Буфер для передачи данных через сокет
	char buff[1014];
	char servaddr[80] = { 0 };
	int n, len;
	SOCKET my_sock;
	printf("TCP client demo\n");
	if (argc > 1) strcpy(servaddr, argv[1]);
	else strcpy(servaddr, SERVERADDR);
	printf("Send data to %s\n", servaddr);
	while (1)
	{
		// Шаг 1 - иницилизация библиотеки Winsocks
		if (WSAStartup(0x202, (WSADATA*)&buff[0]))
		{
			printf("WSAStartup error: %d\n", WSAGetLastError());
			return -1;
		}
		// Шаг 2 - открытие сокета
		my_sock = socket(AF_INET, SOCK_STREAM, 0);
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
		//	if (argc > 1) strcpy(servaddr, argv[1]);
		//	else strcpy(servaddr, SERVERADDR);
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
		// структура sockaddr_in и ее размер
		sockaddr_in server_addr;
		int dest_addr_size = sizeof(dest_addr);
		//Установка соединения с удаленным узлом
		if (connect(my_sock, (sockaddr*)&dest_addr, sizeof(dest_addr)))
		{
			printf("Connect error %d\n", WSAGetLastError());
			return -1;
		}

		// Выводим приглашение для ввода команды
		printf("cmd>");
		// Вводим текстовую строку
		gets_s(buff);
		len = strlen(buff);
		if (strlen(buff) == 0) {
			printf("Error:Empty line\n");
			closesocket(my_sock);
			WSACleanup();
			continue;
		}
		// Передача датаграмы - имени файла и символа
		n = send(my_sock, buff, len, 0);
		printf("Sent %d bytes\n", n);
		// проверка на exit
		if (!strcmp(buff, "exit")) {
			printf("exiting....");
			Sleep(1000);
			break;
		}
		// Прием датаграмы
		ZeroMemory(buff, sizeof(buff));
		n = recv(my_sock, &buff[0], sizeof(buff) - 1, 0);
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
		printf("Received: <%s>\n", buff);
	} // end while

	// Шаг последний - выход
	closesocket(my_sock);
	WSACleanup();
	return 0;
}