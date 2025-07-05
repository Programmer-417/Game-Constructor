#undef UNICODE
#pragma warning(disable: 4996)
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "Winmm.lib")

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

HFONT VarOfFonts[210];
HPEN colorPen[2];

COLORREF chColors[14] = {RGB(237, 28, 36), RGB(255, 127, 39), RGB(255, 201, 14), RGB(34, 177, 76), RGB(0, 162, 232), RGB(63, 72, 204),
                        RGB(0, 0, 0), RGB(127, 127, 127), RGB(185, 122, 87), RGB(163, 73, 164), RGB(112, 146, 190), RGB(255, 255, 255)};

char chColorsMass[14][20] = { "RGB(237, 28, 36)", "RGB(255, 127, 39)", "RGB(255, 201, 14)", "RGB(34, 177, 76)", "RGB(0, 162, 232)", "RGB(63, 72, 204)",
                        "RGB(0, 0, 0)", "RGB(127, 127, 127)", "RGB(185, 122, 87)", "RGB(163, 73, 164)", "RGB(112, 146, 190)", "RGB(255, 255, 255)" };


int windowSize[2] = { 1300, 740 };
int windowStartLocation[2] = { 50, 50 };
int mouseCoords[2] = { 0, 0 };
int click = 0;
int clickRight = 0;
int mouseWheel = 0;
int mouseWheelCoords = 0;
int kistParam[10];
char figureNames[10][20] = { "1.Ellipse", "2. Rectangle", "3. RoundRect", "4. Just line", "5. Polygon" };
float sizeVar = 0;

float savedFigures[200][10];
POINT polygonsCoords[30][400];
int polygonsCount = 0;

int savedCount = 0;

float systemParam = 80.0;
float cameraCoords[2] = { 0, 0 };

DWORD W, H;

LRESULT WndProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
    if (message == WM_DESTROY) PostQuitMessage(0);
    else if (message == WM_KEYDOWN) {
        if (wp > 48 && wp < 54) {
            kistParam[1] = wp - 49;
        }
    }
    else if (message == WM_LBUTTONDOWN) {
        click = 1;
    }
    else if (message == WM_RBUTTONDOWN) {
        clickRight = 1;
    }
    else if (message == WM_MOUSEWHEEL) {
        mouseWheel = GET_WHEEL_DELTA_WPARAM(wp) / 24;
        mouseWheelCoords += 2*(mouseWheel / 5.0);
        sizeVar += 2*(mouseWheel / 5.0);
    }
    else if (message == WM_LBUTTONUP) {
        click = 0;
    }
    else if (message == WM_RBUTTONUP) {
        clickRight = 0;
    }
    else if (message == WM_MOUSEMOVE) {
        mouseCoords[0] = LOWORD(lp);
        mouseCoords[1] = HIWORD(lp);
    }
    else DefWindowProcA(hwnd, message, wp, lp);
}

int WinInit() {
    setlocale(LC_ALL, "ru-RU");

    for (int i = 1; i < 200; ++i) {
        VarOfFonts[i] = CreateFontW(i, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 1, 0, 0, 0, 0, "Calibri");
    }

    W = windowSize[0];
    H = windowSize[1];

    colorPen[0] = CreatePen(PS_DASH, 3, RGB(0, 0, 0));
    colorPen[1] = CreatePen(PS_DASH, 3, RGB(255, 255, 255));
}

// 'My' functions

void MyRectangle(HDC dc, int x1, int y1, int x2, int y2) {
    Rectangle(dc, cameraCoords[0] + x1+windowSize[0] / 2, cameraCoords[1] + y1 + windowSize[1] / 2, cameraCoords[0] + x2 + windowSize[0] / 2, cameraCoords[1] + y2 + windowSize[1] / 2);
}

void MyEllipse(HDC dc, int x1, int y1, int x2, int y2) {
    Ellipse(dc, cameraCoords[0] + x1 + windowSize[0] / 2, cameraCoords[1] + y1 + windowSize[1] / 2, cameraCoords[0] + x2 + windowSize[0] / 2, cameraCoords[1] + y2 + windowSize[1] / 2);
}

void MyLine(HDC dc, int x1, int y1, int x2, int y2) {
    MoveToEx(dc, cameraCoords[0] + x1 + windowSize[0] / 2, cameraCoords[1] + y1 + windowSize[1] / 2, 0);
    LineTo(dc, cameraCoords[0] + x2 + windowSize[0] / 2, cameraCoords[1] + y2 + windowSize[1] / 2);
}

