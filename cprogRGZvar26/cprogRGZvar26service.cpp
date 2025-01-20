#include <windows.h> 
#include <stdio.h> 
#include <cstdlib>
#include <aclapi.h>

int AddLogMessage(const char* message, int errorCode = 0);

int WorkWithClient();

void WaitClient();

int ServiceStart();

void ServiceStop();

#define FILE_DIRECTORY "C:\\Users\\Nikita\\source\\repos\\cprogRGZvar26\\"
#define LOG_FILE_PATH "C:\\logfile.log"

#define PATH_SIZE 128
#define BUF_SIZE 256

HANDLE hFileMapping;
LPVOID lpFileMap;
const char* fileShareName = "Global\\fileMap";

HANDLE hMutexClient;
HANDLE hMutexServer;
const char* mutexClientName = "Global\\mutexClient";
const char* mutexServerName = "Global\\mutexServer";

int replacesCount = 0;

int AddLogMessage(const char* message, int errorCode)
{
	HANDLE hFile;
	DWORD bytesWritten;
	static bool isFirstLog = true;
	char buffer[BUF_SIZE] = { 0 };

	DWORD dwCreationDistribution;

	if (isFirstLog)
	{
		dwCreationDistribution = CREATE_ALWAYS;
		isFirstLog = false;
	}
	else
		dwCreationDistribution = OPEN_ALWAYS;

	hFile = CreateFile(LOG_FILE_PATH, GENERIC_WRITE, 0, NULL, dwCreationDistribution, FILE_FLAG_WRITE_THROUGH, NULL);

	if (!hFile)
		return -1;

	SetFilePointer(hFile, 0, NULL, FILE_END);

	if (errorCode == 0)
		sprintf(buffer, "%s\n", message);
	else
		sprintf(buffer, "%s, code:%d\n", message, errorCode);

	WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);
	CloseHandle(hFile);
	return 0;
}

void AddFileNameToPath(char* path, LPTSTR fileName)
{
	for (int i = 0; i < PATH_SIZE; i++)
		path[i] = '\0';
	strcat(path, FILE_DIRECTORY);
	strcat(path, fileName);
}

int SymbolCountInString(CHAR* str, char symbol)
{
	int n = 0;
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == symbol)
			n++;
	}
	return n;
}

char* CreateNewStringFromBuffer(CHAR* buffer, int replacesMaxCount)
{
	char* newStr = (char*)calloc(strlen(buffer) + SymbolCountInString(buffer, '.') + 1, sizeof(CHAR));

	int length = strlen(buffer);
	int newIndex = 0;

	for (int i = 0; i < length; i++)
	{
		newStr[newIndex] = buffer[i];
		if (buffer[i] == '.' && replacesCount < replacesMaxCount)
		{
			newIndex++;
			newStr[newIndex] = '\n';
			replacesCount++;
		}
		newIndex++;
	}

	return newStr;
}

bool IsDigit(char symbol)
{
	return 48 <= symbol && symbol <= 57;
}

int WorkWithFiles(char* path, LPTSTR fileName, int replacesMaxCount)
{
	if (replacesMaxCount < 0)
		return -1;

	HANDLE hIn, hOut;
	DWORD readBytesCount, writeBytesCount;
	CHAR buffer[BUF_SIZE] = { 0 };

	hIn = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hIn == INVALID_HANDLE_VALUE)
	{
		AddLogMessage("The input file can't be opened", GetLastError());
		return -2;
	}

	strtok(fileName, ".");
	strcat(fileName, "out.txt");
	AddFileNameToPath(path, fileName);

	hOut = CreateFile(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hOut == INVALID_HANDLE_VALUE)
	{
		AddLogMessage("The output file can't be opened", GetLastError());
		return -3;
	}

	while (ReadFile(hIn, buffer, BUF_SIZE, &readBytesCount, NULL) && readBytesCount > 0)
	{
		char* newStr = CreateNewStringFromBuffer(buffer, replacesMaxCount);
		readBytesCount = strlen(newStr) * sizeof(CHAR);

		WriteFile(hOut, newStr, readBytesCount, &writeBytesCount, NULL);

		if (readBytesCount != writeBytesCount)
		{
			AddLogMessage("Fatal recording error", GetLastError());
			return -1;
		}
	}
	CloseHandle(hIn);
	CloseHandle(hOut);
	return 0;
}

