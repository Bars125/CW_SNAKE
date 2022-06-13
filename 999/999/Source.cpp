#define _CRT_SECURE_NO_WARNINGS
#define WINDOW_WIDTH  720
#define WINDOW_HEIGHT 564
#define SNAKE_SIZE    22
#define BORDERPEN     2
#define ROWS          23
#define COLS          32
#define SN_NONE       0  // условное обочзначение пустой клетки
#define SNAKE_BODY    1
#define FOOD          2
#define SNAKE_HEAD    3
#define FILE_NAME     "results.txt"

#include <windows.h> // підключення бібліотеки з функціями API
#include "resource.h"
#include <vector>
#include <string>
#include <fstream>

// Глобальні змінні:
HINSTANCE hInst; 	//Дескриптор програми	
LPCTSTR szWindowClass = "QWERTY";
LPCTSTR szTitle = "SNAKE";
BYTE g_mat[ROWS][COLS];
HWND hWnd;
HANDLE hFile;
int snake_speed = 100;
bool g_over, wait_before_start;
int game_mode = 0;
int count_scores = 0;
std::vector < std::string>File_read_vec;

// Попередній опис функцій
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DlgProc1(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void Start_prepare(int& dir);
void Draw_snake(HWND hWnd, HDC mdc, int dir, HBRUSH back, HPEN pen, HBRUSH food);
void Food_Location(void);
void Go_text(HDC mdc);

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW; 		//стиль вікна
	wcex.lpfnWndProc = (WNDPROC)WndProc; 		//віконна процедура
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; 			//дескриптор програми
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)); 		//визначення іконки
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW); 	//визначення курсору
	wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW + 1); //установка фону
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); 				//визначення меню
	wcex.lpszClassName = szWindowClass; 		//ім’я класу
	wcex.hIconSm = NULL;

	if (!RegisterClassEx(&wcex))
		return 1;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	// Реєстрація класу вікна 
	MyRegisterClass(hInstance);

	// Створення вікна програми
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	// Цикл обробки повідомлень
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnregisterClass(szWindowClass, hInstance);
	return 0;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; //зберігає дескриптор додатка в змінній hInst
	DWORD sty = WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX);
	hWnd = CreateWindow(szWindowClass, szTitle, sty, (GetSystemMetrics(SM_CXSCREEN) - WINDOW_WIDTH) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - WINDOW_HEIGHT) / 2, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
	SetClassLong(hWnd, GCLP_HICON, (LONG)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)));
	if (!hWnd) 	//Якщо вікно не творилось, функція повертає FALSE
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow); 		//Показати вікно
	UpdateWindow(hWnd); 				//Оновити вікно
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static HBITMAP hbm = NULL;
	static HBRUSH body = NULL;
	static HPEN    pen = NULL;
	static HBRUSH food = NULL;
	static HDC mdc = NULL;
	static int dir = VK_UP;
	static std::string temp = "";
	static std::ifstream myfile;

	switch (message) {
	case WM_CREATE:
		hdc = GetDC(hWnd);
		mdc = CreateCompatibleDC(hdc);
		hbm = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
		SelectObject(mdc, hbm);
		ReleaseDC(hWnd, hdc);
		body = CreateSolidBrush(RGB(0, 255, 0));
		food = CreateSolidBrush(RGB(200, 0, 20));
		pen = CreatePen(PS_SOLID, BORDERPEN, RGB(0, 200, 200));
		Start_prepare(dir);
		SetTimer(hWnd, NULL, snake_speed, NULL);
		hFile = CreateFile(FILE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
		CloseHandle(hFile);
		break;
	case WM_ERASEBKGND:
		PatBlt(mdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, WHITENESS);
		Draw_snake(hWnd, mdc, dir, body, pen, food);
		BitBlt((HDC)wParam, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, mdc, 0, 0, SRCCOPY);
		return 1;
	case WM_KEYDOWN:
		wait_before_start = true;
		switch (LOWORD(wParam)) {
		case VK_LEFT:
			dir = VK_LEFT;
			break;
		case VK_RIGHT:
			dir = VK_RIGHT;
			break;
		case VK_UP:
			dir = VK_UP;
			break;
		case VK_DOWN:
			dir = VK_DOWN;
			break;
		case VK_RETURN:
			if (g_over)
				Start_prepare(dir);
			break;
		}
		break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_MODE_NORMAL:
		{
			game_mode = 0;
		}
		break;
		case ID_MODE_CHEAT:
		{
			++game_mode;
		}
		break;
		case ID_MAIN_HOWTOPLAY:
		{
			MessageBox(hWnd, "Управление осуществляется стрелками\n   - Вверх\n   - Вниз\n   - Влево\n   - Вправо", "RULES", MB_OK | MB_ICONINFORMATION);
		}
		break;
		case ID_VIEWRESULTS_RANKINGTABLE:
		{
			myfile.open(FILE_NAME);
			while (getline(myfile, temp)) {
				File_read_vec.push_back(temp);  // Output the text from the file
			}
			temp = "";
			myfile.close();
			for (int i = 0; i < File_read_vec.size(); i++) {
				temp += File_read_vec[i];
				temp += "\n";
			}
			MessageBoxA(hWnd, temp.c_str(), "Rank", MB_OK);
			File_read_vec.clear();
			temp = "";
		}
		break;
		case ID_VIEWRESULTS_CLEARLIST:
		{
			DeleteFileA(FILE_NAME);
		}
		break;
		case ID_MAIN_EXIT:
		{
			PostQuitMessage(0);
		}break;
		}

	}break;
	case WM_TIMER:
		if (wait_before_start == true) {
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;
	case WM_DESTROY:
		DeleteDC(mdc);
		DeleteObject(hbm);
		DeleteObject(body);
		DeleteObject(pen);
		DeleteObject(food);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

struct point {
	int x = 0, y = 0;
	point(void) {}
	point(int _x, int _y) :x(_x), y(_y) {}
};
std::vector<point> g_snake;

void Start_prepare(int& dir) {
	count_scores = 0;
	SetWindowText(hWnd, ("SNAKE - " + std::to_string(count_scores)).c_str());
	dir = VK_UP;
	g_snake.clear();
	for (int i = 0; i < ROWS; ++i) {
		for (int j = 0; j < COLS; ++j)
			g_mat[i][j] = SN_NONE;
	}

	for (int b = 0; b < 3; ++b) {
		g_snake.push_back(point(COLS / 2 - 1, ROWS / 2 + b));
		const point& p = g_snake.back();
		g_mat[p.y][p.x] = SNAKE_BODY;
	}
	Food_Location();
	g_over = false;
}

void Food_Location(void) {
	int x, y;
	do {
		x = rand() % COLS;
		y = rand() % ROWS;
		if (g_mat[y][x] == SN_NONE) {
			g_mat[y][x] = FOOD;
			break;
		}
	} while (1);
}

void Go_text(HDC mdc) {
	std::string str_gm = "GAME OVER!";
	std::string str_ta = "TAP ENTER TO RETRY";
	TextOut(mdc, WINDOW_WIDTH / 2.4, WINDOW_HEIGHT / 2.7, str_gm.c_str(), str_gm.length());
	TextOut(mdc, WINDOW_WIDTH / 2.6, WINDOW_HEIGHT / 2.3, str_ta.c_str(), str_ta.length());
}

void Draw_snake(HWND hWnd, HDC mdc, int dir, HBRUSH back, HPEN pen, HBRUSH food) {

	if (g_over) {
		wait_before_start = false;
		Go_text(mdc);
		if (count_scores > 5) {
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)DlgProc1);
		}
		return;
	}

	point prev = g_snake.front();
	switch (dir) {
	case VK_LEFT:
		--g_snake.front().x;
		break;
	case VK_RIGHT:
		++g_snake.front().x;
		break;
	case VK_UP:
		--g_snake.front().y;
		break;
	case VK_DOWN:
		++g_snake.front().y;
		break;
	}

	//проверка на столкновение в разных режимах
	point q = g_snake.front();
	if (game_mode == 0) {
		if ((q.x < 0) || (q.y < 0) || (q.y >= ROWS) || (q.x >= COLS) || (g_mat[q.y][q.x] == SNAKE_BODY)) {
			g_over = true;
			return;
		}
	}
	else if (game_mode == 1) {
		if ((q.x < 0) || (q.y < 0) || (q.y >= ROWS) || (q.x >= COLS)) {
			g_over = true;
			return;
		}
	}

	point e = g_snake.back();
	std::vector<point>::size_type s;
	for (s = 1; s < g_snake.size(); ++s)
		std::swap(g_snake[s], prev);

	if (g_mat[q.y][q.x] == FOOD) {
		if (g_snake.size() > 512) {
			g_over = true;
			return;
		}
		if (game_mode == 1) {
			snake_speed -= 80;
		}
		count_scores++;
		SetWindowText(hWnd, ("SNAKE - " + std::to_string(count_scores)).c_str());
		g_snake.push_back(e);
		g_mat[q.y][q.x] = SN_NONE;
		Food_Location();
	}
	g_mat[e.y][e.x] = SN_NONE;
	g_mat[g_snake.front().y][g_snake.front().x] = SNAKE_HEAD;

	for (s = 1; s < g_snake.size(); ++s)
		g_mat[g_snake[s].y][g_snake[s].x] = SNAKE_BODY;

	//рисование змейки, пищи
	RECT rc;
	HGDIOBJ a, b;
	int  x, y, m;
	a = SelectObject(mdc, back);
	b = SelectObject(mdc, pen);
	for (int i = 0; i < ROWS; ++i) {
		for (int j = 0; j < COLS; ++j) {
			switch (g_mat[i][j]) {
			case SNAKE_HEAD: //вывод головы
				x = j * SNAKE_SIZE;
				y = i * SNAKE_SIZE;
				m = SetROP2(mdc, R2_XORPEN);
				Rectangle(mdc, x, y, x + SNAKE_SIZE, y + SNAKE_SIZE);
				SetROP2(mdc, m);
				break;
			case SNAKE_BODY: //вывод тела
				x = j * SNAKE_SIZE;
				y = i * SNAKE_SIZE;
				Rectangle(mdc, x, y, x + SNAKE_SIZE, y + SNAKE_SIZE);
				break;
			case FOOD: //вывод пищи
				x = j * SNAKE_SIZE;
				y = i * SNAKE_SIZE;
				SetRect(&rc, x, y, x + SNAKE_SIZE, y + SNAKE_SIZE);
				FillRect(mdc, &rc, food);
				break;
			}
		}
	}
	SelectObject(mdc, a);
	SelectObject(mdc, b);
}

BOOL CALLBACK DlgProc1(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	HWND edit;
	char buf[256];
	int j = 0;
	std::string finalString = "";
	std::string name = "";
	std::ofstream myfile;

	switch (message)
	{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			WORD File_symbols_col = (WORD)SendDlgItemMessage(hWnd, IDC_EDIT1, EM_LINELENGTH, (WPARAM)0, (LPARAM)0); // посылает сообщения IDC_EDIT1 для получения количества символов в веденной строке			
			if (File_symbols_col == 0) {
				MessageBox(hWnd, "You haven't entered symbols!", "Error", MB_OK);
				return FALSE;

			}
			edit = GetDlgItem(hWnd, IDC_EDIT1);
			GetWindowTextA(edit, buf, sizeof(buf));
			while (buf[j] != 0) {
				name += buf[j];
				j++;
			}
			finalString = name + " - " + std::to_string(count_scores);
			myfile.open(FILE_NAME, std::ios::app);
			myfile << finalString << std::endl;
			myfile.close();
			EndDialog(hWnd, wParam);
		}break;
		case SKIP:
		{
			EndDialog(hWnd, wParam);
			return TRUE;
		}break;
		}
	}
	}
	return FALSE;
}