void MyText(HDC dc, int x1, int y1, char text[100]) {
    TextOut(dc, cameraCoords[0] + x1, cameraCoords[1] + y1, text, strlen(text));
}

void MyRoundRect(HDC dc, int x1, int y1, int x2, int y2, int wd, int hd) {
    RoundRect(dc, cameraCoords[0] + x1 + windowSize[0] / 2, cameraCoords[1] + y1 + windowSize[1] / 2, cameraCoords[0] + x2 + windowSize[0] / 2, cameraCoords[1] + y2 + windowSize[1] / 2, wd, hd);
}

void MyPolygon(HDC dc, POINT coords[100], int size) {
    for (int i = 0; i < size; i++) {
        coords[i].x = cameraCoords[0] + coords[i].x;
        coords[i].y = cameraCoords[1] + coords[i].y;
    }
    Polygon(dc, coords, size);
}

// Other functions

int intLimit(int num, int maxNum) {
    if (num > maxNum) return maxNum;
    else return num;
}

void ShowCoordsSystem(HDC dc) {
    SetClr(dc, RGB(100, 100, 100));

    SetTextColor(dc, RGB(100, 100, 100));
    SetBkMode(dc, TRANSPARENT);

    char sym[10];
    static int mouseStatic[2] = { 0, 0 };

    int systemSize = 80+mouseWheelCoords;

    if (clickRight) {
        if (mouseStatic[0] + mouseStatic[1]) {
            cameraCoords[0] += mouseCoords[0] - mouseStatic[0];
            cameraCoords[1] += mouseCoords[1] - mouseStatic[1];
        }

        mouseStatic[0] = mouseCoords[0];
        mouseStatic[1] = mouseCoords[1];
    }
    else {
        mouseStatic[0] = 0;
        mouseStatic[1] = 0;
    }

    if (systemSize > 110) {
        mouseWheelCoords = 0;
        systemParam /= 2.0;
    }
    else if (systemSize < 50) {
        mouseWheelCoords = 0;
        systemParam *= 2.0;
    }

    for (int i = -100; i < 100; i++){
        sprintf(sym, "%.0f", i*systemParam);

        MyText(dc, windowSize[0] / 2 + i * systemSize, windowSize[1] / 2, sym);
        MyLine(dc, i * systemSize, -10000, i * systemSize, 10000);

        MyText(dc, windowSize[0] / 2, windowSize[1] / 2 - i * systemSize, sym);
        MyLine(dc, -10000, -i * systemSize, 10000, -i * systemSize);
    }
}

void ColorChanger(HDC dc) {
    SetBkMode(dc, TRANSPARENT);
    SetClr(dc, RGB(150, 150, 150));
    RoundRect(dc, 300, 20, 1000, 100, 50, 50);

    for (int i = 0; i < 12; i++){
        SetClr(dc, chColors[i]);
        SelectObject(dc, colorPen[kistParam[0]==i]);

        int x = 330 + 40 * (i % 6), y = 25 + 40 * (i / 6.0 >= 1);

        Ellipse(dc, x, y, x+30, y+30);

        if (mouseCoords[0] > x && mouseCoords[1] > y && mouseCoords[0] < x + 30 && mouseCoords[1] < y + 30 && click) {
            click = 0;
            kistParam[0] = i;
        }
    }

    SetTextColor(dc, RGB(0, 0, 0));
    SelectObject(dc, VarOfFonts[30]);

    TextOut(dc, 570, 40, figureNames[kistParam[1]], strlen(figureNames[kistParam[1]]));

    if (mouseCoords[0] > 720 && mouseCoords[0] < 820 && mouseCoords[1]>30 && mouseCoords[1] < 80 && click) {
        SelectObject(dc, colorPen[1]);
        kistParam[2] = 1;
        click = 0;
    }
    else {
        SelectObject(dc, colorPen[0]);
        kistParam[2] = 0;
    }

    RoundRect(dc, 720, 30, 820, 80, 15, 15);

    TextOut(dc, 725, 35, "Back", 4);

    SetClr(dc, RGB(255 * !kistParam[3], 255 * kistParam[3], 0));

    if (mouseCoords[0] > 840 && mouseCoords[0] < 890 && mouseCoords[1]>30 && mouseCoords[1] < 80 && click) {
        kistParam[3] = !kistParam[3];
        SelectObject(dc, colorPen[1]);
        click = 0;
    }
    else {
        SelectObject(dc, colorPen[0]);
    }

    RoundRect(dc, 840, 30, 890, 80, 15, 15);
    TextOut(dc, 845, 35, "Tr", 2);

    SetClr(dc, RGB(255, 255, 255));

    if (mouseCoords[0] > 910 && mouseCoords[0] < 960 && mouseCoords[1]>30 && mouseCoords[1] < 80 && click) {
        SelectObject(dc, colorPen[1]);
        kistParam[4] = 1;
        click = 0;
    }
    else {
        SelectObject(dc, colorPen[0]);
    }

    RoundRect(dc, 910, 30, 960, 80, 15, 15);
    TextOut(dc, 912, 35, "Go", 2);
}

