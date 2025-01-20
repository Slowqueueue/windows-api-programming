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
	char result[80] = { 0 };
	CHAR Buffer[BUF_SIZE];
	CHAR Buffer2[BUF_SIZE] = { 0 };
	DWORD cbWritten = 0;
	char testfin[10] = { 0 };
	DWORD digitcnt = 0;
	DWORD i = 0;
	DWORD j = 0;
	DWORD k = 0;
	DWORD changeMAX = 0;

	char name[80];
	char* nm = name;

	WriteFile(GetStdHandle(STD_ERROR_HANDLE), "(Child)File recieved\n", 20, &cbWritten, NULL);
	ReadFile(GetStdHandle(STD_INPUT_HANDLE), arguments, 80, &cbWritten, NULL);

	while (arg[i] != '/') {
		testfin[i] = arg[i];
		i++;
		digitcnt++;
	}
	changeMAX = atoi(testfin);

	for (DWORD i = 0; i < cbWritten - 1 - digitcnt; i++) {
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
			if (Buffer[i] == '.') {
				j = i;
				if (changeMAX > 0) {
					while (i < nIn - 1) {
						Buffer2[i - j] = Buffer[i + 1];
						i++;
					}
					i = j;
					Buffer[i + 1] = '\n';
					nIn++;
					while (i < nIn - 1) {
						Buffer[i + 2] = Buffer2[k];
						k++;
						i++;
					}
					total++;
					changeMAX--;
					i = j + 2;
					k = 0;
				}
			}
		}
			WriteFile(hOut, Buffer, nIn, &nOut, NULL);
			if (nIn != nOut) {
				printf("Неустранимая ошибка записи: %x\n", GetLastError());
				return -1;
			}
	}

	Sleep(500);
	sprintf(message, "(Child): %lu, file:%s, changes = %d", GetCurrentProcessId(), filename, total);
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
	// сообщение в канал 

	sprintf(result, "%d", total);
	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), result, strlen(result) + 1, &cbWritten, NULL);
	// закрываем дискрипторы
	CloseHandle(hIn);
	CloseHandle(hOut);
	return total;
}