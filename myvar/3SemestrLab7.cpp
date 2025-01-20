// Программа из конспекта "Системное программное обеспечение"
// Версия для Windows
// стр. ?? 
// Приложение MTHREADS
// многопоточное приложение, синхронизация мьютексами
#define _CRT_SECURE_NO_WARNINGS
#include   <windows.h> 
#include   <stdio.h> 
#include   <string.h> 
#define threads 64
#define BUF_SIZE 256
// глобальный мьютекс
HANDLE hMutex;
// стркутура для обмена данными
struct MYDATA
{
	int num;
	char file[80];
	int changes;
} data[threads];
// функция потока
DWORD WINAPI threadfunc(LPVOID);
int main(int argc, char* argv[])
{
	// дескрипторы потоков
	HANDLE hThreads[threads];
	// вспомогательные переменные
	int i;
	DWORD res;
	if (argc < 3)
	{
		printf("Usage: mthreads.exe filename ...");
		exit(-1);
	}
	// создание мьютекса
	hMutex = CreateMutex(NULL, FALSE, NULL);
	// захват мьютекса
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0; i < argc - 1; i++)
	{	
		// заполнение структуры данных
		data[i].num = i;
		strcpy(data[i].file, argv[i + 1]);
		data[i].file[strlen(argv[i + 1])] = 0;
		data[i].changes = -1;
		hThreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadfunc, (LPVOID)&data[i], 0, NULL);
		if (hThreads[i] == NULL)
		{
			printf("Cant create thread %d!\n", i);
			exit(-i);
		}
		else printf("(Main)Create thread %d!\n", i);
	}
	// освобождение мьютекса, чтобы отработали потоки
	ReleaseMutex(hMutex);
	// удаляем мьютекс
	CloseHandle(hMutex);
	// закрываем дескрипторы потоков
	for (i = 0; i < argc - 1; i++)
	{
		// убедимся, что все потоки завершены
		WaitForSingleObject(hThreads[i], INFINITE);
		// проверка результата
		GetExitCodeThread(hThreads[i], &res);
		printf("(Main)In file %s found changes %d\n", data[i].file, data[i].changes);
		CloseHandle(hThreads[i]);
	}
}
DWORD WINAPI threadfunc(LPVOID num)
{
	CHAR string[80];
	CHAR Buffer[BUF_SIZE];
	HANDLE hIn = { 0 };
	HANDLE hOut = { 0 };
	DWORD nIn, nOut = 0;
	char name[80];
	char* nm = name;
	char* str = string;
	DWORD changeMAX;
	char testfin[10] = { 0 };
	char filename[80] = { 0 };
	DWORD chacha = 0;
	int i = *((int*)num); //data.num;
	int total = 0;
	printf("Thread #%d wait mutex...\n", i);
	// захват мьютекса
	WaitForSingleObject(hMutex, INFINITE);
;
	strcpy(str, data[i].file);
	DWORD j = 0;
	while (str[j] != '/') {
		testfin[j] = str[j];
		j++;
		chacha++;
	}
	changeMAX = atoi(testfin);

	j = 0;
	for (DWORD k = 0; k < strlen(str) - chacha; k++) {
		filename[k] = str[k + 1 + chacha];
	}
	printf("Thread #%d catch mutex! Open file %s...\n", i, filename);
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
	printf("(ThreadID: %lu), File: %s changes = %d\n", GetCurrentThreadId(), filename, total);
	 	data[i].changes = total;
	// освобождение мьютекса
	ReleaseMutex(hMutex);
	return total;
}