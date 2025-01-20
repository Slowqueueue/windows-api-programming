#include <windows.h> 
#include <stdio.h> 
#include <cstdlib>
#include <windows.h> 
#include <aclapi.h>

int AddLogMessage(const char* message, int errorCode = 0);

int WorkWithClient();

void WaitClient();

int ServiceStart();

void ServiceStop();

#define SERVICE_NAME "cprogRGZ26service"

SERVICE_STATUS        serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle;

void ReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	if (dwCurrentState == SERVICE_START_PENDING)
		serviceStatus.dwControlsAccepted = 0;
	else
		serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	serviceStatus.dwCurrentState = dwCurrentState;
	serviceStatus.dwWin32ExitCode = dwWin32ExitCode;
	serviceStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_RUNNING ||
		dwCurrentState == SERVICE_STOPPED)
		serviceStatus.dwCheckPoint = 0;
	else
		serviceStatus.dwCheckPoint = dwCheckPoint++;

	SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

void WINAPI ServiceControl(DWORD dwControlCode)
{
	if (dwControlCode == SERVICE_CONTROL_STOP)
	{
		ReportStatus(SERVICE_STOP_PENDING, NOERROR, 0);
		AddLogMessage("Service is stopping...");
		ServiceStop();
		ReportStatus(SERVICE_STOPPED, NOERROR, 0);
	}
	else
	{
		ReportStatus(serviceStatus.dwCurrentState, NOERROR, 0);
	}
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	serviceStatusHandle = RegisterServiceCtrlHandler((LPTSTR)SERVICE_NAME, (LPHANDLER_FUNCTION)ServiceControl);
	if (!serviceStatusHandle)
	{
		AddLogMessage("Error RegisterServiceCtrlHandler", GetLastError());
		return;
	}

	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwServiceSpecificExitCode = 0;
	ReportStatus(SERVICE_START_PENDING, NO_ERROR, 30000);
	AddLogMessage("Service is starting...");

	if (ServiceStart() != 0)
		ServiceControl(SERVICE_CONTROL_STOP);

	AddLogMessage("Service started");
	ReportStatus(SERVICE_RUNNING, NOERROR, 0);

	while (TRUE && serviceStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		AddLogMessage("Waiting for client...");

		WaitClient();
		int code = WorkWithClient();

		if (serviceStatus.dwCurrentState == SERVICE_STOP_PENDING)
			continue;

		if (code == 0)
			AddLogMessage("Successful work with client");
		else if (code == -1)
			AddLogMessage("Unsuccessful work with client");
		else if (code == -2)
			ServiceControl(SERVICE_CONTROL_STOP);

		Sleep(1000);
	}
}


int main(int argc, LPTSTR argv[])
{
	SERVICE_TABLE_ENTRY DispatcherTable[] =
	{
		{(LPTSTR)SERVICE_NAME,(LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL,NULL}
	};

	if (StartServiceCtrlDispatcher(DispatcherTable) == FALSE)
		AddLogMessage("Error start service control dispatcher", GetLastError());
}