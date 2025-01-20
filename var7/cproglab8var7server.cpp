#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <locale.h>
// подключение библиотеки
#pragma comment(lib,"Ws2_32.lib")
#define PORT 666
#define BUF_SIZE 256
// глобальная переменная – количество активных пользователей 
int nclients = 0;

// сокет
SOCKET mysocket;

// Буфер для передачи данных через канал
char buff[1014];

int main()
{
	setlocale(LC_ALL, "Russian");
	HANDLE hIn = { 0 }, hOut = { 0 };
	HINSTANCE hLib;
	DWORD nIn, nOut = 0;
	CHAR Buffer[BUF_SIZE];
	DWORD numFromCMD = 0;
	DWORD cbWritten = 0;

	char BufNumFromCMD[20];
	char clearBuffer[BUF_SIZE];
	int index = 0, newIndex = 0, space = 0, j = 0;

	// Буфер для передачи данных через канал

	DWORD total = 0;

	// буферы для  сообщения об ошибке, результата
	char message[260] = { 0 };
	char message2[260] = { 0 };
	printf("TCP server demo\n");

	if (WSAStartup(0x0202, (WSADATA*)&buff[0]))
	{
		// Ошибка!
		printf("Error WSAStartup %d\n",
			WSAGetLastError());
		return -1;
	}

	// Шаг 2 - создание сокета
	SOCKET mysocket;
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Socket() error: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	// Выводим сообщение о создании сокета
	printf("Socket created\n");
	// Шаг 3 - связывание сокета с локальным адресом
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(mysocket, (sockaddr*)&local_addr, sizeof(local_addr)))
	{
		printf("Bind error: %d\n", WSAGetLastError()); //error
		closesocket(mysocket);
		WSACleanup();
		return -1;
	}
	// Шаг 4 ожидание подключение и обработка пакетов, присланных клиентами
	int len, lenstr, bytes_recv;
	DWORD k = 0;
	while (1)
	{
		total = 0;
		ZeroMemory(buff, sizeof(buff));
		printf("(Server)Waiting for connect...\n");
		if (listen(mysocket, 0x100))
		{
			// Ошибка
			printf("Error listen %d\n", WSAGetLastError());
			closesocket(mysocket);
			WSACleanup();
			return -1;
		}
		printf("(Server)Listening...\n");
		//извлекаем сообщение из очереди
		SOCKET my_sock;
		sockaddr_in client_addr;
		int client_addr_size = sizeof(client_addr);
		my_sock = accept(mysocket, (sockaddr*)&client_addr, &client_addr_size);
		printf("(Server)Accepting data from queue...\n");
		// получение имени файла и символа
		if (((bytes_recv = recv(my_sock, buff, sizeof(buff) - 1, 0)) > 0) //buff - строка от клиента
			&& bytes_recv != SOCKET_ERROR) { // принятие сообщения от клиента
			printf("(Server)Get %d bytes\n", bytes_recv);
			// Определяем IP-адрес клиента и прочие атрибуты
			char host[20];
			char* hst = host;
			gethostname(hst, 20);
			sprintf(message, "+%s [%s:%d] new connect!\n",
				hst,
				inet_ntoa(client_addr.sin_addr),
				ntohs(client_addr.sin_port));
			// увеличиваем счетчик подключившихся клиентов
			nclients++;
			// вывод сведений о клиенте	
			if (nclients > 0) {
				printf("(Server)%d user on-line\n", nclients);
			}
			else {
				printf("(Server)No User on line\n");
			}
			index = 0, newIndex = 0, space = 0, j = 0;
			// Выводим принятую строку на консоль 
			printf("(Server)Received:%s\n", buff);

			// Если пришла команда "exit", завершаем работу приложения
			if (!strcmp(buff, "exit")) {
				printf("Client exited!\n");
				return(-1);
				// Иначе считаем что принято имя файла
			}
			else
			{
				char fileName[80] = { 0 };
				strcat(fileName, buff);

				while (fileName[j] != '\0') {
					if (fileName[index] != ' ') {
						index++;
					}
					if (fileName[index] == ' ') {
						space++;
					}
					j++;
				}

				ZeroMemory(BufNumFromCMD, sizeof(BufNumFromCMD));

				bool emptyNumver = false;
				if (space == 0) {
					emptyNumver = true;
				}
				else {
					index++;

					while (fileName[index] >= '0' && fileName[index] <= '9') {
						BufNumFromCMD[newIndex] = fileName[index];
						index++;
						newIndex++;
					}

				}

				if (emptyNumver == true) {
					numFromCMD = 9999;
				}
				else {
					numFromCMD = atoi(BufNumFromCMD);
				}

				if (numFromCMD == 0) {
					printf("(Server)Wrong parameter\n");
					sprintf(message2, "(Server)Wrong parameter");
					len = send(my_sock, message2, strlen(message2) + 1, 0);
					printf("(Server)Sent: <%s>\n", message2);
					printf("(Server)Sent %d bytes\n", len);
					nclients--;
					continue;
				}

				char newFileName[BUF_SIZE];
				char nameoffile[BUF_SIZE];
				index = 0;

				if (emptyNumver == false) {
					while (fileName[index] != ' ' || fileName[index] == '\0') {
						newFileName[index] = fileName[index];
						index++;
					}
					newFileName[index] = { NULL };
				}
				if (emptyNumver == true) {
					strcpy(newFileName, fileName);
				}


				bool key = false;
				hIn = CreateFile(newFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
				if (hIn == INVALID_HANDLE_VALUE) {
					printf("(Server)Can not open file or file does not exist. Error: %x\n", GetLastError());
					sprintf(message2, "(Server)Can not open file or file does not exist. Error: %x", GetLastError());
					len = send(my_sock, message2, strlen(message2) + 1, 0);

					printf("(Server)Sent: <%s>\n", message2);
					printf("(Server)Sent %d bytes\n", len);
					CloseHandle(hIn);
					nclients--;
					key = true;
					continue;
				}

				if (key == false) {
					memset(newFileName, 0, BUF_SIZE);

					index = 0;
					while (fileName[index] != '.') {
						newFileName[index] = fileName[index];
						index++;
					}
					strcpy(nameoffile, newFileName);
					strcat(nameoffile, ".txt");
					strcat(newFileName, "_Output.txt");

					hOut = CreateFile(newFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hOut == INVALID_HANDLE_VALUE) {
						printf("(Server)Can not open out file. Error: %x\n", GetLastError());
						return -1;
					}
					DWORD numFromCMDlegacy = numFromCMD;
					while (ReadFile(hIn, Buffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {
						for (DWORD i = 0; i < nIn; i++)
						{
							if (Buffer[i] == ' ') {
								if (numFromCMD > 0) {
									j = i;
									k = i;
									for (DWORD i = 0; i < nIn; i++) {
										Buffer[j] = Buffer[j + 1];
										j++;
									}
									i = k;
									total++;
									numFromCMD--;
									nIn--;
								}
							}
						}
						WriteFile(hOut, Buffer, nIn, &nOut, NULL);
						if (nIn != nOut) {
							printf("Fatal recording error: %x\n", GetLastError());
							return -1;
						}


						// сообщение в консоль ошибок
						printf("(Server): file:%s, Max changes:%d, changes:%d\n", nameoffile, numFromCMDlegacy, total);

						WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
						// сообщение в канал
						if (numFromCMDlegacy == 9999) {
							sprintf(clearBuffer, "No parameter.Max changes will be 9999. Changes:%d", total);
							len = send(my_sock, clearBuffer, strlen(clearBuffer) + 1, 0);
						}
						else {
							sprintf(clearBuffer, "%d", total);
							len = send(my_sock, clearBuffer, strlen(clearBuffer) + 1, 0);
						}
						printf("(Server)Sent: <%s>\n", clearBuffer);
						printf("(Server)Sent %d bytes\n", len);
						nclients--;
						//closesocket(mysocket);
						CloseHandle(hIn);
						CloseHandle(hOut);
					}
				}
			}
		}
		else {
			printf("(Server)Can not recieve message from socket. Error: %x\n", GetLastError());
		}
		Sleep(500);
	}
	return 0;
}