#include <Windows.h>
#include <string>
#include <fstream>
#include <time.h>
#include <commctrl.h>
#include "KWnd.h"
#include "clSnake.h"
#include "resource.h"
#pragma comment( lib, "winmm" )
#pragma comment( lib , "comctl32.lib" )

#define STATUS_BAR_PARTS 2
HWND hwndStatusBar;

// typedef struct { //размер поля в клетках
// 	//const int Scale = 25; // размер клекти
// 	int Width;
// 	int Height;
// } FieldSize;

char szSoundName[] = "MY_SOUND"; //Звук при наезде на яблоко
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);				//About
void DrawField(HWND hwnd, HDC hdc, FieldSize& fs, HBITMAP Field);	//отрисовка поля
BOOL GameOver(HWND, HDC, const FieldSize& fs, const Snake& sn);			//проверка на окончание игры
void UpdateStatusBar(HWND hwnd, int Score, int HighScore);			//изменение счета в стату баре

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	HACCEL hAccel;
	MSG msg;
	srand(time(0));
	KWnd mainWind("Змейка", hInstance, nCmdShow, WndProc, MAKEINTRESOURCE(IDR_MENU1),
					0, 0, 831, 645,  CS_HREDRAW | CS_VREDRAW, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME);
	//Для использования горячих клавиш меню
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	while(GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(mainWind.GetHWnd(), hAccel,  &msg)) {		//если сообщение не от горячей клавиши
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}

