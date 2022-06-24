#pragma once

// The Windows API.
#pragma warning( push,3 )

#include <Windows.h>

#pragma warning( pop )


// String functions such as sprintf, etc.
#include <stdio.h>

// Nicer data types, e.g., uint8_t, int32_t, etc.
#include <stdint.h>


#define GAME_NAME		"Mithra_game"

#define GAME_RES_WIDTH	384

#define GAME_RES_HEIGHT	240

#define GAME_BPP		32

#define GAME_DRAWING_AREA_MEMORY_SIZE		(GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))

#define CALCULATE_AVG_FPS_EVERY_X_FRAMES	100

#define TARGET_MICROSECONDS_PER_FRAME		16667 

typedef struct PIXEL32
{
	uint8_t Blue;

	uint8_t Green;

	uint8_t Red;

	uint8_t Alpha;

} PIXEL32;

// GAMEBITMAP is any sort of bitmap, which might be a sprite, or a background, 
// or a font sheet, or even the back buffer itself.
#pragma warning (disable:4820) // disable struct padding warning
#pragma warning (disable:4820) // disable spectre exploit CPU velnerabality
#pragma warning (disable:4710) // inline warnings


typedef struct GAMEBITMAP
{
	BITMAPINFO BitmapInfo;

	void* Memory;

} GAMEBITMAP;

typedef struct GAMEPERFDATA
{
	uint64_t TotalFramesRendered;

	float RawFPSAverage;

	float CookedFPSAverage;

	int64_t PerfFrequency;

	MONITORINFO gMonitorInfo;

	int32_t gMonitorWidth;

	int32_t gMonitorHeight;

}GAMEPERFDATA;


// A global handle to the game window.
extern HWND gGameWindow;

// Set this to FALSE to exit the game immediately. This controls the main game loop in WinMain.
extern BOOL gGameIsRunning;

LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RenderFrameGraphics(void);