#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <CommCtrl.h>
#include "resource.h"
#include <time.h>
#include <math.h>
#include <Digitalv.h>

UINT wDeviceID = 0;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PauseChildProc(HWND, UINT, WPARAM, LPARAM);

HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("똥피하기");

void DoubleBuffering(void);
void Makeddong(void);
void SetStatusText(HWND hWnd);
void TransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask);


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
    , LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG Message;
    WNDCLASS WndClass;
    g_hInst = hInstance;

    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = WndProc;
    WndClass.lpszClassName = lpszClass;
    WndClass.lpszMenuName = NULL;
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClass(&WndClass);

    WndClass.lpfnWndProc = PauseChildProc;
    WndClass.lpszClassName = TEXT("PauseChild");
    WndClass.lpszMenuName = NULL;
    WndClass.style = CS_SAVEBITS;
    RegisterClass(&WndClass);

    hWnd = CreateWindow(lpszClass, lpszClass, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 250, 400,
        NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);

    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return (int)Message.wParam;
}
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
    HDC MemDC;
    HBITMAP OldBitmap;
    int bx, by;
    BITMAP bit;

    MemDC = CreateCompatibleDC(hdc);
    OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

    GetObject(hBit, sizeof(BITMAP), &bit);
    bx = bit.bmWidth;
    by = bit.bmHeight;

    BitBlt(hdc, x, y, bx, by, MemDC, 0, 0, SRCCOPY);

    SelectObject(MemDC, OldBitmap);
    DeleteDC(MemDC);
}
typedef struct _ddong {
    int x;
    int y;
    int speed;
    bool be;
} ddong;
ddong ass[100];

HBITMAP hBufBit;
HBITMAP hBit[11];
int nx = 110;
int c = 0;
int count = 0;
bool MoveL = FALSE;
bool MoveR = FALSE;
int Score = 0;
int Time = 0;
int RankScore = 0;
bool isGameRunning = FALSE;
bool bPause = FALSE;
bool bFade = TRUE;
bool bdead = FALSE;

