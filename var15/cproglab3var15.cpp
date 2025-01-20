#include <windows.h> 
#include <iostream>
#include <stdio.h> 
#include <tchar.h>
#define BUF_SIZE 256
using namespace std;

int main(int argc, LPTSTR argv[]) {  // exe changeMAX/txt1 changeMAX/txt2...
	setlocale(LC_ALL, "Rus");
	int i;
	STARTUPINFO si[255];
	PROCESS_INFORMATION pi[255];
	HANDLE hThread[255];
	DWORD finish, result, total = 0;
	LPDWORD res = &result;
	CHAR path[] = { "C:\\Users\\Nikita\\source\\repos\\cproglab3var15child\\x64\\Debug\\cproglab3var15child.exe" };
	char line[255];
	char* ln = line;
	if (argc < 2) {
		printf("No file to process!\n");
		return -1;
	}
	// Create multiple processes.
	for (DWORD i = 0; i < (argc - 1); i++) {
		strcpy(ln, path);
		ln = strcat(ln, " ");
		ln = strcat(ln, argv[i + 1]);
		ZeroMemory(&si[i], sizeof(si[i]));
		si[i].cb = sizeof(si);
		ZeroMemory(&pi[i], sizeof(pi[i]));
		if (!CreateProcess(NULL, // No module name (use command line).
			line, // Command line.
			NULL,             // Process handle not inheritable.
			NULL,             // Thread handle not inheritable.
			TRUE,            // Set handle inheritance to FALSE.
			NULL,                // No creation flags.
			NULL,             // Use parent's environment block.
			NULL,             // Use parent's starting directory.
			&si[i],              // Pointer to STARTUPINFO structure.
			&pi[i])             // Pointer to PROCESS_INFORMATION structure.
			)
		{
			printf("CreateProcess failed.\n");
			return -1;
		}
		else {
			printf("Process %lu started for file %s\n", pi[i].dwProcessId, argv[i + 1]);
			hThread[i] = pi[i].hProcess;
			Sleep(500);
		}
	}
	// Wait until child process exits.
	for (i = 0; i < (argc - 1); i++) {
		finish = WaitForSingleObject(pi[i].hProcess, INFINITE);
		if (finish == WAIT_OBJECT_0) {
			GetExitCodeProcess(pi[i].hProcess, res);
			printf("Process %lu finished by %d\n", pi[i].dwProcessId, result);
			total += result;
		}
		else
		{
			// Close process and thread handles.
			CloseHandle(pi[i].hProcess);
			CloseHandle(pi[i].hThread);
			TerminateProcess(pi[i].hProcess, 0);
		}
	}
	printf("%d File(s) processed with total spaces %d!\n", argc - 1, total);
	return 0;
}