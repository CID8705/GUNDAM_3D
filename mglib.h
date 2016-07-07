/******************************************************************************/
/*
Copyright (c) 2012, Chikara Nakagawa
*/
/*	Note: this header file is coded under the policy that a programming
beginner can easily create his/her own graphical program in C or C++.
Then all functions and global variables are written in this file,
which should be avoided in many other cases.
*/
/******************************************************************************/
#include <windows.h>
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#define mgRED 255, 0, 0
#define mgGREEN 0, 255, 0
#define mgBLUE 0, 0, 255
#define mgYELLOW 255, 255, 0
#define mgPURPLE 255, 0, 255
#define mgAQUA 0, 255, 255
#define mgWHITE 255, 255, 255
#define mgBLACK 0, 0, 0
#define mgGRAY 127, 127, 127
#define mgPI 3.14159265359
#define mgRtoD 180.0 / 3.14159265359
#define mgDtoR 3.14159265359 / 180.0
#define mgMESSAGE() MessageBoxA(NULL, "mgOpenWindow関数を先に実行して下さい！", "mgLIB", MB_OK)
#define mgOK 1
#define mgCAN 2
#define mgYES 6
#define mgNO 7
#define mgMboxOk(s) MessageBox(NULL, s, "mgLIB", MB_OK)
#define mgMboxOkCan(s) MessageBox(NULL, s, "mgLIB", MB_OKCANCEL)
#define mgMboxYesNo(s) MessageBox(NULL, s, "mgLIB", MB_YESNO)
#define mgMboxYesNoCan(s) MessageBox(NULL, s, "mgLIB", MB_YESNOCANCEL)
/******************************************************************************/
static HBITMAP hBitmap = NULL;
static HDC hBuffer = NULL;
static HWND ghwnd = NULL;
static HPEN dPen = NULL;
static HBRUSH dBrush = NULL;
static SIZE gws;
static CRITICAL_SECTION mgCritical_Section;
static HANDLE mgMain_Handle;
static int mgKey = -1, mgMouse_Val = -1;
static POINT mgMouse_Pos;
static int mgCreate_VAL = NULL;
static int mgLine_Width = NULL;
static int mgLine_Style = PS_SOLID;
static int mgLine_RED = NULL;
static int mgLine_GRN = NULL;
static int mgLine_BLU = NULL;
static int mgFill_RED = 255;
static int mgFill_GRN = 255;
static int mgFill_BLU = 255;
static int mgOffset_X = NULL;
static int mgOffset_Y = NULL;
/******************************************************************************/
inline int GMouse(int *x, int *y) {
	int ret = -1;
	EnterCriticalSection(&mgCritical_Section);
	ret = mgMouse_Val;
	//	mgMouse_Val = -1;
	*x = mgMouse_Pos.x - mgOffset_X;
	*y = gws.cy - mgMouse_Pos.y - mgOffset_Y;
	LeaveCriticalSection(&mgCritical_Section);
	return ret;
}
/******************************************************************************/
static void at_exit() {
	/* Wait for the window to be closed */
	if (hBitmap != NULL)
		SuspendThread(mgMain_Handle);
}
/******************************************************************************/
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	HDC hdc;
	PAINTSTRUCT ps;
	// static HBITMAP hBitmap;
	switch (msg) {
	case WM_CREATE:
		hdc = GetDC(hwnd);
		hBitmap = CreateCompatibleBitmap(hdc, gws.cx, gws.cy);
		hBuffer = CreateCompatibleDC(hdc);
		dPen = CreatePen(PS_SOLID, mgLine_Width, RGB(0, 0, 0));
		dBrush = CreateSolidBrush(RGB(0xff, 0xff, 0xff));
		SelectObject(hBuffer, hBitmap);
		SelectObject(hBuffer, GetStockObject(NULL_PEN));
		PatBlt(hBuffer, 0, 0, gws.cx, gws.cy, WHITENESS);
		ReleaseDC(hwnd, hdc);
		return NULL;
	case WM_CHAR:
		EnterCriticalSection(&mgCritical_Section);
		mgKey = (TCHAR)wp;
		LeaveCriticalSection(&mgCritical_Section);
		return NULL;
	case WM_DESTROY:
		DeleteDC(hBuffer);
		DeleteObject(hBitmap);
		DeleteObject(dPen);
		DeleteObject(dBrush);
		hBitmap = NULL;
		PostQuitMessage(NULL);
		ResumeThread(mgMain_Handle);
		return NULL;
	case WM_LBUTTONDOWN:
		EnterCriticalSection(&mgCritical_Section);
		mgMouse_Val = 0;
		mgMouse_Pos.x = LOWORD(lp);
		mgMouse_Pos.y = HIWORD(lp);
		LeaveCriticalSection(&mgCritical_Section);
		return NULL;
	case WM_PAINT:
		EnterCriticalSection(&mgCritical_Section);
		hdc = BeginPaint(hwnd, &ps);
		BitBlt(hdc, 0, 0, gws.cx, gws.cy, hBuffer, 0, 0, SRCCOPY);
		EndPaint(hwnd, &ps);
		LeaveCriticalSection(&mgCritical_Section);
		return NULL;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}
