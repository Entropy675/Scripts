#include <Windows.h>
#include <stdio.h>

int main(int argc, char* argv[])
{

	DWORD PID = NULL, TID = NULL;
	LPVOID buffer = NULL;

	HMODULE kernel32 = NULL;
	HANDLE myProc = NULL, myThread = NULL;

	wchar_t dllName[] = L"randomDLL.dll";
	wchar_t dllPath[MAX_PATH];


	if (argc < 2)
	{
		printf("usage: %s", argv[0]);
		return EXIT_FAILURE;
	}

	PID = atoi(argv[1]);

	printf("Trying to get a handle to the process... (%ld)\n", PID);

	myProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (myProc == NULL)
		return EXIT_FAILURE;
	
	printf("Got a handle to the process (%ld)\n*-> 0x%p\n", PID, myProc);

	GetCurrentDirectory(MAX_PATH, dllPath);
	wcscat_s(dllPath, MAX_PATH, L"\\");
	wcscat_s(dllPath, MAX_PATH, dllName);

	size_t dllPathSize = (wcslen(dllPath) + 1) * sizeof(wchar_t);

	buffer = VirtualAllocEx(myProc, NULL, dllPathSize, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
	printf("Got that block of memory with PAGE_READWRITE permissions\n");

	if (buffer == NULL)
	{
		printf("Syke lmao nerd\n");
		CloseHandle(myProc);
		return EXIT_FAILURE;
	}

	// store in process memory that we alloced in the buffer...
	WriteProcessMemory(myProc, buffer, dllPath, dllPathSize, NULL);
	printf("Wrote %S to process memory\n", dllPath);

	// finally we can load the module
	kernel32 = GetModuleHandleW(L"Kernel32.dll"); 

	if (kernel32 == NULL)
	{
		printf("Failed to get handle to Kernel32, err: %ld", GetLastError());
		CloseHandle(myProc);
		return EXIT_FAILURE;
	}

	printf("Got handle! Its -0x%p", kernel32);

	// start routine is the entry point thread, we are loading the LoadLibraryW function
	// that means that the startThis is an entry point straight into the LoadLibraryW function from kernel32
	LPTHREAD_START_ROUTINE startThis = (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32, "LoadLibraryW");

	myThread = CreateRemoteThread(myProc, NULL, 0, startThis, buffer, 0, &TID);

	if (myThread == NULL)
	{
		printf("Failed to get handle to thread, err: %ld", GetLastError());
		CloseHandle(myProc);
		return EXIT_FAILURE;
	}

	printf("Have handle to new thread %ld\n --0xx%p\n", TID, myThread);
	printf("Waiting for thread to close... \n");

	WaitForSingleObject(myThread, INFINITE);

	printf("Thread closed. Clearning up. \n");

	CloseHandle(myThread);
	CloseHandle(myProc);

	printf("It is finished. The DLL Injection has been done...");

	return EXIT_SUCCESS;
}