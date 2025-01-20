
#include <windows.h> 
#include <stdio.h> 
#include <string.h> 
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
	DWORD allfilesres = 0;
	if (argc < 2)
	{
		printf("Usage: programname.exe changeMAX/filename...");
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
		printf("(Main)For query %s found changes %d\n", data[i].file, data[i].changes);
		allfilesres += data[i].changes;
		CloseHandle(hThreads[i]);
	}
	printf("(Main)Total changes for all files is %d\n", allfilesres);
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
	DWORD j;
	DWORD k;
	char testfin[10] = { 0 };
	char filename[80] = { 0 };
	DWORD digitcnt = 0;
	int i = *((int*)num); //data.num;
	int total = 0;
	//bool
	bool ischangeMAXexist;
	printf("Thread #%d wait mutex...\n", i);
	// захват мьютекса
	WaitForSingleObject(hMutex, INFINITE);
	CloseHandle(hIn);
	CloseHandle(hOut);
	ZeroMemory(str, sizeof(str));
	ZeroMemory(testfin, sizeof(testfin));
	ischangeMAXexist = false;

	strcpy(str, data[i].file);
	printf("Received: <%s>\n", str);
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
		changeMAX = atoi(testfin);
		//printf("changeMAX is:%d\n", changeMAX);
		if (testfin[0] != '0' and atoi(testfin) == 0) {
			printf("Wrong parameter. Parameter must be a number\n");
			data[i].changes = 0;
			return 0;
		}
	}
	else {
		changeMAX = 9999; //если максимальное число замен не задано, меняем все
		//printf("For changeMAX was not entered, the whole file will be processed\n");
	}
	j = 0;
	if (ischangeMAXexist == true) {
		for (DWORD k = 0; k < strlen(str) - 1 - digitcnt; k++) {
			filename[k] = str[k + 1 + digitcnt];
		}
	}
	else strcpy(filename, str);

	if (ischangeMAXexist) printf("For file %s changeMAX is:%d\n", filename, changeMAX);
	else printf("For file %s changeMAX was not entered, the whole file will be processed\n", filename);

	//printf("filename is:%s\n", filename);
	digitcnt = 0;
	printf("Thread #%d catch mutex! Open file %s...\n", i, filename);
	strcpy(nm, "Output");
	strcat(nm, filename);
	hIn = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		printf("Can not open file or file does not exist. Error: %x\n", GetLastError());
		data[i].changes = 0;
		return  0;
	}
	hOut = CreateFile(nm, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		printf("Can not open out file. Error: %x\n", GetLastError());
		data[i].changes = 0;
		return 0;
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
	printf("(ThreadID: %lu), File: %s changes = %d\n", GetCurrentThreadId(), filename, total);
	data[i].changes = total;
	// освобождение мьютекса
	ReleaseMutex(hMutex);
	return total;
}