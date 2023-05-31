#include <Windows.h>


BOOL WINAPI DllMain(
    HINSTANCE moduleHandle,  // handle to DLL module
    DWORD reason,     // reason for calling function
    LPVOID lpvReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        // runs whenever the DLL is loaded (ex LoadLibrary) in a process
        // Lets pop a message box
        MessageBoxW(NULL, L"SUP MOFOS", L"I DO BE CHILLIN HERE", MB_ICONQUESTION | MB_OK);

        break;

    case DLL_THREAD_ATTACH:
        // runs whenever the DLL is loaded in a thread

        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.

        break;

    case DLL_PROCESS_DETACH:

        if (lpvReserved != nullptr)
        {
            break; // do not do cleanup if process termination scenario
        }

        // Perform any necessary cleanup.
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}