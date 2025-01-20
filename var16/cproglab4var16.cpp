#include <windows.h> 
#include <iostream>
#include <stdio.h> 
#include <tchar.h>
#define BUF_SIZE 256
using namespace std;

int main(int argc, LPTSTR argv[]) {  // exe changeMAX/txt1 changeMAX/txt2...
	setlocale(LC_ALL, "Rus");
	DWORD cbWritten, cbRead;
	HANDLE hWritePipe1, hReadPipe2;
	HANDLE hReadPipe1 = { 0 };
	HANDLE hWritePipe2 = { 0 };
	CHAR path[] = { "C:\\Users\\Nikita\\source\\repos\\cproglab4var16child\\x64\\Debug\\cproglab4var16child.exe" };
	SECURITY_ATTRIBUTES PipeSA = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE }; // Для наследуемых дескрипторов
	STARTUPINFO StartInfoChild;
	PROCESS_INFORMATION ProcInfoChild;
	DWORD total = 0; //total - общее колво замен
	char result[80] = { 0 };
	CHAR filename[80] = { 0 };
	CHAR arguments[80] = { 0 };

	if (argc < 2) {
		printf("No file to process!\n");
		return -1;
	}

	GetStartupInfo(&StartInfoChild);
	// Create multiple processes.

	for (DWORD i = 0; i < (argc - 1); i++) {

		CreatePipe(&hReadPipe1, &hWritePipe1, &PipeSA, 0);
		CreatePipe(&hReadPipe2, &hWritePipe2, &PipeSA, 0);

		StartInfoChild.hStdInput = hReadPipe1; //GetStdHandle(hReadPipe1);
		StartInfoChild.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		StartInfoChild.hStdOutput = hWritePipe2;
		StartInfoChild.dwFlags = STARTF_USESTDHANDLES;

		strcpy(filename, argv[i + 1]);

		if (!CreateProcess(NULL, // No module name (use command line).
			path, // Command line.
			NULL,             // Process handle not inheritable.
			NULL,             // Thread handle not inheritable.
			TRUE,            // Set handle inheritance to FALSE.
			NULL,                // No creation flags.
			NULL,             // Use parent's environment block.
			NULL,             // Use parent's starting directory.
			&StartInfoChild,              // Pointer to STARTUPINFO structure.
			&ProcInfoChild)             // Pointer to PROCESS_INFORMATION structure.
			)
		{
			printf("CreateProcess failed.\n");
			return -1;
		}
		else {
			printf("Process %lu started for file %s\n", ProcInfoChild.dwProcessId, filename);
			Sleep(500);
		}

		/* Отправить имя файла в канал. */
		WriteFile(hWritePipe1, filename, strlen(filename), &cbWritten, NULL);
		printf("\n(Parent)Bytes written:%d\n", (int)cbWritten);
		/* Закрыть дескриптор записи канала, поскольку он больше не нужен, чтобы вторая команда могла обнаружить конец файла. */

		CloseHandle(hWritePipe1);
		/* Ожидать результат дочернего процесса. */

		ReadFile(hReadPipe2, result, 80, &cbRead, NULL);
		printf("\n(Parent)Bytes read:%d, content:%s\n", (int)cbRead, result);
		/* Закрыть дескриптор чтения из канала, поскольку он больше не нужен, чтобы вторая команда могла обнаружить конец файла. */

		CloseHandle(hReadPipe2);
		/* вывод числа замен в переданном файле или сообщение об ошибке. */

		if (atoi(result) > 0)
			printf("(Parent)ChangeMAX/File %s processed by %u has %d changes\n", filename, ProcInfoChild.dwProcessId, atoi(result));
		else
			printf("(Parent)ChangeMAX/File %s can't processed by %u because of %s\n", filename, ProcInfoChild.dwProcessId, result);

		printf("Process %lu finished by %d\n", ProcInfoChild.dwProcessId, atoi(result));
		total += atoi(result);
		/* Ожидать завершения дочернего процесса. */
		WaitForSingleObject(ProcInfoChild.hProcess, INFINITE);
		CloseHandle(ProcInfoChild.hProcess);
	}
	printf("%d File(s) processed with total changes %d!\n", argc - 1, total);
	return 0;
}