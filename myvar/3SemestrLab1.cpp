#include <windows.h> 
#include <iostream>
#include <stdio.h> 
#define BUF_SIZE 256
int main(int argc, LPTSTR argv[]) {
	setlocale(LC_ALL, "Rus");
	HANDLE hIn, hOut;
	DWORD nIn, nOut,count = 0;
	DWORD changeMAX = atoi(argv[3]);
	CHAR Buffer[BUF_SIZE];
	if (argc != 4) {
		printf("Использование: cpw файл1 файл2 максимальное кол-во замен\n");
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
	while (ReadFile(hIn, Buffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {

		for (DWORD i = 0; i < nIn - 1; i++)
		{
			if (Buffer[i] == Buffer[i + 1]) {
				if (changeMAX > 0) {
					if (Buffer[i] != ' ') {
						Buffer[i] = ' ';
						Buffer[i + 1] = ' ';
						count++;
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
	printf("Количество замен: %d", count);
	CloseHandle(hIn);
	CloseHandle(hOut);
	return count;
}