#include <windows.h>
#include <Lmcons.h>

HANDLE heap;

extern void error_message_box();

enum OutputTarget {
	DebuggerOutput = 0,
	MessageBoxOutput = 1
} OutputTarget;

void display_last_error(enum OutputTarget outputTarget)
{
	wchar_t* messageBuffer;
	DWORD dw = GetLastError();

	// extract message from last error
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&messageBuffer,
		0,
		NULL);

	if (outputTarget == DebuggerOutput) {
		OutputDebugStringW(messageBuffer);
	}
	else if (outputTarget == MessageBoxOutput) {
		error_message_box(messageBuffer);
	}

	LocalFree(messageBuffer);
}

#define USER_NAME_BUFFER_SIZE (UNLEN + 1)

wchar_t* getUserName() {
	ULONG64 userNameSize = USER_NAME_BUFFER_SIZE;
	wchar_t userName[USER_NAME_BUFFER_SIZE];

	if (GetUserNameW(userName, (LPDWORD)&userNameSize) == 0) {
		display_last_error(MessageBoxOutput);
		return NULL;
	}

	wchar_t* userNameCopy = HeapAlloc(heap, 0, (userNameSize * 2) + 1);
	lstrcpyW(userNameCopy, userName);
	return userNameCopy;
}

#define COMPUTER_NAME_BUFFER_SIZE ( MAX_COMPUTERNAME_LENGTH + 1 )

wchar_t* getComputerName() {
	ULONG64 computerNameSize = COMPUTER_NAME_BUFFER_SIZE;
	wchar_t computerName[COMPUTER_NAME_BUFFER_SIZE];

	if (GetComputerNameW(computerName, (LPDWORD)&computerNameSize) == 0) {
		display_last_error(MessageBoxOutput);
		return NULL;
	}

	wchar_t* computerNameCopy = HeapAlloc(heap, 0, (computerNameSize * 2) + 1);
	lstrcpyW(computerNameCopy, computerName);
	return computerNameCopy;
}

int WINAPI WinMainCRTStartup(void) {
	heap = GetProcessHeap(); // for memory allocations
	wchar_t* commandLineStr = GetCommandLineW();
	int argc;
	LPWSTR* args = CommandLineToArgvW(commandLineStr, &argc); // args is array of string with arguments

	for (size_t i = 1; i < argc; i++)
	{
		wchar_t* arg = args[1];
		if (lstrcmpiW(arg, L"-d") == 0) {
			// debug mode
			OutputDebugStringW(L"debug mode active...\r\n");
		}
	}


	// console allocation is necessary, 
	// since we're running in windows subsystem
	BOOL consoleAttached;
	if (AttachConsole(ATTACH_PARENT_PROCESS) == 0) { // attachment to exising console failed

		DWORD lastError = GetLastError();
		// allocate new console, if invalid handle is present, that means current process has no console allocated
		if (lastError == ERROR_INVALID_HANDLE && AllocConsole() == 0) { // allocating console failed
			display_last_error(MessageBoxOutput);
			return 1;
		}
		consoleAttached = FALSE;
	}
	else {
		consoleAttached = TRUE;
	}


	HANDLE stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdOutHandle == NULL) {
		return 1;
	}

	wchar_t* computerName = getComputerName();
	wchar_t* userName = getUserName();

	int messageSize = ((lstrlenW(computerName) + lstrlenW(userName) + 1 /* backslash */) * 2) + 3 /* \r\n\0 */;
	wchar_t* message = HeapAlloc(heap, 0, messageSize);

	lstrcpyW(message, computerName);
	lstrcatW(message, L"\\");
	lstrcatW(message, userName);
	lstrcatW(message, L"\r\n");

	HeapFree(heap, 0, computerName);
	HeapFree(heap, 0, userName);


	LPDWORD charsWritten;
	if (WriteConsoleW(stdOutHandle, message, lstrlenW(message), (LPDWORD)&charsWritten, NULL) == 0) {
		display_last_error(MessageBoxOutput);
	}

	CloseHandle(stdOutHandle);
	HeapFree(heap, 0, message);
	if (consoleAttached)
		FreeConsole();
	return 0;
}