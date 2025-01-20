#include <windows.h> 
#include <iostream>
#include <stdio.h> 
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
using namespace std;
#define BUF_SIZE 256

int main(int argc, LPTSTR argv[]) {  //exe
	setlocale(LC_ALL, "Rus");

	HANDLE hIn, hOut;
	DWORD nIn, nOut = 0;
	DWORD total = 0;
	char filename[80] = { 0 };
	CHAR arguments[80];
	char* arg = arguments;
	char message[80] = { 0 };
	CHAR Buffer[BUF_SIZE];
	char testfin[10] = { 0 };
	DWORD digitcnt = 0;
	DWORD i = 0;
	DWORD changeMAX = 0;
	char name[80];
	char* nm = name;
	char* line;

	if (argc < 2) {
		printf("No file to process!\n");
		exit(-1);
	}
	// запрос к ОС на открытие файла (только для чтения) 
	line = (char*)GetCommandLine();
	strcpy(arg, argv[1]);
	while (arg[i] != '/') {
		testfin[i] = arg[i];
		i++;
		digitcnt++;
	}
	changeMAX = atoi(testfin);
	memset(testfin, 0, sizeof(testfin));

	for (DWORD i = 0; i < strlen(arg) - 1 - digitcnt; i++) {
		filename[i] = arguments[i + 1 + digitcnt];
	}

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
			printf("Неустранимая ошибка записи: %x\n", GetLastError());
			return -1;
		}
	}
	// сообщение в консоль ошибок 

	Sleep(500);
	sprintf(message, "(Child): %lu, file:%s, changes = %d", GetCurrentProcessId(), filename, total);

	// закрываем дискрипторы
	CloseHandle(hIn);
	CloseHandle(hOut);

	return total;
}