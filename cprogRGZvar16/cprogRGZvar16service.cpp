#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <AclAPI.h>

#define BUF_SIZE 256

extern int addLogMessage(const char* text);
int ServiceStart();
void ServiceStop();


// Идентификаторы почтовых ящиков (туда и обратно)
HANDLE hMailslot1, hMailslot2;

// Имя серверного канала Mailslot
LPSTR  lpszReadMailslotName = (LPSTR)"\\\\.\\mailslot\\$Channel1$";
// Имя клиентского канала Mailslot
LPSTR  lpszWriteMailslotName = (LPSTR)"\\\\.\\mailslot\\$Channel2$";

int Server()
{
	HANDLE hIn, hOut;
	DWORD nIn, nOut = 0;
	CHAR Buffer[BUF_SIZE];
	INT numFromCMD = 0;

	char BufNumFromCMD[20];
	char clearBuffer[BUF_SIZE];
	int index = 0, newIndex = 0, space = 0, j = 0;

	// Код возврата из функций
	BOOL   fReturnCode;

	DWORD  cbMessages, cbMsgNumber;

	// Буфер для передачи данных через канал
	char   szBuf[512];

	DWORD  cbRead, cbWritten, total = 0;

	// буферы для  сообщения об ошибке, результата
	char message[260] = { 0 };
	char message2[260] = { 0 };
	sprintf(message, "Mailslot server demo\n");
	addLogMessage(message);

	// Ожидаем соединения со стороны клиента
	// Цикл получения команд через канал
	while (1)
	{
		ZeroMemory(szBuf, sizeof(szBuf));

		// Определяем состояние канала Mailslot
		fReturnCode = GetMailslotInfo(hMailslot1, NULL, &cbMessages, &cbMsgNumber, NULL);
		if (!fReturnCode)
		{
			sprintf(message, "GetMailslotInfo: Error %ld\n", GetLastError());
			addLogMessage(message);
			continue;
		}
		// Если в канале есть Mailslot сообщения,
		// читаем первое из них и выводим на экран
		if (cbMsgNumber != 0)
		{
			if (ReadFile(hMailslot1, szBuf, 512, &cbRead, NULL))
			{
				index = 0, newIndex = 0, space = 0, j = 0;

				// Выводим принятую строку на консоль 
				sprintf(message, "Received: <%s>\n", szBuf);
				addLogMessage(message);

				// Если пришла команда "exit", завершаем работу приложения
				if (!strcmp(szBuf, "exit"))
				{
					sprintf(message, "Client exited!\n");
					addLogMessage(message);
					return(-1);
					// Иначе считаем что принято имя файла
				}
				else
				{
					char fileName[160] = "C://Users/Nikita/source/repos/cprogRGZvar16/cprogRGZvar16service/x64/Debug/";
					strcat(fileName, szBuf);

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


					// Открываем канал с процессом MSLOTCLIENT
					hMailslot2 = CreateFile(lpszWriteMailslotName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
					if (hMailslot2 == INVALID_HANDLE_VALUE)
					{
						sprintf(message, "CreateFile for send: Error %ld\n", GetLastError());
						addLogMessage(message);
						CloseHandle(hMailslot2);
						continue;
					}

					if (emptyNumver == true) {
						numFromCMD = 9999;
					}
					else {
						numFromCMD = atoi(BufNumFromCMD);
					}

			

					if (numFromCMD == 0) {
						sprintf(message2, "(Server)Wrong parameter");
						addLogMessage(message2);
						WriteFile(hMailslot2, message2, strlen(message2) + 1, &cbWritten, NULL);
						sprintf(message, "(Server)Sent: <%s>\n", message2);
						addLogMessage(message);
						sprintf(message, "(Server)Sent %d bytes\n", cbWritten);
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
						sprintf(message2, "(Server)Can not open file or file does not exist. Error: %x", GetLastError());
						addLogMessage(message2);
						WriteFile(hMailslot2, message2, strlen(message2) + 1, &cbWritten, NULL);
						sprintf(message, "(Server)Sent: <%s>\n", message2);
						addLogMessage(message);
						sprintf(message, "(Server)Sent %d bytes\n", cbWritten);
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
							sprintf(message, "(Server)Can not open out file. Eror: %x\n", GetLastError());
							addLogMessage(message);
							return -1;
						}

						while (ReadFile(hIn, Buffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {
							total = 0;
							for (DWORD i = 0; i < nIn; i++)
							{
								if (int(Buffer[i]) < 48) {
									if (numFromCMD > 0) {
										if (Buffer[i] != ' ') {
											Buffer[i] = ' ';
											total++;
											numFromCMD--;
										}
									}
								}
							}
							WriteFile(hOut, Buffer, nIn, &nOut, NULL);

							// сообщение в консоль ошибок
							sprintf(message, "(Server): file:%s, Max changes:%d, changes:%d\n", nameoffile, numFromCMD + total, total);
							addLogMessage(message);
							WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
							// сообщение в канал
							if (emptyNumver == true) {
								sprintf(clearBuffer, "No parameter. Max changes will be 9999. Changes:%d", total);
								WriteFile(hMailslot2, clearBuffer, strlen(clearBuffer) + 1, &cbWritten, NULL);
							}
							else {
								sprintf(clearBuffer, "%d", total);
								WriteFile(hMailslot2, clearBuffer, strlen(clearBuffer) + 1, &cbWritten, NULL);
							}
							sprintf(message, "(Server)Sent: <%s>\n", clearBuffer);
							addLogMessage(message);
							sprintf(message, "(Server)Sent %d bytes\n", cbWritten);
							addLogMessage(message);
							CloseHandle(hIn);
							CloseHandle(hOut);
						}
					}
				}
			}
			else
			{
				sprintf(message, "ReadFile: Error %ld\n", GetLastError());
				addLogMessage(message);
				break;
			}
		}

		Sleep(500);
	}
	return 0;
}

int ServiceStart()
{
	char message[80] = { 0 };
	DWORD res;
	// Создаем канал Mailslot, имеющий имя lpszReadMailslotName
	hMailslot1 = CreateMailslot(
		lpszReadMailslotName, 0,
		MAILSLOT_WAIT_FOREVER, NULL);
	// Если возникла ошибка, выводим ее код и зваершаем работу приложения
	if (hMailslot1 == INVALID_HANDLE_VALUE)
	{
		sprintf(message, "CreateMailslot: Error %ld\n", GetLastError());
		addLogMessage(message);
		return -1;
	}
	// Выводим сообщение о создании канала
	SetSecurityInfo(hMailslot1, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);
	sprintf(message, "Mailslot created");
	addLogMessage(message);
	return 0;
}

void ServiceStop()
{
	CloseHandle(hMailslot1);
	addLogMessage("Mailslot closed!");
}