void FigureCreator(HDC dc) {
    static int x0, y0;
    static int polygonCreating = 0;
    POINT polygonCoords[400];

    if (polygonCreating) {
        int clrSlct = savedFigures[savedCount][4];

        SetClr(dc, chColors[clrSlct]);

        if (!savedFigures[savedCount][6]) {
            SelectObject(dc, GetStockObject(NULL_BRUSH));
        }

        float sizeMaster = 1.0 / (sizeVar / savedFigures[savedCount][7]);

        for (int j = 0; j < savedFigures[savedCount][0]; j++) {
            polygonCoords[j].x = polygonsCoords[polygonsCount][j].x * sizeMaster + cameraCoords[0]+windowSize[0] / 2;
            polygonCoords[j].y = polygonsCoords[polygonsCount][j].y * sizeMaster + cameraCoords[1]+ windowSize[1] / 2;
        }

        Polygon(dc, polygonCoords, savedFigures[savedCount][0]);
    }

    if (click && mouseCoords[1]>100) {
        if (kistParam[1] == 4 || polygonCreating) {
            click = 0;

            if (!polygonCreating) {
                savedFigures[savedCount][4] = kistParam[0];
                savedFigures[savedCount][5] = kistParam[1];
                savedFigures[savedCount][6] = kistParam[3];

                savedFigures[savedCount][7] = sizeVar;
            }

            polygonsCoords[polygonsCount][polygonCreating].x = mouseCoords[0] - cameraCoords[0]- windowSize[0] / 2;
            polygonsCoords[polygonsCount][polygonCreating].y = mouseCoords[1] - cameraCoords[1]- windowSize[1] / 2;

            polygonCreating += 1;
            savedFigures[savedCount][0] = polygonCreating;

            if (polygonCreating > 2) {
                int x1 = polygonsCoords[polygonsCount][polygonCreating - 1].x;
                int y1 = polygonsCoords[polygonsCount][polygonCreating - 1].y;

                int x2 = polygonsCoords[polygonsCount][polygonCreating-2].x;
                int y2 = polygonsCoords[polygonsCount][polygonCreating-2].y;

                float size = sqrt(((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2)));

                if (size < 5) {
                    polygonCreating = 0;
                    polygonsCount += 1;
                    savedCount += 1;
                }
            }
        }
        else{
            if (!x0) {
                x0 = mouseCoords[0];
                y0 = mouseCoords[1];
            }

            SetClr(dc, chColors[kistParam[0]]);

            if (!kistParam[3]) {
                SelectObject(dc, GetStockObject(NULL_BRUSH));
            }

            if (kistParam[1] == 0) {
                Ellipse(dc, x0, y0, mouseCoords[0], mouseCoords[1]);
            }
            else if (kistParam[1] == 1) {
                Rectangle(dc, x0, y0, mouseCoords[0], mouseCoords[1]);
            }
            else if (kistParam[1] == 2) {
                float size = sqrt((x0 - mouseCoords[0]) * (x0 - mouseCoords[0]) + (y0 - mouseCoords[1]) * (y0 - mouseCoords[1]));
                size *= 0.2;

                RoundRect(dc, x0, y0, mouseCoords[0], mouseCoords[1], size, size);
            }
            else if (kistParam[1] == 3) {
                MoveToEx(dc, x0, y0, 0);
                LineTo(dc, mouseCoords[0], mouseCoords[1]);
            }
        }
    }
    else if(x0) {
        click = 0;

        savedFigures[savedCount][0] = x0- cameraCoords[0]-windowSize[0]/2;
        savedFigures[savedCount][1] = y0- cameraCoords[1]- windowSize[1] / 2;
        savedFigures[savedCount][2] = mouseCoords[0]-cameraCoords[0]-windowSize[0] / 2;
        savedFigures[savedCount][3] = mouseCoords[1]-cameraCoords[1]-windowSize[1] / 2;

        savedFigures[savedCount][4] = kistParam[0];
        savedFigures[savedCount][5] = kistParam[1];
        savedFigures[savedCount][6] = kistParam[3];

        savedFigures[savedCount][7] = sizeVar;

        savedCount += 1;

        x0 = 0;
        y0 = 0;
    }
    if (kistParam[2] && savedCount>0) {
        kistParam[2] = 0;
        if (savedFigures[savedCount][5] == 4 || savedFigures[savedCount][5] == 0 && savedFigures[savedCount-1][5] == 4) {
            float size = 0;

            if (polygonCreating > 1) {
                int x1 = polygonsCoords[polygonsCount][polygonCreating - 1].x;
                int y1 = polygonsCoords[polygonsCount][polygonCreating - 1].y;

                int x2 = polygonsCoords[polygonsCount][polygonCreating - 2].x;
                int y2 = polygonsCoords[polygonsCount][polygonCreating - 2].y;

                size = sqrt(((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2)));
            }
            else {
                size = 154;
                polygonsCount -= 1;
                savedCount -= 1;
            }

            savedFigures[savedCount][4] = 0;
            savedFigures[savedCount][5] = 0;
            savedFigures[savedCount][6] = 0;
            savedFigures[savedCount][7] = 0;

            for (int i = 0; i < savedFigures[savedCount][0]; i++){
                polygonsCoords[polygonsCount][i].x = 0;
                polygonsCoords[polygonsCount][i].y = 0;
            }

            polygonCreating = 0;
            savedFigures[savedCount][0] = 0;

            if (size < 5) {
                polygonsCount -= 1;
                savedCount -= 1;
            }
        }
        else savedCount -= 1;
    }
}

