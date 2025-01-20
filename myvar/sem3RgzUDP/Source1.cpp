#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <AclAPI.h>
#include <locale.h>
// ����������� ����������
#pragma comment(lib,"Ws2_32.lib")
#define PORT 666
#define BUF_SIZE 256
// ���������� ���������� � ���������� �������� ������������� 
int nclients = 0;

//������� � DLL
DWORD(*Bufferchange)(DWORD, CHAR*, DWORD);   // nIn buffer namecmd

extern int addLogMessage(const char* text);
int ServiceStart();
void ServiceStop();

// �����
SOCKET mysocket;

// ����� ��� �������� ������ ����� �����
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

	// ����� ��� �������� ������ ����� �����

	DWORD total = 0;

	// ����� ���  ��������� �� ������, ����������
	char message[160] = { 0 };

	sprintf(message, "UPD server demo\n");
	addLogMessage(message);

	// ��� 1 - ����������� ����������
	if (WSAStartup(0x202, (WSADATA*)&buff[0]))
	{
		sprintf(message, "Error WSAStartup : %d\n", WSAGetLastError());
		addLogMessage(message);
		return -1;
	}
	// ��� 2 - �������� ������
	SOCKET mysocket;
	if ((mysocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		sprintf(message, "Socket() error: %d\n", WSAGetLastError());
		addLogMessage(message);
		WSACleanup();
		return -1;
	}

	// ��� 3 - ���������� ������ � ��������� �������
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
	// ��� 4 ��������� �������, ���������� ���������
	int len, lenstr, bytes_recv;
	while (1)
	{
		ZeroMemory(buff, sizeof(buff));
		sprintf(message, "(Server)Waiting for datagram...\n");
		addLogMessage(message);

		sockaddr_in client_addr;
		int client_addr_size = sizeof(client_addr);
		// ��������� ����� ����� � �������
		if (((bytes_recv = recvfrom(mysocket, buff, sizeof(buff) - 1, 0, (sockaddr*)&client_addr, &client_addr_size)) > 0) //buff - ������ �� �������
			&& bytes_recv != SOCKET_ERROR) { // �������� ��������� �� �������
			sprintf(message, "(Server)Get %d bytes\n", bytes_recv);
			addLogMessage(message);
			// ���������� IP-����� ������� � ������ ��������
			char host[20];
			char* hst = host;
			gethostname(hst, 20);
			sprintf(message, "+%s [%s:%d] new DATAGRAM!\n",
				hst,
				inet_ntoa(client_addr.sin_addr),
				ntohs(client_addr.sin_port));
			addLogMessage(message);
			// ����������� ������� �������������� ��������
			nclients++;
			// ����� �������� � �������	
			if (nclients > 0) {
				sprintf(message, "(Server)%d user on-line\n", nclients);
				addLogMessage(message);
			}
			else {
				sprintf(message, "(Server)No User on line\n");
				addLogMessage(message);
			}
			index = 0, newIndex = 0, space = 0, j = 0;
			// ������� �������� ������ �� ������� 
			sprintf(message, "(Server)Received:%s", buff);
			addLogMessage(message);

			// ���� ������ ������� "exit", ��������� ������ ����������
			if (!strcmp(buff, "exit")) {
				sprintf(message, "Client exited!\n");
				addLogMessage(message);
				return(-1);
				// ����� ������� ��� ������� ��� �����
			}
			else
			{
				char fileName[80] = "C://Users/Nikita/source/repos/sem3RGZ/Service/x64/Debug/";
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


				hLib = LoadLibrary("C:\\Users\\Nikita\\source\\repos\\sem3RGZ\\DLL\\x64\\Debug\\DLL.dll");
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
					sprintf(message, "(Server)Wrong parameter");
					addLogMessage(message);
					len = sendto(mysocket, message, strlen(message) + 1, 0,
						(sockaddr*)&client_addr, sizeof(client_addr));
					sprintf(message, "(Server)Sent: <%s>\n", message);
					addLogMessage(message);
					sprintf(message, "(Server)Sent %d bytes\n", len);
					addLogMessage(message);
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
					sprintf(message, "(Server)Can not open file or file does not exist. Error: %x", GetLastError());
					addLogMessage(message);
					len = sendto(mysocket, message, strlen(message) + 1, 0,
						(sockaddr*)&client_addr, sizeof(client_addr));
					sprintf(message, "(Server)Sent: <%s>\n", message);
					addLogMessage(message);
					sprintf(message, "(Server)Sent %d bytes\n", len);
					addLogMessage(message);
					CloseHandle(hIn);
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
						WriteFile(hOut, Buffer, nIn, &nOut, NULL);

						// ��������� � ������� ������
						sprintf(message, "(Server): file:%s, Max changes:%d, changes:%d\n", nameoffile, numFromCMD, total);
						addLogMessage(message);
						WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
						// ��������� � �����
						if (numFromCMD == 9999) {
							sprintf(clearBuffer, "No parameter.Max changes will be 9999. Changes:%d", total);
							len = sendto(mysocket, clearBuffer, strlen(clearBuffer) + 1, 0,
								(sockaddr*)&client_addr, sizeof(client_addr));
						}
						else {
							sprintf(clearBuffer, "%d", total);
							len = sendto(mysocket, clearBuffer, strlen(clearBuffer) + 1, 0,
								(sockaddr*)&client_addr, sizeof(client_addr));
						}
						sprintf(message, "(Server)Sent: <%s>\n", clearBuffer);
						addLogMessage(message);
						sprintf(message, "(Server)Sent %d bytes\n", len);
						addLogMessage(message);
						nclients--;

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
	// ���������� ����������
	if (WSAStartup(0x202, (WSADATA*)&buff[0]))
	{
		// ���� �������� ������, ������� �� ��� � ��������� ������ ����������
		sprintf(message, "Error WSAStartup : %d\n", WSAGetLastError());
		addLogMessage(message);
		return -1;
	}
	// ������� �����
	SOCKET mysocket;
	if ((mysocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		// ���� �������� ������, ������� �� ��� � ��������� ������ ����������
		sprintf(message, "Socket() error: %d\n", WSAGetLastError());
		addLogMessage(message);
		WSACleanup();
		return -1;
	}

	// ������� ��������� � �������� ������
	sprintf(message, "Socket created\n");
	addLogMessage(message);
	return 0;
}

void ServiceStop()
{
	closesocket(mysocket);
	WSACleanup();
	addLogMessage("Socket closed!");
}