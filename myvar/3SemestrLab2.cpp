#include <windows.h> 
#include <iostream>
#include <stdio.h> 
#include <tchar.h>
#define BUF_SIZE 256
using namespace std;
HINSTANCE hlib;
DWORD(*Bufferchange)(DWORD, CHAR*, DWORD);

int main(int argc, LPTSTR argv[]) {
	setlocale(LC_ALL, "Rus");
	HANDLE hIn, hOut;
	DWORD nIn, nOut, count = 0;
	CHAR Buffer[BUF_SIZE];
	DWORD changeMAX = atoi(argv[3]);
	

	if (argc != 4) {
		printf("Использование: cpw файл1 файл2\n");
		return  -1;
	}
	hIn = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		printf("Невозможно открыть входной файл. Ошибка: %x\n", GetLastError());
		return  -1;
	}
	hOut = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		printf("Невозможно открыть выходной файл. Ошибка: %x\n", GetLastError());
		return -1;
	}
	while (ReadFile(hIn, Buffer, BUF_SIZE, &nIn, NULL) && nIn > 0)
	{
		hlib = LoadLibrary("C:\\Users\\Nikita\\source\\repos\\DLL2\\Debug\\DLL2.dll");
		if (hlib == NULL) {
			printf("Невозможно открыть файл библиотеки. Ошибка: %x\n", GetLastError());
			return -1;
		}
		if (hlib != NULL) printf("Библиотека подключена\n");

		Bufferchange = (DWORD(*)(DWORD, CHAR*, DWORD))GetProcAddress(hlib, "Bufferchange");
		if (Bufferchange == NULL) {
			printf("Функция отсутсвует в библиотеке. Ошибка: %x\n", GetLastError());
			return -1;
		}
		if (Bufferchange != NULL) count = Bufferchange(nIn, Buffer, changeMAX);

		WriteFile(hOut, Buffer, nIn, &nOut, NULL);
		if (nIn != nOut) {
			printf("Неустранимая ошибка записи: %x\n", GetLastError());
			return -1;
		}
	}
	printf("Количество замен: %d", count);
	CloseHandle(hIn);
	CloseHandle(hOut);
	FreeLibrary(hlib);
	return count;
}