void ShowSavedFigures(HDC dc) {
    float sizeVar1 = (((mouseWheelCoords + 30) / 60.0) / 0.25 - 2) * 0.5;
    sizeVar = systemParam / 80.0;

    int polyCounter = 0;

    if (sizeVar1 < 0) sizeVar = sizeVar - sizeVar *sizeVar1;
    else if(sizeVar1!=0) sizeVar = sizeVar - sizeVar *0.5* sizeVar1;

    for (int i = 0; i < savedCount; i++){
        int clrSlct = savedFigures[i][4];

        SetClr(dc, chColors[clrSlct]);

        float sizeMaster = 1.0/(sizeVar/savedFigures[i][7]);

        if (!savedFigures[i][6]) {
            SelectObject(dc, GetStockObject(NULL_BRUSH));
        }

        if (savedFigures[i][5] == 0) {
            MyEllipse(dc, savedFigures[i][0]*sizeMaster, savedFigures[i][1] * sizeMaster, savedFigures[i][2] * sizeMaster, savedFigures[i][3] * sizeMaster);
        }
        else if (savedFigures[i][5] == 1) {
            MyRectangle(dc, savedFigures[i][0] * sizeMaster, savedFigures[i][1] * sizeMaster, savedFigures[i][2] * sizeMaster, savedFigures[i][3] * sizeMaster);
        }
        else if (savedFigures[i][5] == 2) {
            float size = sqrt((savedFigures[i][0]- savedFigures[i][2]) * (savedFigures[i][0] - savedFigures[i][2]) + (savedFigures[i][1]- savedFigures[i][3]) * (savedFigures[i][1] - savedFigures[i][3])) * sizeMaster;
            size *= 0.2;

            MyRoundRect(dc, savedFigures[i][0] * sizeMaster, savedFigures[i][1] * sizeMaster, savedFigures[i][2] * sizeMaster, savedFigures[i][3] * sizeMaster, size, size);
        }
        else if (savedFigures[i][5] == 3) {
            MyLine(dc, savedFigures[i][0] * sizeMaster, savedFigures[i][1] * sizeMaster, savedFigures[i][2] * sizeMaster, savedFigures[i][3] * sizeMaster);
        }
        else if (savedFigures[i][5] == 4) {
            POINT polygonCoords[400];

            for (int j = 0; j < savedFigures[i][0]; j++){
                polygonCoords[j].x = polygonsCoords[polyCounter][j].x * sizeMaster + cameraCoords[0]+ windowSize[0] / 2;
                polygonCoords[j].y = polygonsCoords[polyCounter][j].y * sizeMaster + cameraCoords[1]+ windowSize[1] / 2;
            }

            Polygon(dc, polygonCoords, savedFigures[i][0]);
            polyCounter += 1;
        }
    }
}

