#include <windows.h>
#include <stdio.h>

int fail()
{
	printf("FAIL!\n");
	system("pause");
	return -1;
}
int main(void) {
	LPCTSTR pipename = "\\\\.\\pipe\\mynamedpipe";					
	HANDLE hNamedPipe;												
	DWORD szAddress;												
	DWORD cbRead, cbWritten;										
	char* msg;														
	BOOL Success;													

	printf("Creating pipe...");
	hNamedPipe = CreateNamedPipe(
		pipename,													
		PIPE_ACCESS_DUPLEX,											
		PIPE_TYPE_MESSAGE |											
		PIPE_READMODE_MESSAGE |
		PIPE_WAIT,													
		PIPE_UNLIMITED_INSTANCES,									
		sizeof(DWORD),												
		sizeof(DWORD),												
		100,														
		NULL);														
	if (hNamedPipe == INVALID_HANDLE_VALUE) return fail();

	printf("\nConnecting to pipe...");
	ConnectNamedPipe(hNamedPipe, NULL);

	printf("\nReading from pipe...");
	Success = ReadFile(
		hNamedPipe,													
		&szAddress,													
		sizeof(szAddress),											
		&cbRead,													
		NULL);														
	if (Success == FALSE) return fail();

	printf("\nYOUR STRING: ");
	msg = (char*)szAddress;
	printf_s("%s\n", msg);

	/////////////////////////////////////////////////////////////////

	printf("Sending answer to ProccessA...");
	Success = WriteFile(
		hNamedPipe,													
		&szAddress,													
		sizeof(szAddress),											
		&cbWritten,													
		NULL);														
	if (Success == FALSE) return fail();
	//printf(msg);
	printf("\nDisconnecting from named pipe...");
	Success = DisconnectNamedPipe(hNamedPipe);
	if (Success == FALSE) return fail();

	printf("\nClosing named pipe...");
	Success = CloseHandle(hNamedPipe);
	if (Success == FALSE) return fail();

	printf("\nProcessB was finished...\n");
	
	system("pause");
	return 0;
}