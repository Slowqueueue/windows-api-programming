// Пример простого UDP сервера для обработки файлов
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <winsock2.h>
#include <locale.h>
#include <windows.h>

// подключение библиотеки
#pragma comment(lib,"Ws2_32.lib")
#define PORT 666
#define BUF_SIZE 256
// глобальная переменная – количество активных пользователей 
int nclients = 0;
// переменные
CHAR string[80];
DWORD digitcnt = 0;
char* str = string;
CHAR Buffer[BUF_SIZE];
HANDLE hIn = { 0 };
HANDLE hOut = { 0 };
DWORD nIn, nOut = 0;
DWORD changeMAX;
char testfin[10] = { 0 };
char filename[80] = { 0 };
char name[80];
char* nm = name;
//bool
bool ischangeMAXexist;
// макрос для печати количества активных пользователей 
#define PRINTNUSERS if(nclients)\
  printf("%d user on-line\n",nclients);\
	    else printf("No User on line\n");
// функция по варианту
char* myfunc(char* filename, int changeMAX)
{
	char* outstring = new char[80];
	CloseHandle(hIn);
	CloseHandle(hOut);
	ZeroMemory(str, sizeof(str));
	ZeroMemory(testfin, sizeof(testfin));
	ZeroMemory(outstring, sizeof(outstring));
	int total = 0;
	strcpy(nm, "Output");
	strcat(nm, filename);
	hIn = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		sprintf(outstring, "Can not open file or file does not exist. Error: %x", GetLastError());
		return(outstring);
	}
	hOut = CreateFile(nm, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		sprintf(outstring, "Can not open out file. Error: %x", GetLastError());
		return(outstring);
	}
	while (ReadFile(hIn, Buffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {

		for (DWORD i = 0; i < nIn; i++)
		{
			if (int(Buffer[i]) < 48) {
				if (changeMAX > 0) {
					if (Buffer[i] != ' ') {
						Buffer[i] = ' ';
						total++;
						changeMAX--;
					}
				}
			}
		}
		WriteFile(hOut, Buffer, nIn, &nOut, NULL);
		if (nIn != nOut) {
			sprintf(outstring, "Fatal recording error: %x\n", GetLastError());
			printf("Fatal recording error: %x\n", GetLastError());
			return(outstring);
		}
	}
	printf("(Function): file:%s, changes = %d\n", filename, total);
	_itoa(total, outstring, 10);
	return outstring;
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");
	char buff[1014];
	printf("UDP DEMO Server\n");
	// Шаг 1 - подключение библиотеки
	if (WSAStartup(0x202, (WSADATA*)&buff[0]))
	{
		printf("Error WSAStartup : %d\n", WSAGetLastError());
		return -1;
	}
	// Шаг 2 - создание сокета
	SOCKET mysocket;
	if ((mysocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("Socket() error: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	// Шаг 3 - связывание сокета с локальным адресом
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(mysocket, (sockaddr*)&local_addr, sizeof(local_addr)))
	{
		printf("bind error: %d\n", WSAGetLastError());
		closesocket(mysocket);
		WSACleanup();
		return -1;
	}
	// Шаг 4 обработка пакетов, присланных клиентами
	char strfromfunc[80] = { 0 };
	char message[80] = { 0 };
	char* strff = strfromfunc;
	int len, lenstr, bytes_recv;
	while (1)
	{
		sockaddr_in client_addr;
		int client_addr_size = sizeof(client_addr);
		ischangeMAXexist = false;
		ZeroMemory(testfin, sizeof(testfin));
		ZeroMemory(message, sizeof(message));
		ZeroMemory(buff, sizeof(buff));
		ZeroMemory(str, sizeof(str));
		ZeroMemory(strff, sizeof(strff));
		ZeroMemory(strfromfunc, sizeof(strfromfunc));
		// получение имени файла и символа
		if (((bytes_recv = recvfrom(mysocket, buff, sizeof(buff) - 1, 0, (sockaddr*)&client_addr, &client_addr_size)) > 0)
			&& bytes_recv != SOCKET_ERROR) // принятие сообщения от клиента
			printf("Get %d bytes\n", bytes_recv);
		strcpy(str, buff);
		DWORD j = 0;
		digitcnt = 0;
		for (j = 0; j < strlen(str); j++) {
			if (str[j] == '/') {
				ischangeMAXexist = true;
				break;
			}
		}
		j = 0;
		if (ischangeMAXexist == true) {
			while (str[j] != '/') {
				testfin[j] = str[j];
				j++;
				digitcnt++;
			}
			j = 0;
			changeMAX = atoi(testfin);
			printf("changeMAX is:%d\n", changeMAX);
			if (testfin[0] != '0' and atoi(testfin) == 0) {
				printf("Wrong parameter. Parameter must be a number\n");
				sprintf(message, "Wrong parameter. Parameter must be a number.");
				len = sendto(mysocket, &message[0], strlen(message), 0,
					(sockaddr*)&client_addr, sizeof(client_addr));
				printf("Sent %d bytes\n", len);
				continue;
			}
		}
		else {
			changeMAX = 9999; //если максимальное число замен не задано, меняем все
			printf("changeMAX was not entered, the whole file will be processed\n");
		}

		if (ischangeMAXexist == true) {
			for (DWORD k = 0; k < strlen(str) - 1 - digitcnt; k++) {
				filename[k] = str[k + 1 + digitcnt];
				filename[bytes_recv - digitcnt - 2] = 0;
			}
		}
		else {
			strcpy(filename, str);
			filename[bytes_recv - digitcnt - 1] = 0;
		}
		//filename[bytes_recv - digitcnt - 2] = 0;
		digitcnt = 0;
		printf("Client => Server:%d/%s\n", changeMAX, filename);
		// Определяем IP-адрес клиента и прочие атрибуты
		//HOSTENT* hst;
		char host[20];
		char* hst = host;
		gethostname(hst, 20);
		//hst = gethostbyaddr((char*)&client_addr.sin_addr, 4, AF_INET);
		printf("+%s [%s:%d] new DATAGRAM!\n",
			//	(hst) ? hst->h_name : "Unknown host",
			hst,
			inet_ntoa(client_addr.sin_addr),
			ntohs(client_addr.sin_port));
		// увеличиваем счетчик подключившихся клиентов
		nclients++;
		// вывод сведений о клиенте	
		PRINTNUSERS
			// проверка на выход
			if (!strcmp(filename, "quit") || !strcmp(filename, "Quit") || !strcmp(filename, "QUIT"))
			{
				sprintf(buff, "Exiting...");
				buff[strlen(buff)] = '\n';// '\n'; // добавление к сообщению символа конца строки
				printf("Server:%s\n", buff);
				buff[strlen(buff) + 1] = '\r';
				// посылка датаграммы клиенту
				len = sendto(mysocket, &buff[0], strlen(buff), 0,
					(sockaddr*)&client_addr, sizeof(client_addr));
				printf("Sent %d bytes\n", len);
				exit(changeMAX);
			}
		// вызов пользовательской функции
		strff = myfunc(filename, changeMAX);
		if (atoi(strff) != 0) {
			ZeroMemory(buff, sizeof(buff));
			strcpy(buff, "Changes:");
			strcat(buff, strff);
		}
		else {
			ZeroMemory(buff, sizeof(buff));
			strcpy(buff, strff);
		}
		buff[strlen(buff)] = '\n';// '\n'; // добавление к сообщению символа конца строки
		printf("Server:%s", buff);
		buff[strlen(buff) + 1] = '\r';
		// посылка датаграммы клиенту
		len = sendto(mysocket, &buff[0], strlen(buff), 0,
			(sockaddr*)&client_addr, sizeof(client_addr));
		printf("Sent %d bytes\n", len);
		nclients--; // уменьшаем счетчик активных клиентов
		printf("-disconnect\n");
		PRINTNUSERS
	}//end while
}