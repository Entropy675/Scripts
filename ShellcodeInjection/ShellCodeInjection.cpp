#include <Windows.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	DWORD PID = NULL;

	if (argc < 2)
	{
		printf("Usage: this.exe <PID>\nAttempts to inject into provided PID. This is meme program so it injects garbage and runs it, likely crashing.");
		return 1;
	}

	PID = atoi(argv[1]);
	printf("Attempting to open handle... %ld\n", PID);

	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (handle == NULL)
	{
		printf("Failed to open process. Error code: %ld\n", GetLastError());
		return 1;
	}
	printf("Handle Opened.  %ld\n", PID);
	
	LPVOID baseAddress = VirtualAllocEx(handle, NULL, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (baseAddress == NULL)
	{
		printf("Failed to allocate virtual memory. Error code: %ld\n", GetLastError());
		CloseHandle(handle);
		return 1;
	}

	unsigned char buf[] = " /*some magical code, please don't kill me windows*/ ";
	size_t bufSize = sizeof(buf) - 1;

	size_t bytesWritten;
	if (!WriteProcessMemory(handle, baseAddress, buf, bufSize, &bytesWritten))
	{
		printf("Failed to write process memory. Error code: %ld\n", GetLastError());
		VirtualFreeEx(handle, baseAddress, 0, MEM_RELEASE);
		CloseHandle(handle);
		return 1;
	}

	printf("Successfully wrote %Iu bytes to process memory.\n", bytesWritten);

	DWORD TID;
	HANDLE thread = CreateRemoteThreadEx(handle, NULL, 0, (LPTHREAD_START_ROUTINE)baseAddress, NULL, 0, 0, &TID);
	if (thread == NULL)
	{
		printf("Could not create thread: %ld\n", GetLastError());
		VirtualFreeEx(handle, baseAddress, 0, MEM_RELEASE);
		CloseHandle(handle);
		return 1;
	}

	VirtualFreeEx(handle, baseAddress, 0, MEM_RELEASE);
	CloseHandle(handle);
	CloseHandle(thread);
	printf("~ mission complete ~\n");

	return 0;
}