void SetMutexesToDefaultState() {
	WaitForSingleObject(hMutexServer, INFINITE);
	ReleaseMutex(hMutexClient);
}

char* SumStrings(const char* str1, const char* str2)
{
	int length = strlen(str1) + strlen(str2);
	char* newStr = (char*)calloc(length, sizeof(CHAR));
	strcat(newStr, str1);
	strcat(newStr, str2);
	return newStr;
}

void SetError(char* errorBuf, const char* error)
{
	if (strlen(errorBuf) != 0)
		return;

	strcpy(errorBuf, error);
	AddLogMessage(error);
}

int WorkWithClient()
{
	char buffer[BUF_SIZE] = { 0 };
	char errorBuffer[BUF_SIZE] = { 0 };
	replacesCount = 0;

	const char* messagesToClient[2] = { "Send file name", "Send replaces max count" };

	char fileName[BUF_SIZE] = { 0 };
	int replacesMaxCount = 0;
	int iteraction = 0;

	int timeout = INFINITE;

	while (iteraction < 2)
	{
		strcpy((char*)lpFileMap, messagesToClient[iteraction]);
		AddLogMessage(SumStrings("Message sent to client: ", messagesToClient[iteraction]));
		ReleaseMutex(hMutexServer);

		WaitForSingleObject(hMutexClient, timeout);
		strcpy(buffer, (char*)lpFileMap);
		AddLogMessage(SumStrings("Message read from client: ", buffer));

		if (strcmp(buffer, "stop service") == 0)
			return -2;

		if (strlen(buffer) == 0)
			SetError(errorBuffer, "Missed argument");

		if (iteraction == 0)
			strcpy(fileName, buffer);
		else if (iteraction == 1)
		{
			if (!IsDigit(buffer[0]))
				SetError(errorBuffer, "Invalid argument");
			else
				replacesMaxCount = atoi(buffer);
		}

		iteraction++;
		SetMutexesToDefaultState();
	}

	char path[PATH_SIZE] = { 0 };
	AddFileNameToPath(path, fileName);

	if (WorkWithFiles(path, fileName, replacesMaxCount) != 0)
		SetError(errorBuffer, "File can not be opened or file does not exist");

	if (strlen(errorBuffer) > 0)
		strcpy(buffer, errorBuffer);
	else
		_itoa(replacesCount, buffer, 10);

	strcpy((char*)lpFileMap, buffer);
	AddLogMessage(SumStrings("Message with result sent to client: ", buffer));
	ReleaseMutex(hMutexServer);

	CloseHandle(hMutexClient);
	WaitForSingleObject(hMutexServer, INFINITE);
	return (strlen(errorBuffer) > 0 ? -1 : 0);
}

void WaitClient()
{
	hMutexClient = OpenMutex(MUTEX_ALL_ACCESS, false, mutexClientName);

	while (hMutexClient == NULL)
	{
		Sleep(200);
		hMutexClient = OpenMutex(MUTEX_ALL_ACCESS, false, mutexClientName);
	}
}

int ServiceStart()
{
	hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, fileShareName);
	if (hFileMapping == NULL)
	{
		AddLogMessage("Error CreateFileMapping", GetLastError());
		return -1;
	}

	SetSecurityInfo(hFileMapping, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);

	lpFileMap = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, BUF_SIZE);

	if (lpFileMap == 0)
	{
		AddLogMessage("Error MapViewOfFile", GetLastError());
		return -1;
	}

	hMutexServer = CreateMutex(NULL, TRUE, mutexServerName);

	if (hMutexServer == INVALID_HANDLE_VALUE)
		AddLogMessage("Error create mutex", GetLastError());

	SetSecurityInfo(hMutexServer, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);

	return 0;
}

void ServiceStop()
{
	UnmapViewOfFile(lpFileMap);
	FindClose(hFileMapping);
	CloseHandle(hFileMapping);

	FindClose(hMutexServer);
	CloseHandle(hMutexServer);

	AddLogMessage("All kernel objects was closed");
	AddLogMessage("Service stopped!");
}
