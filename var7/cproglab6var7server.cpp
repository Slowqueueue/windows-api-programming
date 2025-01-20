#include <windows.h>
#include <stdio.h>
#include <conio.h>
#define BUF_SIZE 256
// Идентификаторы объектов-семафоров, которые используются для синхронизации задач, принадлежащих разным процессам
HANDLE hSemSend;
HANDLE hSemRecv;
HANDLE hSemTermination;
HANDLE hSems[2];
// Имя объектов-семафоров для синхронизации записи и чтения из отображаемого файла
CHAR lpSemSendName[] = "$MySemSendName$";
CHAR lpSemRecvName[] = "$MySemRecvName$";
// Имя объекта-события для завершения процесса
CHAR lpSemTerminationName[] = "$MySemTerminationName$";
// Имя отображния файла на память
CHAR lpFileShareName[] = "$MyFileShareName$";
// Идентификатор отображения файла на память
HANDLE hFileMapping;
// Указатель на отображенную область памяти
LPVOID lpFileMap;
LONG res;
LPLONG lpres = &res;
int main()
{
	CHAR Buffer[BUF_SIZE] = { 0 };
	CHAR Buffer2[BUF_SIZE] = { 0 };
	HANDLE hIn = { 0 };
	HANDLE hOut = { 0 };
	DWORD dwRetCode;
	DWORD nIn, nOut = 0;
	DWORD i = 0, k = 0, j = 0;
	DWORD digitcnt = 0;
	DWORD changeMAX = 0;
	char name[80];
	char* nm = name;
	CHAR string[80];
	char* str = string;
	char testfin[10] = { 0 };
	char filename[80] = { 0 };
	DWORD cbWritten;
	DWORD total = 0;
	//bool
	bool ischangeMAXexist;
	// буфер для  сообщения об ошибке, результата
	char message[80] = { 0 };
	char message2[80] = { 0 };
	printf("Mapped and shared file, semaphore sync, server process\n");
	// Создаем объекты-семафоры для синхронизации записи и чтения в отображаемый файл, выполняемого в разных процессах
	hSemSend = CreateSemaphore(NULL, 0, 1, lpSemSendName);
	hSemRecv = CreateSemaphore(NULL, 0, 1, lpSemRecvName); // установлен изначально!
	// Если произошла ошибка, получаем и отображаем ее код, а затем завершаем работу приложения
	if (hSemSend == NULL || hSemRecv == NULL)
	{
		fprintf(stdout, "CreateSemaphore: Error %ld\n", GetLastError());
		_getch();
		return 0;
	}
	// Если объект-семафор с указанным именем существует, считаем, что приложение-сервер уже было запущено
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		printf("\nServerApplication already started\n"
			"Press any key to exit...");
		_getch();
		return 0;
	}
	// Создаем объект-семафор для определения момента завершения работы процесса ввода
	hSemTermination = CreateSemaphore(NULL, 0, 1, lpSemTerminationName);
	if (hSemTermination == NULL)
	{
		fprintf(stdout, "CreateSemaphore (Termination): Error %ld\n", GetLastError());
		_getch();
		return 0;
	}
	// Создаем объект-отображение, файл не создаем!!!
	hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 100, lpFileShareName);
	// Если создать не удалось, выводим код ошибки
	if (hFileMapping == NULL)
	{
		fprintf(stdout, "CreateFileMapping: Error %ld\n", GetLastError());
		_getch();
		return 0;
	}
	// Выполняем отображение файла на память.
	// В переменную lpFileMap будет записан указатель на отображаемую область памяти
	lpFileMap = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	// Если выполнить отображение не удалось, выводим код ошибки
	if (lpFileMap == 0)
	{
		fprintf(stdout, "MapViewOfFile: Error %ld\n", GetLastError());
		_getch();
		return 0;
	}
	// Готовим массив идентификаторов семафоров для функции WaitForMultipleObjects
	hSems[0] = hSemTermination;
	hSems[1] = hSemSend;
	// Цикл чтения/записи. Этот цикл завершает свою работу при завершении процесса-клиента
	while (1)
	{
		total = 0;
		// Выполняем ожидание одного из двух событий:
		//   - завершение клиентского процесса;
		//   - завершение записи данных клиентом
		dwRetCode = WaitForMultipleObjects(2, hSems, FALSE, INFINITE);
		// Если ожидание любого из двух событий было отменено, или если произошла ошибка, прерываем цикл
		if (dwRetCode == WAIT_ABANDONED_0 ||
			dwRetCode == WAIT_ABANDONED_0 + 1 ||
			dwRetCode == WAIT_OBJECT_0 ||
			dwRetCode == WAIT_FAILED)
			break;
		// если произошла установка любого из двух семафоров из массива
		// Читаем данные (имя файла для обработки) из отображенной
		// области памяти, записанный туда клиентским 
		// процессом, и отображаем его в консольном окне
		else
		{
			CloseHandle(hIn);
			CloseHandle(hOut);
			ZeroMemory(str, sizeof(str));
			ZeroMemory(testfin, sizeof(testfin));
			ischangeMAXexist = false;
			puts(((LPSTR)lpFileMap));
			// обработка данных
			strcpy(str, ((LPSTR)lpFileMap));
			printf("Received: <%s>\n", str);
			for (i = 0; i < strlen(str); i++) {
				if (str[i] == '/') {
					ischangeMAXexist = true;
					break;
				}
			}
			i = 0;
			if (ischangeMAXexist == true) {
				while (str[i] != '/') {
					testfin[i] = str[i];
					i++;
					digitcnt++;
				}
				changeMAX = atoi(testfin);
				printf("changeMAX is:%d\n", changeMAX);
				if (testfin[0] != '0' and atoi(testfin) == 0) {
					printf("Wrong parameter. Parameter must be a number\n");
					sprintf(message2, "(Server)Wrong parameter. Parameter must be a number.");
					strcpy(((LPSTR)lpFileMap), message2);
					ReleaseSemaphore(hSemRecv, 1, lpres);
					continue;
				}
			}
			else {
				changeMAX = 9999; //если максимальное число замен не задано, меняем все
				printf("changeMAX was not entered, the whole file will be processed\n");
			}
			i = 0;
			if (ischangeMAXexist == true) {
				for (DWORD k = 0; k < strlen(str) - 1 - digitcnt; k++) {
					filename[k] = str[k + 1 + digitcnt];
				}
			}
			else strcpy(filename, str);
			printf("filename is:%s\n", filename);
			digitcnt = 0;
			strcpy(nm, "Output");
			strcat(nm, filename);
			hIn = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (hIn == INVALID_HANDLE_VALUE) {
				sprintf(message, "(Server)Can not open file or file does not exist. Error: %x", GetLastError());
				printf("Can not open file or file does not exist. Error: %x\n", GetLastError());
				strcpy(((LPSTR)lpFileMap), message);
				ReleaseSemaphore(hSemRecv, 1, lpres);
				continue;
			}
			hOut = CreateFile(nm, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hOut == INVALID_HANDLE_VALUE) {
				sprintf(message, "(Server)Can not open out file. Error: %x", GetLastError());
				printf("Can not open out file. Error: %x\n", GetLastError());
				strcpy(((LPSTR)lpFileMap), message);
				ReleaseSemaphore(hSemRecv, 1, lpres);
				continue;
			}
			while (ReadFile(hIn, Buffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {

				for (DWORD i = 0; i < nIn; i++)
				{
					if (Buffer[i] == ' ') {
						if (changeMAX > 0) {
							j = i;
							k = i;
							for (DWORD i = 0; i < nIn; i++) {
								Buffer[j] = Buffer[j + 1];
								j++;
							}
							i = k;
							total++;
							changeMAX--;
							nIn--;
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
		strcpy(((LPSTR)lpFileMap), message);
		total = 0;
		// разрешакм читать клиенту
		ReleaseSemaphore(hSemRecv, 1, lpres);
		//Выполняем задержку на 500 милисекунд
		Sleep(500);
	}
	// Закрываем идентификаторы объектов-семафоров  
	CloseHandle(hSemSend);
	CloseHandle(hSemRecv);
	CloseHandle(hSemTermination);
	// Отменяем отображение файла
	UnmapViewOfFile(lpFileMap);
	// Освобождаем идентификатор созданного объекта-отображения
	CloseHandle(hFileMapping);
	return 0;
}