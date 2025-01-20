#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 256

int main(int argc, LPTSTR argv[]) {  //exe
	setlocale(LC_ALL, "Rus");

	// Код возврата из функций
	BOOL   fReturnCode;
	// Размер сообщения в байтах
	DWORD  cbMessages;
	// Количество сообщений в канале Mailslot
	DWORD  cbMsgNumber;
	// Идентификатор канала Mailslot
	HANDLE hMailslot1 = { 0 };
	HANDLE hMailslot2 = { 0 };
	// Имя серверного канала Mailslot
	LPCTSTR  lpszReadMailslotName = "\\\\.\\mailslot\\$Channel1$";
	// Имя клиентского канала Mailslot
	LPCTSTR  lpszWriteMailslotName = "\\\\.\\mailslot\\$Channel2$";
	// Буфер для передачи данных через канал
	char szBuf[512];
	// всякие чары для работы с файлом
	char* szB = szBuf;
	char testfin[10] = { 0 };
	char filename[80] = { 0 };
	char name[80];
	char* nm = name;
	CHAR Buffer[BUF_SIZE];
	CHAR Buffer2[BUF_SIZE] = { 0 };
	// Количество байт данных, принятых через канал
	DWORD cbRead;
	// Количество байт данных, переданных через канал
	DWORD  cbWritten;
	DWORD  total = 0;
	// буферы для  сообщения об ошибке, результата
	char message[80] = { 0 };
	char message2[80] = { 0 };
	// хендлы для работы с файлом
	HANDLE hIn = { 0 };
	HANDLE hOut = { 0 };
	// всякие дворды для работы с файлом
	DWORD nIn, nOut = 0;
	DWORD i = 0;
	DWORD j = 0;
	DWORD k = 0;
	DWORD digitcnt = 0;
	DWORD changeMAX = 0;
	//bool
	bool ischangeMAXexist;
	printf("Mailslot server demo\n");
	// Создаем канал Mailslot, имеющий имя lpszReadMailslotName
	hMailslot1 = CreateMailslot(lpszReadMailslotName, 0, MAILSLOT_WAIT_FOREVER, NULL);
	// Если возникла ошибка, выводим ее код и зваершаем работу приложения
	if (hMailslot1 == INVALID_HANDLE_VALUE)
	{
		fprintf(stdout, "CreateMailslot: Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}
	// Выводим сообщение о создании канала
	fprintf(stdout, "Mailslot created\n");
	// Ожидаем соединения со стороны клиента
	// Цикл получения команд через канал
	while (1)
	{
		CloseHandle(hIn);
		CloseHandle(hOut);
		ZeroMemory(szBuf, sizeof(szBuf));
		ZeroMemory(testfin, sizeof(testfin));
		ischangeMAXexist = false;
		// Определяем состояние канала Mailslot
		fReturnCode = GetMailslotInfo(
			hMailslot1, NULL, &cbMessages, &cbMsgNumber, NULL);
		if (!fReturnCode)
		{
			fprintf(stdout, "GetMailslotInfo: Error %ld\n",
				GetLastError());
			_getch();
			break;
		}
		// Если в канале есть Mailslot сообщения, читаем первое из них и выводим на экран
		if (cbMsgNumber != 0)
		{
			if (ReadFile(hMailslot1, szBuf, 512, &cbRead, NULL))
			{
				// Выводим принятую строку на консоль 
				printf("Received: <%s>\n", szBuf);
				printf("CbRead: %d\n", cbRead);
				// Если пришла команда "exit", завершаем работу приложения
				if (!strcmp(szBuf, "exit"))
					break;
				// Иначе считаем что принято имя файла
				else
				{
					// Открываем канал с процессом MSLOTCLIENT
					hMailslot2 = CreateFile(lpszWriteMailslotName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
					// Если возникла ошибка, выводим ее код и завершаем работу приложения
					if (hMailslot2 == INVALID_HANDLE_VALUE)
					{
						fprintf(stdout, "CreateFile for send: Error %x\n", GetLastError());
						_getch();
						break;
					}
					for (i = 0; i < cbRead; i++) {
						if (szB[i] == '/') {
							ischangeMAXexist = true;
							break;
						}
					}
					i = 0;
					if (ischangeMAXexist == true) {
						while (szB[i] != '/') {
							testfin[i] = szB[i];
							i++;
							digitcnt++;
						}
						changeMAX = atoi(testfin);
						printf("changeMAX is:%d\n", changeMAX);
						if (testfin[0] != '0' and atoi(testfin) == 0) {
							printf("Wrong parameter. Parameter must be a number\n");
							sprintf(message2, "(Server)Wrong parameter. Parameter must be a number.");
							(WriteFile(hMailslot2, message2, strlen(message2) + 1, &cbWritten, NULL));
							CloseHandle(hMailslot2);
						}
						//ZeroMemory(testfin, sizeof(testfin));
					}
					else {
						changeMAX = 9999; //если максимальное число замен не задано, меняем все
						printf("changeMAX was not entered, the whole file will be processed\n");
					}
					//printf("changeMAX is:%d\n", changeMAX);
					i = 0;
					if (ischangeMAXexist == true) {
						for (DWORD k = 0; k < cbRead - 1 - digitcnt; k++) {
							filename[k] = szBuf[k + 1 + digitcnt];
						}
					}
					else strcpy(filename, szBuf);

					printf("filename is:%s\n", filename);
					digitcnt = 0;
					strcpy(nm, "Output");
					strcat(nm, filename);
					printf("output filename is:%s\n", name);
					hIn = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
					if (hIn == INVALID_HANDLE_VALUE) {
						sprintf(message, "(Server)Can not open file or file does not exist. Error: %x", GetLastError());
						printf("Can not open file or file does not exist. Error: %x\n", GetLastError());
						(WriteFile(hMailslot2, message, strlen(message) + 1, &cbWritten, NULL));
						CloseHandle(hMailslot2);
						continue;
					}

					hOut = CreateFile(nm, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hOut == INVALID_HANDLE_VALUE) {
						sprintf(message, "(Server)Can not open out file. Error: %x", GetLastError());
						printf("Can not open out file. Error: %x\n", GetLastError());
						(WriteFile(hMailslot2, message, strlen(message) + 1, &cbWritten, NULL));
						continue;
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
							printf("Fatal recording error: %x\n", GetLastError());
							return -1;
						}
					}
				}
				// сообщение в консоль ошибок
				sprintf(message, "(Server): file:%s, changes = %d\n", filename, total);
				WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
				// сообщение в канал
				sprintf(message, "%d", total);
				total = 0;
				if (WriteFile(hMailslot2, message, strlen(message) + 1, &cbWritten, NULL)) {
					printf("Message is:%s\n", message);
					printf("Bytes sent %d\n", cbWritten);
				}
			}
		}
		// Выполняем задержку на  500 миллисекунд
		Sleep(500);
		//конец цикла while
	}
	CloseHandle(hMailslot1);
	CloseHandle(hMailslot2);
	return 0;
}