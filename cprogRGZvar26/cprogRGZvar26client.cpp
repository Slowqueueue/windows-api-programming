#include <windows.h>
#include <stdio.h>

#define BUF_SIZE 256

HANDLE hMutexClient;
HANDLE hMutexServer;
const char* mutexClientName = "Global\\mutexClient";
const char* mutexServerName = "Global\\mutexServer";

HANDLE hFileMapping;
LPVOID lpFileMap;
const char* fileShareName = "Global\\fileMap";

void OutputError(const char* error)
{
	int x = GetLastError();
	if (x == 0)
		x = -1;

	printf("%s, code:%d\n", error, x);
	exit(-1);
}

void SetMutexesToDefaultState() {
	ReleaseMutex(hMutexServer);
	WaitForSingleObject(hMutexClient, INFINITE);
}

int UseService()
{
	DWORD bytesRead, bytesWritten;
	char buffer[BUF_SIZE] = { 0 };

	printf("Print \"stop service\" to stop service and exit\n\n");

	int iteraction = 0;

	hMutexServer = OpenMutex(MUTEX_ALL_ACCESS, false, mutexServerName);

	int timeout = INFINITE;
	while (iteraction < 2)
	{
		WaitForSingleObject(hMutexServer, timeout);
		strcpy(buffer, (char*)lpFileMap);
		printf("Service message:%s\n", buffer);

		printf("Client answer:");
		fgets(buffer, BUF_SIZE - 1, stdin);
		buffer[strlen(buffer) - 1] = 0; // to remove \n

		strcpy((char*)lpFileMap, buffer);
		ReleaseMutex(hMutexClient);

		if (strcmp(buffer, "stop service") == 0)
			return -1;

		for (int i = 0; i < BUF_SIZE; i++)
			buffer[i] = 0;

		iteraction++;
		SetMutexesToDefaultState();
	}
	CloseHandle(hMutexClient);
	WaitForSingleObject(hMutexServer, INFINITE);

	strcpy(buffer, (char*)lpFileMap);
	printf("Service work result: %s\n\n", buffer);
	return 0;
}

int main()
{
	hMutexClient = CreateMutex(NULL, TRUE, mutexClientName);

	if (hMutexClient == INVALID_HANDLE_VALUE)
		OutputError("Error create mutex");

	hFileMapping = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, fileShareName);
	if (hFileMapping == NULL)
		OutputError("Error open file mapping");

	lpFileMap = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, BUF_SIZE);
	if (hFileMapping == NULL)
		OutputError("Error create map view");

	UseService();

	CloseHandle(hMutexServer);
	UnmapViewOfFile(lpFileMap);
	CloseHandle(hFileMapping);
	system("pause");
	return 0;
}