//Обработка сообщений основоного окна
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	
	static HINSTANCE hInst;
	static HMENU hMenu;
	HICON hIcon;								//Иконка приложения 32*32
	HICON hIconSm;								//Иконка приложения 16*16
	static HBITMAP Field, SnakeSkin, AppleSkin; //изображения для кистей поля, змейки и фрукта

	static FieldSize fieldSize;
	static int score=0 , HighScore;
	static Snake sn;							//Главный герой игры 
	static Fructs apple;						//Фрукт за которым гоняется змейка
	static BOOL Pause;							//Пауза
	static BOOL bSound = TRUE;					//Вкл/выкл звук
	static BOOL Moving = FALSE;					//Движется ли змейка
	static int Speed;							//Скорость игры
	
	ifstream infile;							//Для записи и считывания рекорда из текстового файла
	ofstream ofile;

	switch(uMsg) {
	case WM_CREATE:
		hdc = GetDC(hwnd);
		hMenu = GetMenu(hwnd);
		hInst = GetModuleHandle(NULL);
		// загрузка и установка иконок 
		hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SNAKE));
		hIconSm = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SNAKE),
			IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		SetClassLong(hwnd, GCL_HICON, (LONG)hIcon);
		SetClassLong(hwnd, GCL_HICONSM, (LONG)hIconSm);

		// Создание строки состояния
		hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", hwnd, 200);

		//Считываем рекорд и отображаем его в строке состояния
		infile.open("Record.txt");
		infile >> HighScore;
		infile.close();
		UpdateStatusBar(hwnd, score, HighScore);

		//Загружаем изображения для кистей
		Field = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP1));
		SnakeSkin = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SKIN));
		AppleSkin = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_APPLE));

		//Устанавливаем начальную скорость и включаем таймер
		Pause = FALSE;
		Speed = 76;
		SetTimer(hwnd, 1, Speed, NULL);
		
		//Освобождаем контекст устройства
		ReleaseDC(hwnd,hdc);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		
		//Рисуем начальное положение
		DrawField(hwnd, hdc, fieldSize, Field);
		sn.DrawSnake(hwnd, hdc, SnakeSkin, Field);
		apple.DrawApple(hwnd, hdc, fieldSize, AppleSkin, sn);
		
		EndPaint(hwnd, &ps);
		break;

	case WM_TIMER:
		hdc=GetDC(hwnd);
		//Двигаем змейку
		sn.Move();
		Moving = FALSE;
		//Проверяем был ли съеден фрукт
		if ((sn.snake[0].x == apple.point.x) && (sn.snake[0].y == apple.point.y)) {
			//Если съеден воспроизводим звук(если включен)
			if (bSound)	PlaySound(szSoundName, hInst, SND_RESOURCE | SND_ASYNC);
			sn.snake.push_back(apple.point);					//Увеличиваем длину змейки
			apple.Taken();										//Рисуем новый фрукт
			score++;											//Увеличиваем счет
			UpdateStatusBar(hwnd, score, HighScore);			//Обновляем строку состояния	
			if (Speed >=40)	{									//Увеличиваем скорость, если она еще не максимальна
				Speed -= 2;							
				KillTimer(hwnd,1);
				SetTimer(hwnd, 1, Speed, NULL);
			}
		}

		//Проверка на окончание игры
		if(GameOver(hwnd, hdc, fieldSize, sn)){
			KillTimer(hwnd,1);							//Останавливаем таймер
			string scr="Ваш счет: ";					//Выводим сообщение об окончании игры и количестве
			scr += to_string((long long)score);			//набранных очков и предлагаем начать новую игру
			if (score > HighScore){						//Проверяем побит ли рекорд
				ofile.open("Record.txt");
				ofile.clear();
				ofile << score;
				ofile.close();
				scr += " Новый рекорд!";
			}
			scr += "\nПовторить?";
			
			if(IDNO==MessageBox(hwnd, scr.c_str(),"Игра окончена. Ваш счет:" , MB_YESNO))
				SendMessage(hwnd,WM_DESTROY,0,0);
			else {
				DrawField(hwnd, hdc,fieldSize, Field);
				sn.Restart();
				score = 0;
				Speed = 76;
				UpdateStatusBar(hwnd, score, HighScore);
				SetTimer(hwnd, 1, Speed, NULL);
			}
		}// Конец проверки на окончание игры if (GameOver)
		//Если игра не закончена перерисовываем змейку и фрукт
		sn.DrawSnake(hwnd, hdc, SnakeSkin, Field);
		apple.DrawApple(hwnd, hdc, fieldSize, AppleSkin, sn);
		ReleaseDC(hwnd, hdc);
		break;

	case WM_KEYDOWN:
		if (!Moving) {			//Если не стоит пауза
			Moving = TRUE;
			switch(wParam) {	//Меняем направление движения
			case VK_LEFT:		//Если нажимаем кнопку противоположную движению срабатывает звуковой сигнал
				if (sn.GetDir() != RIGHT) 
					sn.SetDir(LEFT);
				else 
					MessageBeep(-1);
				break;
			case VK_RIGHT:
				if (sn.GetDir() != LEFT) 
					sn.SetDir(RIGHT);
				else 
					MessageBeep(-1);
				break;
			case VK_DOWN:
				if (sn.GetDir() != UP)
					sn.SetDir(DOWN);
				else 
					MessageBeep(-1);
				break;
			case VK_UP:
				if (sn.GetDir()!= DOWN)
					sn.SetDir(UP);
				else 
					MessageBeep(-1);
				break;
			case VK_ESCAPE:					//Вкл/Выкл. паузу
				Moving = FALSE;				
				if (!Pause) {				
					KillTimer(hwnd, 1);		//Останавливаем таймер
					Pause = TRUE;
				}
				else {
					SetTimer(hwnd, 1, Speed, NULL);			//Запускаем таймер
					Pause = FALSE;
				}
				break;
			}// закрывает switch
		}//закрывает if
		break;
	
		//Обработка сообщений от пунктов меню
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case ID_GAME_NEW:								//Новая игра
			hdc = GetDC(hwnd);
			DrawField(hwnd, hdc, fieldSize, Field);			
			sn.Restart();
			score = 0;
			Speed = 76;
			SetTimer(hwnd, 1, Speed, NULL);
			break;

		case IDM_GAME_EXIT:								//Выход из игры
			SendMessage(hwnd, WM_DESTROY, NULL, NULL);
			break;

		case IDM_SOUND_ON:								//Включение звука
			bSound = TRUE;
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_SOUND_ON, IDM_SOUND_OFF,
				LOWORD(wParam), MF_BYCOMMAND);
			EnableMenuItem(GetSubMenu(hMenu, 1),IDM_SOUND_ON, MF_BYCOMMAND| MFS_ENABLED);
			DrawMenuBar(hwnd);
			break;

		case IDM_SOUND_OFF:								//Выключение звука
			bSound = FALSE;
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_SOUND_ON, IDM_SOUND_OFF,	
				LOWORD(wParam), MF_BYCOMMAND);
			EnableMenuItem(GetSubMenu(hMenu, 1),IDM_SOUND_OFF, MF_BYCOMMAND |  MFS_ENABLED); //отмечаем пункт "Sound off"
			DrawMenuBar(hwnd);
			break;
		case IDM_ABOUT:									//Пункт меню About
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
			break;
		}
		break;

	case WM_DESTROY:									//Выход из приложения
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

