#include "Main.h"


HWND gGameWindow = NULL;

BOOL gGameIsRunning = TRUE;

GAMEBITMAP gBackBuffer;

GAMEPERFDATA gPerformanceData;


int __stdcall WinMain(_In_ HINSTANCE Instance, _In_opt_ HINSTANCE PreviousInstance, _In_ PSTR CommandLine, _In_ INT CmdShow)
{

	UNREFERENCED_PARAMETER(Instance);

	UNREFERENCED_PARAMETER(PreviousInstance);

	UNREFERENCED_PARAMETER(CommandLine);

	UNREFERENCED_PARAMETER(CmdShow);

	MSG Message = { 0 };

	//start time calculation
	int64_t FrameStart = 0;

	int64_t FrameEnd = 0;

	int64_t ElapsedMicroSecondsPerFrame = 0;

	int64_t ElapsedMicroSecondsPerFrameAccumulatorRaw = 0;

	int64_t ElapsedMicroSecondsPerFrameAccumulatorCooked = 0;
	//end 

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

	QueryPerformanceFrequency(&gPerformanceData.PerfFrequency);

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
		QueryPerformanceCounter(&FrameStart);

		while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE))
		{
			DispatchMessageA(&Message);
		}

		ProcessPlayerInput();

		RenderFrameGraphics();

		QueryPerformanceCounter(&FrameEnd);

		ElapsedMicroSecondsPerFrame = FrameEnd - FrameStart;

		ElapsedMicroSecondsPerFrame *= 1000000;

		ElapsedMicroSecondsPerFrame /= gPerformanceData.PerfFrequency;

		gPerformanceData.TotalFramesRendered++;

		ElapsedMicroSecondsPerFrameAccumulatorRaw += ElapsedMicroSecondsPerFrame;

		while (ElapsedMicroSecondsPerFrame < TARGET_MICROSECONDS_PER_FRAME)
		{
			Sleep(0);

			ElapsedMicroSecondsPerFrame = FrameEnd - FrameStart;

			ElapsedMicroSecondsPerFrame *= 1000000;

			ElapsedMicroSecondsPerFrame /= gPerformanceData.PerfFrequency;

			QueryPerformanceCounter(&FrameEnd);

		}

		ElapsedMicroSecondsPerFrameAccumulatorCooked += ElapsedMicroSecondsPerFrame;

		if (gPerformanceData.TotalFramesRendered % CALCULATE_AVG_FPS_EVERY_X_FRAMES == 0)
		{
			int64_t AverageMicroSecondsPerFrameRaw = ElapsedMicroSecondsPerFrameAccumulatorRaw / CALCULATE_AVG_FPS_EVERY_X_FRAMES;

			int64_t AverageMicroSecondsPerFrameCooked = ElapsedMicroSecondsPerFrameAccumulatorCooked / CALCULATE_AVG_FPS_EVERY_X_FRAMES;

			gPerformanceData.RawFPSAverage = 1.0f / ((ElapsedMicroSecondsPerFrameAccumulatorRaw / 60) * 0.000001f);

			gPerformanceData.CookedFPSAverage = 1.0f / ((ElapsedMicroSecondsPerFrameAccumulatorCooked / 60) * 0.000001f);

			char str[64] = { 0 };

			_snprintf_s(str, _countof(str), _TRUNCATE, " AVG RAW FPS:  %.01f\t AVG COOKED FPS:  %.01f\n", 
				gPerformanceData.RawFPSAverage,
				gPerformanceData.CookedFPSAverage);

			OutputDebugStringA(str);

			ElapsedMicroSecondsPerFrameAccumulatorRaw = 0;

			ElapsedMicroSecondsPerFrameAccumulatorCooked = 0;
		}
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

	gPerformanceData.gMonitorInfo.cbSize = sizeof(MONITORINFO);

	if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gPerformanceData.gMonitorInfo) == 0)
	{
		Result = ERROR_MONITOR_NO_DESCRIPTOR;

		goto Exit;
	}

	gPerformanceData.gMonitorWidth = gPerformanceData.gMonitorInfo.rcMonitor.right - gPerformanceData.gMonitorInfo.rcMonitor.left;

	gPerformanceData.gMonitorHeight = gPerformanceData.gMonitorInfo.rcMonitor.bottom - gPerformanceData.gMonitorInfo.rcMonitor.top;

	if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (SetWindowPos(gGameWindow, HWND_TOP, gPerformanceData.gMonitorInfo.rcMonitor.left, gPerformanceData.gMonitorInfo.rcMonitor.top, gPerformanceData.gMonitorWidth, gPerformanceData.gMonitorHeight, SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

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
	memset(gBackBuffer.Memory, 0xFF, GAME_DRAWING_AREA_MEMORY_SIZE);

	PIXEL32 Pixel = { 0 };

	Pixel.Blue = 0x7F;

	Pixel.Green = 0;

	Pixel.Red = 0;

	Pixel.Alpha = 0xFF;

	for (size_t i = 0; i < GAME_RES_WIDTH * GAME_RES_HEIGHT; i++)
	{
		// # FF 00 00 FF FF 00 00 FF
		memcpy_s(((PIXEL32*)gBackBuffer.Memory + i), sizeof(PIXEL32), &Pixel, sizeof(PIXEL32));
	}

	HDC DeviceContext = GetDC(gGameWindow);

	StretchDIBits(
		DeviceContext,
		0, 0,
		gPerformanceData.gMonitorWidth,
		gPerformanceData.gMonitorHeight,
		0, 0,
		GAME_RES_WIDTH, GAME_RES_HEIGHT,
		gBackBuffer.Memory,
		&gBackBuffer,
		DIB_RGB_COLORS,
		SRCAND);

	ReleaseDC(gGameWindow, DeviceContext);
}