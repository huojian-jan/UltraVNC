#include "framework.h"
#include<stdio.h>
#include <stdlib.h>
#include "snake_game.h"

#define MAX_LOADSTRING 100
#define UP 1
#define Right 2
#define Down 3
#define Left 4
#define Step 10;

const int  Width = 50;
const int Height = 50;


// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT Dir = Right;
HANDLE out_handle;
RECT rects[100];
int rect_count = 0;

void WriteTextDirectly(const wchar_t* text) {
	DWORD written;
	WriteConsoleW(out_handle, text, lstrlenW(text), &written, NULL);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	AllocConsole();
	out_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SNAKEGAME, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNAKEGAME));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKEGAME));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SNAKEGAME);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   SetTimer(hWnd, 1, 100, NULL);
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

bool inited = false;


void init_snake(HWND hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	int width = (rc.right - rc.left)/2;
	int height = (rc.bottom - rc.top)/2;

	RECT r = {
		width - Width / 2,
		height - Height / 2,
		width + Width / 2,
		height + Height / 2
	};

	rects[rect_count++] = r;
}

void update_snake(HWND hWnd)
{
	for (int i = 0; i < rect_count; i++)
	{
		int x, y;
		if (Dir == UP)
		{
			x = 0;
			y = -Step;
		}
		else if (Dir == Right)
		{
			x = Step;
			y = 0;
		}
		else if (Dir == Down)
		{
			x = 0;
			y = Step;
		}
		else if (Dir == Left)
		{
			x = -Step;
			y = 0;
		}
		OffsetRect(&rects[i], x, y);
	}
	InvalidateRect(hWnd, NULL, TRUE);
}

void draw_snake(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	HBRUSH brush = CreateSolidBrush(RGB(0, 255, 0));
	for (int i = 0; i < rect_count; i++)
	{
		FillRect(hdc, &rects[i], brush);
	}
	DeleteObject(brush);
	EndPaint(hWnd, &ps);
}

void redraw(HWND hWnd)
{
    if (!inited)
    {
        init_snake(hWnd);
        inited = true;
    }
    draw_snake(hWnd);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        redraw(hWnd);
		//wchar_t data1[256];
		//swprintf_s(data1, 256, L"%d", Dir);
		//WriteTextDirectly(data1);
        break;
    case WM_TIMER:
        update_snake(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYUP:
        switch (wParam)
        {
        case VK_UP:
        case  'W':
            Dir = UP;
            break;
        case VK_LEFT:
        case 'A':
            Dir = Left;
            break;
        case VK_DOWN:
        case 'S':
            Dir = Down;
            break;
        case VK_RIGHT:
        case 'D':
            Dir = Right;
        default:
            break;
        }
        wchar_t data[256];
        swprintf_s(data, 256, L"%d", Dir);
        WriteTextDirectly(data);
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
