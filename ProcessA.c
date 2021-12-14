#include <windows.h>
#include <stdio.h>
#include <TlHelp32.h>

#define MAXLENGTH 101
// функция получения ID процесса
DWORD GetProcessID(char* process) {
	PROCESSENTRY32 processEntry;
	DWORD processID = 0;
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Snapshot != INVALID_HANDLE_VALUE) {
		processEntry.dwSize = sizeof(processEntry);
		Process32First(Snapshot, &processEntry);

		do {
			if (strcmp(processEntry.szExeFile, process) == 0) {
				processID = processEntry.th32ProcessID;
				break;
			}
		} while (Process32Next(Snapshot, &processEntry));

		CloseHandle(Snapshot);
	}

	return processID;
}
int fail()
{
	printf("FAIL!\n");
	system("pause");
	return -1;
}

int main(void)
{
	LPCTSTR pipename = "\\\\.\\pipe\\mynamedpipe";					
	HANDLE hProcessB;												
	LPVOID szAddress;												
	DWORD cbWritten, cbRead, dBuf;									
	BOOL Success;													
	DWORD szSize = sizeof(char) * MAXLENGTH;						
	char *str = (char*)malloc(szSize);								
	HANDLE hPipe = INVALID_HANDLE_VALUE;							

	printf("INPUT STRING: ");
	gets_s(str, MAXLENGTH - 1);

	printf("Opening ProcessB...");
	hProcessB = OpenProcess(
		PROCESS_ALL_ACCESS,											
		FALSE,														
		GetProcessID("ProcessB.exe"));	                            
	if (hProcessB == INVALID_HANDLE_VALUE) return fail();

	printf("\nAllocating memory for ProcessB...");
	szAddress = VirtualAllocEx(
		hProcessB,													
		0,															
		szSize,														
		MEM_RESERVE |												
		MEM_COMMIT,
		PAGE_EXECUTE_READWRITE);									
	
	printf("\nWriting your string in ProcessB memory...");
	Success = WriteProcessMemory(
		hProcessB,													
		szAddress,													
		str,														
		szSize,														
		0);															
	if (Success == FALSE) return fail();

	printf("\nConnecting to named pipe...");
	while (hPipe == INVALID_HANDLE_VALUE) {
		hPipe = CreateFile(
			pipename,												
			GENERIC_READ |											
			GENERIC_WRITE,
			0,														
			NULL,													
			OPEN_EXISTING,											
			0,														
			NULL);													
	}

	printf("\nWriting memory address...");
	dBuf = (DWORD)szAddress;
	Success = WriteFile(
		hPipe,														
		&dBuf,														
		sizeof(dBuf),												
		&cbWritten,													
		NULL);														
	if (Success == FALSE) return fail();
	printf("\nWaiting answer from ProcessB...");
	Success = ReadFile(
		hPipe,														
		&szAddress,													
		sizeof(szAddress),											
		&cbRead,													
		NULL);														
	if (Success == FALSE) return fail();

	printf("\nReleasing ProcessB memory...");
	Success = VirtualFreeEx(
		hProcessB,
		szAddress,
		0,
		MEM_RELEASE);
	if (Success == FALSE) return fail();

	printf("\nClosing named pipe...");
	Success = CloseHandle(hPipe);
	if (Success == FALSE) return fail();

	printf("\nClosing ProcessB...");
	Success = CloseHandle(hProcessB);
	if (Success == FALSE) return fail();

	printf("\nProcessA was finished...\n");
	system("pause");
	return 0;
}
