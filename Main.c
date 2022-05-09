#include "Main.h"


HWND gGameWindow = NULL;

BOOL gGameIsRunning = TRUE;

GAMEBITMAP gBackBuffer;

MONITORINFO gMonitorInfo = {sizeof(MONITORINFO)};


int __stdcall WinMain(_In_ HINSTANCE Instance, _In_opt_ HINSTANCE PreviousInstance, _In_ PSTR CommandLine, _In_ INT CmdShow)
{

	UNREFERENCED_PARAMETER(Instance);

	UNREFERENCED_PARAMETER(PreviousInstance);

	UNREFERENCED_PARAMETER(CommandLine);

	UNREFERENCED_PARAMETER(CmdShow);

	MSG Message = { 0 };


	if (GameIsAlreadyRunning() == TRUE)
	{
		MessageBoxA(NULL, "Another instance of this program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);

		goto Exit;
	}


	if (CreateMainGameWindow() != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Failed to create game window!", "Error!", MB_ICONERROR | MB_OK);

		goto Exit;
	}

	gBackBuffer.BitmapInfo.bmiHeader.biSize = sizeof(gBackBuffer.BitmapInfo.bmiHeader);

	gBackBuffer.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;

	gBackBuffer.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;

	gBackBuffer.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;

	gBackBuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;

	gBackBuffer.BitmapInfo.bmiHeader.biPlanes = 1;

	gBackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);


	if (gBackBuffer.Memory == NULL)
	{
		MessageBoxA(NULL, "Failed to allocate memory for drawing surface!", "Error!", MB_ICONERROR | MB_OK);

		goto Exit;
	}
	memset(gBackBuffer.Memory, 0x7F, GAME_DRAWING_AREA_MEMORY_SIZE);

	while (gGameIsRunning == TRUE)
	{

		while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE))
		{
			DispatchMessageA(&Message);
		}

		ProcessPlayerInput();

		RenderFrameGraphics();

		Sleep(1);
	}

Exit:

	return (0);

}



LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam)
{
	LRESULT Result = 0;

	switch (Message)
	{
	case WM_CLOSE:
	{
		gGameIsRunning = FALSE;

		PostQuitMessage(0);

		break;
	}
	default:
	{
		Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
	}
	}

	return(Result);
}

DWORD CreateMainGameWindow(void)
{
	DWORD Result = ERROR_SUCCESS;

	WNDCLASSEXA WindowClass = { 0 };

	WindowClass.cbSize = sizeof(WNDCLASSEXA);

	WindowClass.style = 0;

	WindowClass.lpfnWndProc = MainWindowProc;

	WindowClass.cbClsExtra = 0;

	WindowClass.cbWndExtra = 0;

	WindowClass.hInstance = GetModuleHandleA(NULL);

	WindowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION);

	WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);

	WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);

	WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));

	WindowClass.lpszMenuName = NULL;

	WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";

	if (RegisterClassExA(&WindowClass) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

	gGameWindow = CreateWindowExA(0,
		WindowClass.lpszClassName,
		GAME_NAME,
		WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		640,
		480,
		NULL,
		NULL,
		GetModuleHandleA(NULL),
		NULL);

	if (gGameWindow == NULL)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gMonitorInfo) == 0)
	{
		Result = ERROR_MONITOR_NO_DESCRIPTOR;

		goto Exit;
	}

	int MonitorWidth = gMonitorInfo.rcMonitor.right - gMonitorInfo.rcMonitor.left;

	int MonitorHeight = gMonitorInfo.rcMonitor.bottom - gMonitorInfo.rcMonitor.top;

Exit:

	return(Result);
}

BOOL GameIsAlreadyRunning(void)
{
	HANDLE Mutex = NULL;

	Mutex = CreateMutexA(NULL, FALSE, GAME_NAME "_GameMutex");

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

void ProcessPlayerInput(void)
{
	short EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

	if (EscapeKeyIsDown)
	{
		SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
	}
}

void RenderFrameGraphics(void)
{
	HDC DeviceContext = GetDC(gGameWindow);

	StretchDIBits(DeviceContext, 0, 0, 100, 100, 0, 0, 100, 100, gBackBuffer.Memory, &gBackBuffer, DIB_RGB_COLORS, SRCAND);

	ReleaseDC(gGameWindow, DeviceContext);
	;
}