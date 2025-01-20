// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

DWORD Bufferchange(DWORD nIn, CHAR Buffer[], DWORD changeMAX)
{
	DWORD total = 0;
	DWORD j = 0, k = 0;
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
	return total;
}

