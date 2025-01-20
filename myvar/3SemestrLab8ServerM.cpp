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
DWORD chacha = 0;
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
// макрос для печати количества активных пользователей 
#define PRINTNUSERS if(nclients)\
  printf("%d user on-line\n",nclients);\
	    else printf("No User on line\n");
// функция по варианту
int myfunc(char* filename, int changeMAX)
{
	int total = 0;
	strcpy(nm, "Output");
	strcat(nm, filename);
	hIn = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		printf("Невозможно открыть входной файл. Ошибка: %x\n", GetLastError());
		return  -1;
	}
	hOut = CreateFile(nm, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		printf("Невозможно открыть выходной файл. Ошибка: %x\n", GetLastError());
		return -1;
	}
	while (ReadFile(hIn, Buffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {

		for (DWORD i = 0; i < nIn - 1; i++)
		{
			if (Buffer[i] == Buffer[i + 1]) {
				if (changeMAX > 0) {
					if (Buffer[i] != ' ') {
						Buffer[i] = ' ';
						Buffer[i + 1] = ' ';
						total++;
						changeMAX--;
					}
				}
			}
		}
		WriteFile(hOut, Buffer, nIn, &nOut, NULL);
		if (nIn != nOut) {
			printf("Неустранимая ошибка записи: %x\n", GetLastError());
			return -1;
		}
	}
	printf("(Function): file:%s, changes = %d\n", filename, total);
	return total;
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
	char bytes[3] = { 0 };
	int len, lenstr, bytes_recv;
	while (1)
	{
		sockaddr_in client_addr;
		int client_addr_size = sizeof(client_addr);
		// получение имени файла и символа

		if (((bytes_recv = recvfrom(mysocket, buff, sizeof(buff) - 1, 0, (sockaddr*)&client_addr, &client_addr_size)) > 0)
			&& bytes_recv != SOCKET_ERROR) // принятие сообщения от клиента
			printf("Get %d bytes\n", bytes_recv);
		strcpy(str, buff);
		DWORD j = 0;
		while (str[j] != '/') {
			testfin[j] = str[j];
			j++;
			chacha++;
		}
		changeMAX = atoi(testfin);
		ZeroMemory(testfin, sizeof(testfin));
		j = 0;
		for (DWORD k = 0; k < bytes_recv - 1 - chacha; k++) {
			filename[k] = str[k + 1 + chacha]; //filename - File1.txt
		}
		filename[bytes_recv - chacha - 2] = 0;
		chacha = 0;

		printf("C=>S:%s:%d\n", filename, changeMAX);	
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
		len = myfunc(filename, changeMAX);
		ZeroMemory(buff, sizeof(buff));
		_itoa(len, bytes, 10); // преобразование результата выполнения в строку
		// анализ результата
		if (len >= 0)
		{
			strcpy(buff, "Changes:");
			strcat(buff, bytes);
		}
		else strcat(buff, filename);
		buff[strlen(buff)] = '\n';// '\n'; // добавление к сообщению символа конца строки
		printf("Server:%s\n", buff);
		buff[strlen(buff) + 1] = '\r';
		// посылка датаграммы клиенту
		len = sendto(mysocket, &buff[0], strlen(buff), 0,
			(sockaddr*)&client_addr, sizeof(client_addr));
		printf("Sent %d bytes\n", len);
		ZeroMemory(buff, sizeof(buff));
		nclients--; // уменьшаем счетчик активных клиентов
		printf("-disconnect\n");
		PRINTNUSERS
	}//end while
}