/******************************************************************************/
static DWORD WINAPI ThreadFunc(LPVOID Param) {
	WNDCLASS winc;
	MSG msg;
	LPSIZE ws = (SIZE *)Param;
	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc = WndProc;
	winc.cbClsExtra = winc.cbWndExtra = 0;
	winc.hInstance = NULL /*hInstance*/;
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winc.lpszMenuName = NULL;
	winc.lpszClassName = TEXT("mgLIB");
	if (!RegisterClass(&winc))
		return NULL;
	ghwnd = CreateWindow(TEXT("mgLIB"), TEXT("mgLIB window"), (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, ws->cx + GetSystemMetrics(SM_CXFIXEDFRAME) * 2, ws->cy + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION), NULL, NULL, NULL /* hInstance */, NULL);
	/* Resume main thread */
	ResumeThread(mgMain_Handle);
	if (ghwnd == NULL)
		return 1;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return NULL;
}
/******************************************************************************/
inline int mgOpenWindow(int width, int height) {
	DWORD threadID;
	if (mgCreate_VAL == NULL)
		mgCreate_VAL = 1;
	else {
		MessageBoxA(NULL, "mgOpenWindow関数は1度しか実行できません!", "mgLIB", MB_OK);
		return NULL;
	}
	gws.cx = width;
	gws.cy = height;
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mgMain_Handle, DUPLICATE_SAME_ACCESS, FALSE, 0);
	InitializeCriticalSection(&mgCritical_Section);
	CreateThread(NULL, 0, ThreadFunc, (LPVOID)&gws, 0, &threadID);
	/* Wait for ThreadFunc to finish CreateWindow() */
	SuspendThread(mgMain_Handle);
	atexit(at_exit);
	return NULL;
}
/******************************************************************************/
void mgLine(int x1, int y1, int x2, int y2) {
	HPEN hp;
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	x1 += mgOffset_X;
	y1 = gws.cy - y1 - mgOffset_Y;
	x2 += mgOffset_X;
	y2 = gws.cy - y2 - mgOffset_Y;
	EnterCriticalSection(&mgCritical_Section);
	hp = CreatePen(mgLine_Style, mgLine_Width, RGB(mgLine_RED, mgLine_GRN, mgLine_BLU));
	SelectObject(hBuffer, hp);
	MoveToEx(hBuffer, x1, y1, NULL);
	LineTo(hBuffer, x2, y2);
	InvalidateRect(ghwnd, NULL, FALSE);
	DeleteObject(hp);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgRectangle(int x1, int y1, int x2, int y2) {
	HPEN hp;
	HBRUSH hb = NULL;
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	x1 += mgOffset_X;
	y1 = gws.cy - y1 - mgOffset_Y;
	x2 += mgOffset_X;
	y2 = gws.cy - y2 - mgOffset_Y;
	EnterCriticalSection(&mgCritical_Section);
	hp = CreatePen(mgLine_Style, mgLine_Width, RGB(mgLine_RED, mgLine_GRN, mgLine_BLU));
	SelectObject(hBuffer, hp);
	SelectObject(hBuffer, GetStockObject(NULL_BRUSH));
	Rectangle(hBuffer, x1, y1, x2, y2);
	InvalidateRect(ghwnd, NULL, FALSE);
	DeleteObject(hp);
	if (hb != NULL)
		DeleteObject(hb);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgRectangleF(int x1, int y1, int x2, int y2) {
	HPEN hp;
	HBRUSH hb = NULL;
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	x1 += mgOffset_X;
	y1 = gws.cy - y1 - mgOffset_Y;
	x2 += mgOffset_X;
	y2 = gws.cy - y2 - mgOffset_Y;
	EnterCriticalSection(&mgCritical_Section);
	hp = CreatePen(PS_SOLID, NULL, RGB(mgFill_RED, mgFill_GRN, mgFill_BLU));
	SelectObject(hBuffer, hp);
	hb = CreateSolidBrush(RGB(mgFill_RED, mgFill_GRN, mgFill_BLU));
	SelectObject(hBuffer, hb);
	Rectangle(hBuffer, x1, y1, x2, y2);
	InvalidateRect(ghwnd, NULL, FALSE);
	DeleteObject(hp);
	if (hb != NULL)
		DeleteObject(hb);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgEllipse(int x1, int y1, int x2, int y2) {
	HPEN hp;
	HBRUSH hb = NULL;
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	x1 += mgOffset_X;
	y1 = gws.cy - y1 - mgOffset_Y;
	x2 += mgOffset_X;
	y2 = gws.cy - y2 - mgOffset_Y;
	EnterCriticalSection(&mgCritical_Section);
	hp = CreatePen(mgLine_Style, mgLine_Width, RGB(mgLine_RED, mgLine_GRN, mgLine_BLU));
	SelectObject(hBuffer, hp);
	SelectObject(hBuffer, GetStockObject(NULL_BRUSH));
	Ellipse(hBuffer, x1, y1, x2, y2);
	InvalidateRect(ghwnd, NULL, FALSE);
	DeleteObject(hp);
	if (hb != NULL)
		DeleteObject(hb);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgEllipseF(int x1, int y1, int x2, int y2) {
	HPEN hp;
	HBRUSH hb = NULL;
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	x1 += mgOffset_X;
	y1 = gws.cy - y1 - mgOffset_Y;
	x2 += mgOffset_X;
	y2 = gws.cy - y2 - mgOffset_Y;
	EnterCriticalSection(&mgCritical_Section);
	hp = CreatePen(PS_SOLID, NULL, RGB(mgFill_RED, mgFill_GRN, mgFill_BLU));
	SelectObject(hBuffer, hp);
	hb = CreateSolidBrush(RGB(mgFill_RED, mgFill_GRN, mgFill_BLU));
	SelectObject(hBuffer, hb);
	Ellipse(hBuffer, x1, y1, x2, y2);
	InvalidateRect(ghwnd, NULL, FALSE);
	DeleteObject(hp);
	if (hb != NULL)
		DeleteObject(hb);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgPixel(int x, int y) {
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	x += mgOffset_X;
	y = gws.cy - y - mgOffset_Y;
	EnterCriticalSection(&mgCritical_Section);
	SetPixel(hBuffer, x, y, RGB(mgLine_RED, mgLine_GRN, mgLine_BLU));
	InvalidateRect(ghwnd, NULL, FALSE);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgArc(int x1, int y1, int x2, int y2, int ax1, int ay1, int ax2, int ay2) {
	HPEN hp;
	HBRUSH hb = NULL;
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	x1 += mgOffset_X;
	y1 = gws.cy - y1 - mgOffset_Y;
	x2 += mgOffset_X;
	y2 = gws.cy - y2 - mgOffset_Y;
	ax1 += mgOffset_X;
	ay1 = gws.cy - ay1 - mgOffset_Y;
	ax2 += mgOffset_X;
	ay2 = gws.cy - ay2 - mgOffset_Y;
	EnterCriticalSection(&mgCritical_Section);
	hp = CreatePen(mgLine_Style, mgLine_Width, RGB(mgLine_RED, mgLine_GRN, mgLine_BLU));
	SelectObject(hBuffer, hp);
	SelectObject(hBuffer, GetStockObject(NULL_BRUSH));
	Arc(hBuffer, x1, y1, x2, y2, ax1, ay1, ax2, ay2);
	InvalidateRect(ghwnd, NULL, FALSE);
	DeleteObject(hp);
	if (hb != NULL)
		DeleteObject(hb);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgArcF(int x1, int y1, int x2, int y2, int ax1, int ay1, int ax2, int ay2) {
	HPEN hp;
	HBRUSH hb = NULL;
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	x1 += mgOffset_X;
	y1 = gws.cy - y1 - mgOffset_Y;
	x2 += mgOffset_X;
	y2 = gws.cy - y2 - mgOffset_Y;
	ax1 += mgOffset_X;
	ay1 = gws.cy - ay1 - mgOffset_Y;
	ax2 += mgOffset_X;
	ay2 = gws.cy - ay2 - mgOffset_Y;
	EnterCriticalSection(&mgCritical_Section);
	hp = CreatePen(PS_SOLID, NULL, RGB(mgFill_RED, mgFill_GRN, mgFill_BLU));
	SelectObject(hBuffer, hp);
	hb = CreateSolidBrush(RGB(mgFill_RED, mgFill_GRN, mgFill_BLU));
	SelectObject(hBuffer, hb);
	Pie(hBuffer, x1, y1, x2, y2, ax1, ay1, ax2, ay2);
	InvalidateRect(ghwnd, NULL, FALSE);
	DeleteObject(hp);
	if (hb != NULL)
		DeleteObject(hb);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgText(int x, int y, const char *str) {
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	x += mgOffset_X;
	y = gws.cy - y - mgOffset_Y;
	EnterCriticalSection(&mgCritical_Section);
	SetTextColor(hBuffer, RGB(mgLine_RED, mgLine_GRN, mgLine_BLU));
	SetBkColor(hBuffer, RGB(mgFill_RED, mgFill_GRN, mgFill_BLU));
	TextOut(hBuffer, x, y, str, strlen(str));
	InvalidateRect(ghwnd, NULL, FALSE);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgFill(int x, int y) {
	HBRUSH hb;
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	x += mgOffset_X;
	y = gws.cy - y - mgOffset_Y;
	EnterCriticalSection(&mgCritical_Section);
	hb = CreateSolidBrush(RGB(mgFill_RED, mgFill_GRN, mgFill_BLU));
	SelectObject(hBuffer, hb);
	FloodFill(hBuffer, x, y, RGB(mgLine_RED, mgLine_GRN, mgLine_BLU));
	InvalidateRect(ghwnd, NULL, FALSE);
	DeleteObject(hb);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgClear(void) {
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	EnterCriticalSection(&mgCritical_Section);
	SelectObject(hBuffer, GetStockObject(WHITE_PEN));
	SelectObject(hBuffer, GetStockObject(WHITE_BRUSH));
	Rectangle(hBuffer, 0, 0, gws.cx, gws.cy);
	InvalidateRect(ghwnd, NULL, FALSE);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgBackColor(void) {
	HPEN hp;
	HBRUSH hb = NULL;
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	EnterCriticalSection(&mgCritical_Section);
	hp = CreatePen(PS_SOLID, NULL, RGB(mgFill_RED, mgFill_GRN, mgFill_BLU));
	SelectObject(hBuffer, hp);
	hb = CreateSolidBrush(RGB(mgFill_RED, mgFill_GRN, mgFill_BLU));
	SelectObject(hBuffer, hb);
	Rectangle(hBuffer, 0, 0, gws.cx, gws.cy);
	InvalidateRect(ghwnd, NULL, FALSE);
	DeleteObject(hp);
	if (hb != NULL)
		DeleteObject(hb);
	LeaveCriticalSection(&mgCritical_Section);
}
/******************************************************************************/
void mgSetLineWidth(int w) {
	mgLine_Width = w;
}
/******************************************************************************/
void mgSetOffset(int x, int y) {
	mgOffset_X = x;
	mgOffset_Y = y;
}
/******************************************************************************/
void mgSetOffsetCenter() {
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	mgOffset_X = gws.cx / 2;
	mgOffset_Y = gws.cy / 2;
}
/******************************************************************************/
void mgSetLineColor(int R, int G, int B) {
	mgLine_RED = R;
	mgLine_GRN = G;
	mgLine_BLU = B;
	if (R > 255)
		mgLine_RED = 255;
	if (G > 255)
		mgLine_GRN = 255;
	if (B > 255)
		mgLine_BLU = 255;
	if (R < 0)
		mgLine_RED = 0;
	if (G < 0)
		mgLine_GRN = 0;
	if (B < 0)
		mgLine_BLU = 0;
}
/******************************************************************************/
void mgSetFillColor(int R, int G, int B) {
	mgFill_RED = R;
	mgFill_GRN = G;
	mgFill_BLU = B;
	if (R > 255)
		mgFill_RED = 255;
	if (G > 255)
		mgFill_GRN = 255;
	if (B > 255)
		mgFill_BLU = 255;
	if (R < 0)
		mgFill_RED = 0;
	if (G < 0)
		mgFill_GRN = 0;
	if (B < 0)
		mgFill_BLU = 0;
}
/******************************************************************************/
void mgSetLineStyle(int s) {
	mgLine_Style = s;
}
/******************************************************************************/
static void mgSaveBMP(const char *FileName) {
	BITMAPFILEHEADER BMPFileHeader;
	BITMAPINFO bi;
	DIBSECTION DIB;
	DWORD Work;
	HANDLE hFile;
	HBITMAP hbm;
	HDC hdc;
	HGDIOBJ hbmOld;
	int Colors = 0;
	RGBQUAD *RGBQuad;
	HDC hMem, hDC;
	VOID *pvBits;
	if (mgCreate_VAL == NULL) {
		mgMESSAGE();
		exit(NULL);
	}
	// Open file
	if ((hFile = CreateFileA(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL)) == NULL)
		return;
	// ------ Copy bitmap to DIB
	hdc = CreateCompatibleDC(hBuffer);
	ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = gws.cx;
	bi.bmiHeader.biHeight = gws.cy;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 24;
	// Create DIB section as bitmap header claims
	hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &pvBits, NULL, 0);
	// Select object
	hbmOld = SelectObject(hdc, hbm);
	// Copy from hBuffer to hdc (DIB)
	BitBlt(hdc, 0, 0, gws.cx, gws.cy, hBuffer, 0, 0, SRCCOPY);
	// get DIB
	GetObject(hbm, sizeof(DIBSECTION), &DIB);
	// Calculate size of color pallete
	if (DIB.dsBmih.biClrUsed == 0) {
		Colors = 1 << DIB.dsBmih.biClrUsed;
		switch (DIB.dsBmih.biBitCount) {
		case 1:
			Colors = 2;
			break;
		case 4:
			Colors = 16;
			break;
		case 8:
			Colors = 256;
			break;
		}
	}
	else {
		switch (DIB.dsBmih.biBitCount) {
		case 1:
			Colors = DIB.dsBmih.biClrUsed;
			break;
		case 4:
			Colors = DIB.dsBmih.biClrUsed;
			break;
		case 8:
			Colors = DIB.dsBmih.biClrUsed;
			break;
		}
	}
	// set BITMAPFILEHEADER
	ZeroMemory(&BMPFileHeader, sizeof(BITMAPFILEHEADER));
	BMPFileHeader.bfType = 0x4d42;
	BMPFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + Colors*sizeof(RGBQUAD);
	BMPFileHeader.bfSize = BMPFileHeader.bfOffBits + DIB.dsBmih.biSizeImage;
	// write BITMAPFILEHEADER
	WriteFile(hFile, &BMPFileHeader, sizeof(BITMAPFILEHEADER), &Work, NULL);
	// write BITMAPINFOHEADER
	WriteFile(hFile, &(DIB.dsBmih), sizeof(BITMAPINFOHEADER), &Work, NULL);
	// write color pallete
	if (Colors != 0) {
		hDC = GetDC(0);
		hMem = CreateCompatibleDC(hDC);
		SelectObject(hMem, hbm);
		RGBQuad = (RGBQUAD *)malloc(Colors*sizeof(RGBQUAD));
		GetDIBColorTable(hMem, 0, Colors, RGBQuad);
		WriteFile(hFile, RGBQuad, Colors*sizeof(RGBQUAD), &Work, NULL);
		free(RGBQuad);
		DeleteDC(hMem);
		ReleaseDC(0, hDC);
	}
	// write image section (DIB)
	WriteFile(hFile, DIB.dsBm.bmBits, DIB.dsBmih.biSizeImage, &Work, NULL);
	// close file
	CloseHandle(hFile);
	// Deselect hbm
	SelectObject(hdc, hbmOld);
	// Delete hbm
	DeleteObject(hbm);
	// Then, delete hdc
	DeleteDC(hdc);
}
/******************************************************************************/