#include <windows.h>
#include <thread>
#include <chrono>

#include"window_util.h"
#include "render.h"

bool isDone = 0;

HWND hwnd;
PAINTSTRUCT ps;
HDC hdc;
std::thread *Threadupdate;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Update();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"J3ossGL";
	WNDCLASS wc = { };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	// Create the window.
	hwnd = CreateWindowEx(
		0,								// Optional window styles.
		CLASS_NAME,                     // Window class
		L"J3ossGL",						// Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		0, 0, width, height,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	ShowWindow(hwnd, nCmdShow);

	hdc = BeginPaint(hwnd, &ps);

	std::thread Thread(Update);
	Threadupdate = &Thread;

	// Run the message loop.
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
			isDone = 1;
			Threadupdate->join();
			PostQuitMessage(0);
			return 0;

		case WM_PAINT:
		{
			FillRect(BeginPaint(hwnd, &ps), &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
			EndPaint(hwnd, &ps);
			return 0;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Update()
{
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(bmi));
	BITMAPINFOHEADER& h = bmi.bmiHeader;
	h.biSize = sizeof(BITMAPINFOHEADER);
	h.biWidth = width;
	h.biHeight = -height;
	h.biPlanes = 1;
	h.biBitCount = 32;
	h.biCompression = BI_RGB;
	h.biSizeImage = width * height;

	while (!isDone)
	{
		auto start = std::chrono::high_resolution_clock::now();


		SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, render(), &bmi, DIB_RGB_COLORS);

		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	}
}