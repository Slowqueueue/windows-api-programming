﻿// Программа из конспекта "Системное программное обеспечение"
// Файлы, отображаемые на память, синхронизация семафорами
// стр. ?? 
// Приложение MFS_SERVER (серверное приложение)
// Демонстрация использования файлов, отображаемых на память, 
// для передачи данных между процессами, синхронизированными семафорами
#define _CRT_SECURE_NO_WARNINGS
#define BUF_SIZE 256
#include <windows.h>
#include <stdio.h>
#include <conio.h>

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
	CHAR Buffer[BUF_SIZE];
	HANDLE hIn = { 0 };
	HANDLE hOut = { 0 };
	DWORD dwRetCode;
	DWORD nIn, nOut = 0;
	DWORD i = 0;
	DWORD chacha = 0;
	DWORD changeMAX = 0;
	char name[80];
	char* nm = name;
	CHAR string[80];
	char* str = string;
	char testfin[10] = { 0 };
	char filename[80] = { 0 };
	DWORD cbWritten;
	DWORD total = 0;
	// буфер для  сообщения об ошибке, результата
	char message[80] = { 0 };
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
			puts(((LPSTR)lpFileMap));
			// обработка данных
			strcpy(str, ((LPSTR)lpFileMap));
			while (str[i] != '/') {
				testfin[i] = str[i];
				i++;
				chacha++;
			}
			changeMAX = atoi(testfin);
			memset(testfin, 0, sizeof(testfin));
			i = 0;
			for (DWORD k = 0; k < strlen(str) - chacha; k++) { 
				filename[k] = str[k + 1 + chacha];
			}
			chacha = 0;
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