HWND hState;
HWND hPauseChild;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    int SBPart[3];

    switch (iMessage) {
    case WM_CREATE:
        hWndMain = hWnd;
        RankScore = GetPrivateProfileInt(TEXT("RankScore"), TEXT("Score"), 0, TEXT("Score.ini"));

        hPauseChild = CreateWindow(TEXT("PauseChild"), NULL, WS_CHILD | WS_BORDER,
            0, 0, 0, 0, hWnd, (HMENU)0, g_hInst, NULL);
        SetWindowPos(hPauseChild, NULL, 250 / 2 - 100, 400 / 2 - 100, 200, 100, SWP_NOZORDER);

        srand((unsigned int)time(NULL));

        for (int i = 0; i < 11; i++) {
            hBit[i] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1 + i));
        }
        hState = CreateStatusWindow(WS_CHILD | WS_VISIBLE, TEXT(""), hWnd, 0);
        memset(&ass, 0, sizeof(ddong));

        SetTimer(hWnd, 0, 100, NULL);
        SetTimer(hWnd, 3, 1, NULL);
        SetTimer(hWnd, 4, 1, NULL);
        SetTimer(hWnd, 6, 1, NULL);
        return 0;
    case WM_SIZE:
        for (int i = 0; i < 3; i++)
        {
            SBPart[i] = LOWORD(lParam) / 3 * (i + 1);
        }
        SendMessage(hState, SB_SETPARTS, 3, (LPARAM)SBPart);
        SetStatusText(hWnd);
        return 0;
    case WM_TIMER:
        switch (wParam) {
        case 0:
            c++;
            if (c > 2)
                c = 0;
            DoubleBuffering();
            break;
        case 1:
            for (int i = 0; i < 100; i++)
            {
                if (ass[i].be == TRUE)
                {
                    ass[i].y += ass[i].speed;
                    if (((ass[i].x < nx + 15 && nx + 15 < ass[i].x + 20) || (ass[i].x < nx && nx < ass[i].x + 20)) && ass[i].y > 310)
                    {
                        bdead = TRUE;
                        KillTimer(hWnd, 5);
                        KillTimer(hWnd, 1);
                        KillTimer(hWnd, 2);
                        isGameRunning = FALSE;
                        MoveR = FALSE;
                        MoveL = FALSE;
                        TCHAR buf[10] = { 0 };
                        //_itow_s(RankScore, buf, 10);
                        WritePrivateProfileString(TEXT("RankScore"), TEXT("Score"), buf, TEXT("Score.ini"));
                        MessageBox(hWndMain, TEXT("죽었습니다. Space bar를 누르시면 재시작 합니다."), TEXT("알림"), MB_OK);
                    }
                    if (ass[i].y > 320)
                    {
                        memset(&ass[i], 0, sizeof(ddong));
                        if (isGameRunning)
                        {
                            Score += 1;
                            if (Score > RankScore)
                                RankScore++;
                            SetStatusText(hWnd);
                        }
                    }
                }
            }
            break;
        case 2:
            Makeddong();
            break;
        case 3:
            if (MoveL)
            {
                nx -= 3;
                if (nx < 0)
                    nx = 0;
            }
            break;
        case 4:
            if (MoveR)
            {
                nx += 3;
                if (nx > 230)
                    nx = 230;
            }
            break;
        case 5:
            Time++;
            break;
        case 6:
            DoubleBuffering();
            break;
        case 7:
            if (bFade)
            {
                ShowWindow(hPauseChild, SW_SHOW);
                bFade = !bFade;
            }
            else
            {
                ShowWindow(hPauseChild, SW_HIDE);
                bFade = !bFade;
            }
            break;
        }
        return 0;
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_LEFT:
            if (!bdead)
            {
                MoveL = TRUE;
            }
            break;
        case VK_RIGHT:
            if (!bdead)
            {
                MoveR = TRUE;
            }
            break;
        case VK_SPACE:
            if (!isGameRunning)
            {
                for (int i = 0; i < 100; i++) {
                    ass[i].be = FALSE;
                }
                bdead = FALSE;
                Score = 0;
                Time = 0;
                SetStatusText(hWnd);
                nx = 110;
                SetTimer(hWnd, 1, 10, NULL);
                SetTimer(hWnd, 2, 200, NULL);
                SetTimer(hWnd, 5, 1000, NULL);
                isGameRunning = TRUE;
            }
            else if (isGameRunning)
            {
                if (bPause) {
                    bPause = FALSE;
                    SetTimer(hWnd, 1, 10, NULL);
                    SetTimer(hWnd, 2, 200, NULL);
                    SetTimer(hWnd, 5, 1000, NULL);
                    KillTimer(hWnd, 7);
                    ShowWindow(hPauseChild, SW_HIDE);
                }
                else {
                    bPause = TRUE;
                    bFade = TRUE;
                    SetTimer(hWnd, 7, 1000, NULL);
                    KillTimer(hWnd, 1);
                    KillTimer(hWnd, 2);
                    KillTimer(hWnd, 5);
                }
            }
            break;
        }
        return 0;
    case WM_KEYUP:
        switch (wParam) {
        case VK_LEFT:
            MoveL = FALSE;
            break;
        case VK_RIGHT:
            MoveR = FALSE;
            break;
        }
        count = 0;
        return 0;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        DrawBitmap(hdc, 0, 0, hBufBit);
        EndPaint(hWnd, &ps);
        return 0;
    case WM_DESTROY:
        for (int i = 0; i < 4; i++) {
            DeleteObject(hBit[i]);
        }
        PostQuitMessage(0);
        return 0;
    }
    return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}