//Обработка сообщений окна About
BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
	case WM_INITDIALOG:
		return TRUE;
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//Функция рисует поле и разукрашивает его кистью с переданным изображением
void DrawField(HWND hwnd, HDC hDC, FieldSize& fs, HBITMAP Field) {
	RECT rect, rcSB;
	GetClientRect(hwnd,&rect);				//Размеры клиентской области
	GetWindowRect(hwndStatusBar, &rcSB);	//Размеры строки состояния
	int sbHeight = rcSB.bottom - rcSB.top;	//Вычисляем высотку строки состояния
	rect.bottom = rect.bottom - (sbHeight + rect.top);	//Учитываем размер строки состояния при разбиение поля на клетки

	HBRUSH FieldBrush = CreatePatternBrush(Field);				//Создаем кисть для заполнения поля
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC,FieldBrush);	//Выбираем созданную кисть и запоминаем страую
	FillRect(hDC,&rect,FieldBrush);								//Разукрашиваем поле
	FrameRect(hDC,&rect,(HBRUSH)GetStockObject(BLACK_BRUSH));	//Выводим рамку поля
	if (!fs.Calculated){										//Если ранее не считался
		//const int Scale = 25;										//Рамеры стороны клекти
		const int Width = rect.right - rect.left;						//Длина поля
		const int Height = rect.bottom - - rect.top;					//Ширина поля
		//const int Width = xVE;
		//const int Height = yVE;
	
		fs.Width = (int)Width/Scale;								//Запоминаем длину и ширину поля в клетках
		fs.Height = (int)Height/Scale;								//в структуре FieldSize
		int x0 = rect.left, y0 = rect.top;							//Начало кординат
	
		//Устанавливает pежим отобpажения контекста устpойства, котоpый опpеделяет 
		//пpеобpазования логических единиц в единицы устpойства для GDI и кооpдинат осей X и Y.
		SetMapMode(hDC, MM_ISOTROPIC);			//Одинаковый мастшаб по осям
		//Устанавливают значения переменных для формулы преобразования координат 
		SetWindowExtEx(hDC, Width, Height, NULL);	
		SetViewportExtEx(hDC, Width, Height,NULL);
		//устанавливает смещение физической системы координат
		SetViewportOrgEx(hDC, x0, y0, NULL);
		fs.Calculated = TRUE;
	}
	DeleteObject(SelectObject(hDC, hOldBrush));		//Удаляем созданную кисть и восстанавливаем старую
}

//Функция проверяет столкновения с границами поля или с хвостом
BOOL GameOver(HWND hwnd, HDC hdc,const FieldSize& fs, const Snake& sn){
	if ((sn.snake[0].x >= fs.Width) || (sn.snake[0].x < 0) || 
		(sn.snake[0].y >= fs.Height) || (sn.snake[0].y < 0))
		return TRUE;
	//Последний элемент змейки не учитывается в связи с тем, что он используется 
	//для перерисовки освободившейся из под неё клекти поля, что позволяет 
	//не перерисовывать поле при каждом продвижении змейки.
	for (int i=1; i < sn.snake.size()-2; i++)
		if (sn.snake[0].x==sn.snake[i].x && sn.snake[0].y==sn.snake[i].y)
				return TRUE;
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//Функция обновления строки состояния
void UpdateStatusBar(HWND hwnd, int Score, int HighScore) {									
	int paneWidth;
	int aWidths[STATUS_BAR_PARTS];
	string text = "Ваш счет: ";
	RECT rect;

	GetClientRect(hwnd, &rect);

	text += to_string((long long)Score);
	paneWidth = rect.right / STATUS_BAR_PARTS;
	aWidths [0] = paneWidth;
	aWidths [1] = paneWidth * 2;

	SendMessage(hwndStatusBar, SB_SETPARTS, STATUS_BAR_PARTS, (LPARAM)aWidths);	//делим строку на две части

	SendMessage(hwndStatusBar, SB_SETTEXT, 0, (LPARAM)text.c_str());			//выводим текущий счет в 1ую часть
	text = "Лучший счет: " + to_string((long long)HighScore);
	SendMessage(hwndStatusBar, SB_SETTEXT, 1, (LPARAM)text.c_str());			//выводим лучший счет во 2ую часть
}