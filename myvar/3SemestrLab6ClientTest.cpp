// Программа из конспекта "Системное программное обеспечение"
// Файлы, отображаемые на память, синхронизация семафорами
// стр. ?? 
// Приложение MFS_CLIENT (клиентское приложение)
// Демонстрация использования файлов, отображаемых на память, 
// для передачи данных между процессами, синхронизированными семафорами
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

// Идентификаторы объектов-семафоров, которые используются
// для синхронизации задач, принадлежащих разным процессам
HANDLE hSemSend;
HANDLE hSemRecv;
HANDLE hSemTermination;
// Имя объектов-семафоров для синхронизации записи и чтения из отображаемого файла
CHAR lpSemSendName[] = "$MySemSendName$";
CHAR lpSemRecvName[] = "$MySemRecvName$";
// Имя объекта-семафора для завершения процесса
CHAR lpSemTerminationName[] = "$MySemTerminationName$";
// Имя отображния файла на память
CHAR lpFileShareName[] = "$MyFileShareName$";
// Идентификатор отображения файла на память
HANDLE hFileMapping;
// Указатель на отображенную область памяти
LPVOID lpFileMap;

int main()
{
	CHAR str[80];
	DWORD dwRetCode;
	printf("Mapped and shared file, Sem sync, client process\n"
		"\n\nEnter  <Exit> to terminate...\n");
	// Открываем объекты-события для синхронизации чтения и записи
	hSemSend = OpenSemaphore(SEMAPHORE_MODIFY_STATE, FALSE, lpSemSendName); //меняем значение здесь
	hSemRecv = OpenSemaphore(SYNCHRONIZE, FALSE, lpSemRecvName); //меняем значение НЕ здесь!!!
	if (hSemSend == NULL || hSemRecv == NULL)
	{
		fprintf(stdout, "OpenSem: Error %ld\n", GetLastError());
		_getch();
		return 0;
	}
	// Открываем объект-семафор для сигнализации о завершении процесса 
	hSemTermination = OpenSemaphore(SEMAPHORE_MODIFY_STATE, FALSE, lpSemTerminationName); //меняем значение здесь
	if (hSemTermination == NULL)
	{
		fprintf(stdout, "OpenSem (Termination): Error %ld\n", GetLastError());
		_getch();
		return 0;
	}
	// Открываем объект-отображение
	hFileMapping = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, lpFileShareName);
	// Если открыть не удалось, выводим код ошибки
	if (hFileMapping == NULL)
	{
		fprintf(stdout, "OpenFileMapping: Error %ld\n", GetLastError());
		_getch();
		return 0;
	}
	// Выполняем отображение файла на память.
	// В переменную lpFileMap будет записан указатель наотображаемую область памяти
	lpFileMap = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	// Если выполнить отображение не удалось, выводим код ошибки
	if (lpFileMap == 0)
	{
		fprintf(stdout, "MapViewOfFile: Error %ld\n", GetLastError());
		_getch();
		return 0;
	}

	// Цикл чтения/записи данных. Этот цикл завершает свою работу, когда пользователь набирает коанду <Exit>, 
	while (TRUE)
	{
		// Читаем введенную строку
		gets_s(str);
		// Записываем строку в отображенную память, доступную серверному процессу
		strcpy((char*)lpFileMap, str);
		// Если введена команда <Exit>, прерываем цикл
		if (!strcmp(str, "exit") || !strcmp(str, "Exit") || !strcmp(str, "EXIT"))
			break;
		// Устанавливаем объект-событие в отмеченное состояние
		ReleaseSemaphore(hSemSend, 1, NULL);
		// ждем ответа	
		dwRetCode = WaitForSingleObject(hSemRecv, INFINITE);
		// если ответ получен - выводим, если ошибка - пропусквем!
		if (dwRetCode == WAIT_OBJECT_0) puts(((LPSTR)lpFileMap));
		if (dwRetCode == WAIT_ABANDONED || dwRetCode == WAIT_FAILED)
		{
			printf("\nError waiting response! Error:%ld\n", GetLastError());
			//break;
		}
	}
	// После завершения цикла переключаем отправку в отмеченное состояние для отмены ожидания чтения в серверном процессе  
	//SetSem(hSemSend);
	ReleaseSemaphore(hSemTermination, 1, NULL);
	// Закрываем идентификаторы объектов-семафоров
	CloseHandle(hSemSend);
	CloseHandle(hSemRecv);
	CloseHandle(hSemTermination);
	// Отменяем отображение файла
	UnmapViewOfFile(lpFileMap);
	// Освобождаем идентификатор созданного
	// объекта-отображения
	CloseHandle(hFileMapping);

	return 0;
}

