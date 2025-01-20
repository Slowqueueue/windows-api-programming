#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <AclAPI.h>
#include <locale.h>
// подключение библиотеки
#pragma comment(lib,"Ws2_32.lib")
#define PORT 666
#define BUF_SIZE 256
// глобальная переменная – количество активных пользователей 
int nclients = 0;

//Функция в DLL
DWORD(*Bufferchange)(DWORD, CHAR*, DWORD);   // nIn buffer namecmd

extern int addLogMessage(const char* text);
int ServiceStart();
void ServiceStop();

// сокет
SOCKET mysocket;

// Буфер для передачи данных через канал
char buff[1014];

int Server()
{
	setlocale(LC_ALL, "Russian");
	HANDLE hIn, hOut;
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
	sprintf(message, "TCP server demo\n");
	addLogMessage(message);


	// Шаг 2 - создание сокета
	SOCKET mysocket;
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		sprintf(message, "Socket() error: %d\n", WSAGetLastError());
		addLogMessage(message);
		WSACleanup();
		return -1;
	}
	// Выводим сообщение о создании сокета
	sprintf(message, "Socket created\n");
	addLogMessage(message);

	// Шаг 3 - связывание сокета с локальным адресом
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(mysocket, (sockaddr*)&local_addr, sizeof(local_addr)))
	{
		sprintf(message, "Bind error: %d\n", WSAGetLastError());
		addLogMessage(message);
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
		ZeroMemory(Buffer, sizeof(Buffer));
		sprintf(message, "(Server)Waiting for connect...\n");
		addLogMessage(message);
		if (listen(mysocket, 0x100))
		{
			// Ошибка
			sprintf(message, "Error listen %d\n", WSAGetLastError());
			addLogMessage(message);
			closesocket(mysocket);
			WSACleanup();
			return -1;
		}
		sprintf(message, "(Server)Listening...\n");
		addLogMessage(message);
		//извлекаем сообщение из очереди
		SOCKET my_sock;
		sockaddr_in client_addr;
		int client_addr_size = sizeof(client_addr);
		my_sock = accept(mysocket, (sockaddr*)&client_addr, &client_addr_size);
		sprintf(message, "(Server)Accepting data from queue...\n");
		addLogMessage(message);
		// получение имени файла и символа
		if (((bytes_recv = recv(my_sock, buff, sizeof(buff) - 1, 0)) > 0) //buff - строка от клиента
			&& bytes_recv != SOCKET_ERROR) { // принятие сообщения от клиента
			sprintf(message, "(Server)Get %d bytes\n", bytes_recv);
			addLogMessage(message);
			// Определяем IP-адрес клиента и прочие атрибуты
			char host[20];
			char* hst = host;
			gethostname(hst, 20);
			sprintf(message, "+%s [%s:%d] new connect!\n",
				hst,
				inet_ntoa(client_addr.sin_addr),
				ntohs(client_addr.sin_port));
			addLogMessage(message);
			// увеличиваем счетчик подключившихся клиентов
			nclients++;
			// вывод сведений о клиенте	
			if (nclients > 0) {
				sprintf(message, "(Server)%d user on-line\n", nclients);
				addLogMessage(message);
			}
			else {
				sprintf(message, "(Server)No User on line\n");
				addLogMessage(message);
			}
			index = 0, newIndex = 0, space = 0, j = 0;
			// Выводим принятую строку на консоль 
			sprintf(message, "(Server)Received:%s", buff);
			addLogMessage(message);

			// Если пришла команда "exit", завершаем работу приложения
			if (!strcmp(buff, "exit")) {
				sprintf(message, "Client exited!\n");
				addLogMessage(message);
				return(-1);
				// Иначе считаем что принято имя файла
			}
			else
			{
				char fileName[128] = "C://Users/Nikita/source/repos/cprogRGZvar27/cprogRGZvar27service/x64/Debug/";
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


				hLib = LoadLibrary("C:\\Users\\Nikita\\source\\repos\\cprogRGZvar27\\DLL\\x64\\Debug\\DLL.dll");
				if (hLib == NULL) {
					sprintf(message, "(Server)Cannot load library\n", buff, total);
					addLogMessage(message);
					WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
					exit(-1);
				}
				Bufferchange = (DWORD(*)(DWORD, CHAR*, DWORD))GetProcAddress(hLib, "Bufferchange");
				if (Bufferchange == NULL) {
					sprintf(message, "(Server)Bufferchange function not found\n", buff, total);
					addLogMessage(message);
					WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
					return 0;
				}

				if (numFromCMD == 0) {
					sprintf(message2, "(Server)Wrong parameter");
					addLogMessage(message2);
					len = send(my_sock, message2, strlen(message2) + 1, 0);
					sprintf(message, "(Server)Sent: <%s>\n", message2);
					addLogMessage(message);
					sprintf(message, "(Server)Sent %d bytes\n", len);
					addLogMessage(message);
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
					sprintf(message2, "(Server)Can not open file or file does not exist. Error: %x", GetLastError());
					addLogMessage(message2);
					len = send(my_sock, message2, strlen(message2) + 1, 0);

					sprintf(message, "(Server)Sent: <%s>\n", message2);
					addLogMessage(message);
					sprintf(message, "(Server)Sent %d bytes\n", len);
					addLogMessage(message);
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
						sprintf(message, "(Server)Can not open out file. Error: %x\n", GetLastError());
						addLogMessage(message);
						return -1;
					}

					while (ReadFile(hIn, Buffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {
						total = Bufferchange(nIn, Buffer, numFromCMD);
						nIn = nIn - total;
						WriteFile(hOut, Buffer, nIn, &nOut, NULL);
						// сообщение в консоль ошибок
						sprintf(message, "(Server): file:%s, Max changes:%d, changes:%d\n", nameoffile, numFromCMD, total);
						addLogMessage(message);
						WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
						// сообщение в канал
						if (numFromCMD == 9999) {
							sprintf(clearBuffer, "No parameter.Max changes will be 9999. Changes:%d", total);
							len = send(my_sock, clearBuffer, strlen(clearBuffer) + 1, 0);
						}
						else {
							sprintf(clearBuffer, "%d", total);
							len = send(my_sock, clearBuffer, strlen(clearBuffer) + 1, 0);
						}
						sprintf(message, "(Server)Sent: <%s>\n", clearBuffer);
						addLogMessage(message);
						sprintf(message, "(Server)Sent %d bytes\n", len);
						addLogMessage(message);
						nclients--;
						//closesocket(mysocket);
						CloseHandle(hIn);
						CloseHandle(hOut);
					}
				}
			}
		}
		else {
			sprintf(message, "(Server)Can not recieve message from socket. Error: %x", GetLastError());
			addLogMessage(message);
		}
		Sleep(500);
	}
	return 0;
}

int ServiceStart()
{
	char message[80] = { 0 };
	DWORD res;
	// Подключаем библиотеку
	if (WSAStartup(0x202, (WSADATA*)&buff[0]))
	{
		// Если возникла ошибка, выводим ее код и зваершаем работу приложения
		sprintf(message, "Error WSAStartup : %d\n", WSAGetLastError());
		addLogMessage(message);
		return -1;
	}
	return 0;
}

void ServiceStop()
{
	closesocket(mysocket);
	WSACleanup();
	addLogMessage("Socket closed!");
}