int PrintEndCode() {
    int polyCounter = 0;

    for (int i = 0; i < savedCount; i++){
        int clrSlct = savedFigures[i][4];

        printf("SetClr(dc, %s);\n", chColorsMass[clrSlct]);

        float sizeMaster = 1.0 / (sizeVar / savedFigures[i][7]);

        if (!savedFigures[i][6]) {
            printf("SelectObject(dc, GetStockObject(NULL_BRUSH));\n");
        }

        int x1 = savedFigures[i][0];
        int y1 = savedFigures[i][1] + windowSize[1] / 2;
        int x2 = savedFigures[i][2];
        int y2 = savedFigures[i][3] + windowSize[1] / 2;

        if (savedFigures[i][5] == 0) {
            printf("Ellipse(dc, %d, %d, %d, %d);\n", x1, y1, x2, y2);
        }
        else if (savedFigures[i][5] == 1) {
            printf("Rectangle(dc, %d, %d, %d, %d);\n", x1, y1, x2, y2);
        }
        else if (savedFigures[i][5] == 2) {
            int size = 0.2*sqrt((savedFigures[i][0] - savedFigures[i][2]) * (savedFigures[i][0] - savedFigures[i][2]) + (savedFigures[i][1] - savedFigures[i][3]) * (savedFigures[i][1] - savedFigures[i][3])) * sizeMaster;

            printf("RoundRect(dc, %d, %d, %d, %d, %d, %d);\n", x1, y1, x2, y2, size, size);
        }
        else if (savedFigures[i][5] == 3) {
            printf("MoveToEx(dc, %d, %d, 0);\n", x1, y1);
            printf("LineTo(dc, %d, %d);\n", x2, y2);
        }
        else if (savedFigures[i][5] == 4) {
            printf("\nPOINT polyCoords%d[200];\n", polyCounter);
            for (int p = 0; p < savedFigures[i][0]; p++){
                int x3 = polygonsCoords[polyCounter][p].x;
                int y3 = polygonsCoords[polyCounter][p].y + windowSize[1] / 2;

                printf("polyCoords%d[%d].x = %d;\n", polyCounter, p, x3);
                printf("polyCoords%d[%d].y = %d;\n", polyCounter, p, y3);
                printf("\n");
            }
            int count = savedFigures[i][0];

            printf("Polygon(dc, polyCoords%d, %d);\n", polyCounter, count);
            polyCounter += 1;
        }
        printf("\n");
    }

    return 0;
}

int SetClr(HDC dc, COLORREF color) {
    SelectObject(dc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc, color);
    SelectObject(dc, GetStockObject(DC_PEN));
    SetDCPenColor(dc, color);
}

void ClearDisplay(HDC dc) {
    SelectObject(dc, GetStockObject(DC_PEN));
    SetDCPenColor(dc, RGB(50, 50, 50));

    SelectObject(dc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc, RGB(50, 50, 50));

    Rectangle(dc, 0, 0, windowSize[0], windowSize[1]);
}

int WinShow(HDC dc) {
    HDC memDC = CreateCompatibleDC(dc);
    HBITMAP memBM = CreateCompatibleBitmap(dc, windowSize[0], windowSize[1]);
    SelectObject(memDC, memBM);

    ClearDisplay(memDC);

    ShowSavedFigures(memDC);

    FigureCreator(memDC);

    if (kistParam[4]) {
        kistParam[4] = 0;
        PrintEndCode();
    }

    ShowCoordsSystem(memDC);
    ColorChanger(memDC);

    BitBlt(dc, 0, 0, windowSize[0], windowSize[1], memDC, 0, 0, SRCCOPY);
    DeleteDC(memDC);
    DeleteObject(memBM);
}

int main() {
    WNDCLASSA wcl;
    memset(&wcl, 0, sizeof(WNDCLASSA));

    wcl.lpszClassName = "my Window";
    wcl.lpfnWndProc = WndProc;

    RegisterClassA(&wcl);

    HWND hwnd = CreateWindow("my Window", "Game Constructor", WS_OVERLAPPEDWINDOW,
        windowStartLocation[0], windowStartLocation[1], windowSize[0], windowSize[1], NULL, NULL, NULL, NULL);

    ShowWindow(hwnd, SW_SHOWNORMAL);

    HDC dc = GetDC(hwnd);

    WinInit();

    MSG msg;

    while (1) {
        if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
            TranslateMessage(&msg);
            if (msg.message == WM_QUIT) break;
        }
        else {
            WinShow(dc);
            Sleep(1);
        }
    }
}
