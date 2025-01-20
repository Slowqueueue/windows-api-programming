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
	// Количество байт данных, принятых через канал
	DWORD cbRead;
	// Количество байт данных, переданных через канал
	DWORD  cbWritten;
	DWORD  total = 0;
	// буфер для  сообщения об ошибке, результата
	char message[80] = { 0 };
	// хендлы для работы с файлом
	HANDLE hIn = { 0 } ;
	HANDLE hOut = { 0 };
	// всякие дворды для работы с файлом
	DWORD nIn, nOut = 0;
	DWORD i = 0;
	DWORD chacha = 0;
	DWORD changeMAX = 0;
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
				//}
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
					while (szB[i] != '/') {
						testfin[i] = szB[i];
						i++;
						chacha++;
					}
					printf("testfin is:%s\n", testfin);
					changeMAX = atoi(testfin); 
					printf("changeMAX is:%d\n", changeMAX);
					i = 0;
					for (DWORD k = 0; k < cbRead - 1 - chacha; k++) {
						filename[k] = szBuf[k + 1 + chacha];  
					}
					printf("filename is:%s\n", filename);
					chacha = 0;
					strcpy(nm, "Output");
					strcat(nm, filename);
					printf("output filename is:%s\n", name);
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