void DoubleBuffering(void)
{
    RECT crt;
    HDC hdc, hMemDC;
    HBITMAP OldBit;

    GetClientRect(hWndMain, &crt);
    hdc = GetDC(hWndMain);
    if (hBufBit == NULL)
        hBufBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
    hMemDC = CreateCompatibleDC(hdc);
    OldBit = (HBITMAP)SelectObject(hMemDC, hBufBit);

    FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));

    for (int i = 0; i < 100; i++)
    {
        if (ass[i].be == TRUE)
        {
            TransBlt(hMemDC, ass[i].x, ass[i].y, hBit[3], RGB(255, 0, 0));
        }
    }

    if (MoveL)
        DrawBitmap(hMemDC, nx, 315, hBit[4 + c]);
    else if (MoveR)
        DrawBitmap(hMemDC, nx, 315, hBit[7 + c]);
    else if (!bdead)
        DrawBitmap(hMemDC, nx, 315, hBit[c]);
    else
        TransBlt(hMemDC, nx, 315, hBit[10], RGB(255, 0, 0));

    SelectObject(hMemDC, OldBit);
    DeleteDC(hMemDC);
    ReleaseDC(hWndMain, hdc);
    InvalidateRect(hWndMain, NULL, FALSE);
}
void Makeddong(void)
{
    int x, y;
    x = rand() % 220 + 1;
    y = 0;

    for (int i = 0; i < 100; i++)
    {
        if (ass[i].be == FALSE)
        {
            ass[i].be = TRUE;
            ass[i].x = x;
            ass[i].y = y;
            ass[i].speed = rand() % 6 + 1;
            break;
        }
    }
}
void SetStatusText(HWND hWnd)
{
    TCHAR str[128];

    wsprintf(str, TEXT("최고점수: %d "), RankScore);
    SendMessage(hState, SB_SETTEXT, 0, (LPARAM)str);
    wsprintf(str, TEXT("점수: %d"), Score);
    SendMessage(hState, SB_SETTEXT, 1, (LPARAM)str);
    wsprintf(str, TEXT("시간: %d 초"), Time);
    SendMessage(hState, SB_SETTEXT, 2, (LPARAM)str);
}
LRESULT CALLBACK PauseChildProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT crt;
    TEXTMETRIC tm;

    switch (iMessage) {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &crt);
        SetTextAlign(hdc, TA_CENTER);
        GetTextMetrics(hdc, &tm);
        TextOut(hdc, crt.right / 2, crt.bottom / 2 - tm.tmHeight / 2, TEXT("PAUSE"), 5);
        EndPaint(hWnd, &ps);
        return 0;
    }
    return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}
void TransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask)
{
    BITMAP bm;
    COLORREF cColor;
    HBITMAP bmAndBack, bmAndObject, bmAndMem, bmSave;
    HBITMAP bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
    HDC  hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
    POINT ptSize;

    hdcTemp = CreateCompatibleDC(hdc);
    SelectObject(hdcTemp, hbitmap);
    GetObject(hbitmap, sizeof(BITMAP), (LPSTR)&bm);
    ptSize.x = bm.bmWidth;
    ptSize.y = bm.bmHeight;
    DPtoLP(hdcTemp, &ptSize, 1);

    hdcBack = CreateCompatibleDC(hdc);
    hdcObject = CreateCompatibleDC(hdc);
    hdcMem = CreateCompatibleDC(hdc);
    hdcSave = CreateCompatibleDC(hdc);

    bmAndBack = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
    bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
    bmAndMem = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
    bmSave = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

    bmBackOld = (HBITMAP)SelectObject(hdcBack, bmAndBack);
    bmObjectOld = (HBITMAP)SelectObject(hdcObject, bmAndObject);
    bmMemOld = (HBITMAP)SelectObject(hdcMem, bmAndMem);
    bmSaveOld = (HBITMAP)SelectObject(hdcSave, bmSave);

    SetMapMode(hdcTemp, GetMapMode(hdc));

    BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

    cColor = SetBkColor(hdcTemp, clrMask);

    BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

    SetBkColor(hdcTemp, cColor);

    BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, NOTSRCCOPY);
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, x, y, SRCCOPY);
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);
    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);
    BitBlt(hdc, x, y, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY);
    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

    DeleteObject(SelectObject(hdcBack, bmBackOld));
    DeleteObject(SelectObject(hdcObject, bmObjectOld));
    DeleteObject(SelectObject(hdcMem, bmMemOld));
    DeleteObject(SelectObject(hdcSave, bmSaveOld));

    DeleteDC(hdcMem);
    DeleteDC(hdcBack);
    DeleteDC(hdcObject);
    DeleteDC(hdcSave);
    DeleteDC(hdcTemp);
}