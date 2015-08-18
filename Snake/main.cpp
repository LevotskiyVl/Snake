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

// typedef struct { //������ ���� � �������
// 	//const int Scale = 25; // ������ ������
// 	int Width;
// 	int Height;
// } FieldSize;

char szSoundName[] = "MY_SOUND"; //���� ��� ������ �� ������
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);				//About
void DrawField(HWND hwnd, HDC hdc, FieldSize& fs, HBITMAP Field);	//��������� ����
BOOL GameOver(HWND, HDC, const FieldSize& fs, const Snake& sn);			//�������� �� ��������� ����
void UpdateStatusBar(HWND hwnd, int Score, int HighScore);			//��������� ����� � ����� ����

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	HACCEL hAccel;
	MSG msg;
	srand(time(0));
	KWnd mainWind("������", hInstance, nCmdShow, WndProc, MAKEINTRESOURCE(IDR_MENU1),
					0, 0, 831, 645,  CS_HREDRAW | CS_VREDRAW, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME);
	//��� ������������� ������� ������ ����
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	while(GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(mainWind.GetHWnd(), hAccel,  &msg)) {		//���� ��������� �� �� ������� �������
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}

//��������� ��������� ���������� ����
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	
	static HINSTANCE hInst;
	static HMENU hMenu;
	HICON hIcon;								//������ ���������� 32*32
	HICON hIconSm;								//������ ���������� 16*16
	static HBITMAP Field, SnakeSkin, AppleSkin; //����������� ��� ������ ����, ������ � ������

	static FieldSize fieldSize;
	static int score=0 , HighScore;
	static Snake sn;							//������� ����� ���� 
	static Fructs apple;						//����� �� ������� �������� ������
	static BOOL Pause;							//�����
	static BOOL bSound = TRUE;					//���/���� ����
	static BOOL Moving = FALSE;					//�������� �� ������
	static int Speed;							//�������� ����
	
	ifstream infile;							//��� ������ � ���������� ������� �� ���������� �����
	ofstream ofile;

	switch(uMsg) {
	case WM_CREATE:
		hdc = GetDC(hwnd);
		hMenu = GetMenu(hwnd);
		hInst = GetModuleHandle(NULL);
		// �������� � ��������� ������ 
		hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SNAKE));
		hIconSm = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SNAKE),
			IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		SetClassLong(hwnd, GCL_HICON, (LONG)hIcon);
		SetClassLong(hwnd, GCL_HICONSM, (LONG)hIconSm);

		// �������� ������ ���������
		hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", hwnd, 200);

		//��������� ������ � ���������� ��� � ������ ���������
		infile.open("Record.txt");
		infile >> HighScore;
		infile.close();
		UpdateStatusBar(hwnd, score, HighScore);

		//��������� ����������� ��� ������
		Field = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP1));
		SnakeSkin = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SKIN));
		AppleSkin = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_APPLE));

		//������������� ��������� �������� � �������� ������
		Pause = FALSE;
		Speed = 76;
		SetTimer(hwnd, 1, Speed, NULL);
		
		//����������� �������� ����������
		ReleaseDC(hwnd,hdc);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		
		//������ ��������� ���������
		DrawField(hwnd, hdc, fieldSize, Field);
		sn.DrawSnake(hwnd, hdc, SnakeSkin, Field);
		apple.DrawApple(hwnd, hdc, fieldSize, AppleSkin, sn);
		
		EndPaint(hwnd, &ps);
		break;

	case WM_TIMER:
		hdc=GetDC(hwnd);
		//������� ������
		sn.Move();
		Moving = FALSE;
		//��������� ��� �� ������ �����
		if ((sn.snake[0].x == apple.point.x) && (sn.snake[0].y == apple.point.y)) {
			//���� ������ ������������� ����(���� �������)
			if (bSound)	PlaySound(szSoundName, hInst, SND_RESOURCE | SND_ASYNC);
			sn.snake.push_back(apple.point);					//����������� ����� ������
			apple.Taken();										//������ ����� �����
			score++;											//����������� ����
			UpdateStatusBar(hwnd, score, HighScore);			//��������� ������ ���������	
			if (Speed >=40)	{									//����������� ��������, ���� ��� ��� �� �����������
				Speed -= 2;							
				KillTimer(hwnd,1);
				SetTimer(hwnd, 1, Speed, NULL);
			}
		}

		//�������� �� ��������� ����
		if(GameOver(hwnd, hdc, fieldSize, sn)){
			KillTimer(hwnd,1);							//������������� ������
			string scr="��� ����: ";					//������� ��������� �� ��������� ���� � ����������
			scr += to_string((long long)score);			//��������� ����� � ���������� ������ ����� ����
			if (score > HighScore){						//��������� ����� �� ������
				ofile.open("Record.txt");
				ofile.clear();
				ofile << score;
				ofile.close();
				scr += " ����� ������!";
			}
			scr += "\n���������?";
			
			if(IDNO==MessageBox(hwnd, scr.c_str(),"���� ��������. ��� ����:" , MB_YESNO))
				SendMessage(hwnd,WM_DESTROY,0,0);
			else {
				DrawField(hwnd, hdc,fieldSize, Field);
				sn.Restart();
				score = 0;
				Speed = 76;
				UpdateStatusBar(hwnd, score, HighScore);
				SetTimer(hwnd, 1, Speed, NULL);
			}
		}// ����� �������� �� ��������� ���� if (GameOver)
		//���� ���� �� ��������� �������������� ������ � �����
		sn.DrawSnake(hwnd, hdc, SnakeSkin, Field);
		apple.DrawApple(hwnd, hdc, fieldSize, AppleSkin, sn);
		ReleaseDC(hwnd, hdc);
		break;

	case WM_KEYDOWN:
		if (!Moving) {			//���� �� ����� �����
			Moving = TRUE;
			switch(wParam) {	//������ ����������� ��������
			case VK_LEFT:		//���� �������� ������ ��������������� �������� ����������� �������� ������
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
			case VK_ESCAPE:					//���/����. �����
				Moving = FALSE;				
				if (!Pause) {				
					KillTimer(hwnd, 1);		//������������� ������
					Pause = TRUE;
				}
				else {
					SetTimer(hwnd, 1, Speed, NULL);			//��������� ������
					Pause = FALSE;
				}
				break;
			}// ��������� switch
		}//��������� if
		break;
	
		//��������� ��������� �� ������� ����
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case ID_GAME_NEW:								//����� ����
			hdc = GetDC(hwnd);
			DrawField(hwnd, hdc, fieldSize, Field);			
			sn.Restart();
			score = 0;
			Speed = 76;
			SetTimer(hwnd, 1, Speed, NULL);
			break;

		case IDM_GAME_EXIT:								//����� �� ����
			SendMessage(hwnd, WM_DESTROY, NULL, NULL);
			break;

		case IDM_SOUND_ON:								//��������� �����
			bSound = TRUE;
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_SOUND_ON, IDM_SOUND_OFF,
				LOWORD(wParam), MF_BYCOMMAND);
			EnableMenuItem(GetSubMenu(hMenu, 1),IDM_SOUND_ON, MF_BYCOMMAND| MFS_ENABLED);
			DrawMenuBar(hwnd);
			break;

		case IDM_SOUND_OFF:								//���������� �����
			bSound = FALSE;
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_SOUND_ON, IDM_SOUND_OFF,	
				LOWORD(wParam), MF_BYCOMMAND);
			EnableMenuItem(GetSubMenu(hMenu, 1),IDM_SOUND_OFF, MF_BYCOMMAND |  MFS_ENABLED); //�������� ����� "Sound off"
			DrawMenuBar(hwnd);
			break;
		case IDM_ABOUT:									//����� ���� About
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
			break;
		}
		break;

	case WM_DESTROY:									//����� �� ����������
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

//��������� ��������� ���� About
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
//������� ������ ���� � ������������� ��� ������ � ���������� ������������
void DrawField(HWND hwnd, HDC hDC, FieldSize& fs, HBITMAP Field) {
	RECT rect, rcSB;
	GetClientRect(hwnd,&rect);				//������� ���������� �������
	GetWindowRect(hwndStatusBar, &rcSB);	//������� ������ ���������
	int sbHeight = rcSB.bottom - rcSB.top;	//��������� ������� ������ ���������
	rect.bottom = rect.bottom - (sbHeight + rect.top);	//��������� ������ ������ ��������� ��� ��������� ���� �� ������

	HBRUSH FieldBrush = CreatePatternBrush(Field);				//������� ����� ��� ���������� ����
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC,FieldBrush);	//�������� ��������� ����� � ���������� ������
	FillRect(hDC,&rect,FieldBrush);								//������������� ����
	FrameRect(hDC,&rect,(HBRUSH)GetStockObject(BLACK_BRUSH));	//������� ����� ����
	if (!fs.Calculated){										//���� ����� �� ��������
		//const int Scale = 25;										//������ ������� ������
		const int Width = rect.right - rect.left;						//����� ����
		const int Height = rect.bottom - - rect.top;					//������ ����
		//const int Width = xVE;
		//const int Height = yVE;
	
		fs.Width = (int)Width/Scale;								//���������� ����� � ������ ���� � �������
		fs.Height = (int)Height/Scale;								//� ��������� FieldSize
		int x0 = rect.left, y0 = rect.top;							//������ ��������
	
		//������������� p���� ����p������ ��������� ���p������, ����p�� ��p������� 
		//�p���p�������� ���������� ������ � ������� ���p������ ��� GDI � ���p����� ���� X � Y.
		SetMapMode(hDC, MM_ISOTROPIC);			//���������� ������� �� ����
		//������������� �������� ���������� ��� ������� �������������� ��������� 
		SetWindowExtEx(hDC, Width, Height, NULL);	
		SetViewportExtEx(hDC, Width, Height,NULL);
		//������������� �������� ���������� ������� ���������
		SetViewportOrgEx(hDC, x0, y0, NULL);
		fs.Calculated = TRUE;
	}
	DeleteObject(SelectObject(hDC, hOldBrush));		//������� ��������� ����� � ��������������� ������
}

//������� ��������� ������������ � ��������� ���� ��� � �������
BOOL GameOver(HWND hwnd, HDC hdc,const FieldSize& fs, const Snake& sn){
	if ((sn.snake[0].x >= fs.Width) || (sn.snake[0].x < 0) || 
		(sn.snake[0].y >= fs.Height) || (sn.snake[0].y < 0))
		return TRUE;
	//��������� ������� ������ �� ����������� � ����� � ���, ��� �� ������������ 
	//��� ����������� �������������� �� ��� �� ������ ����, ��� ��������� 
	//�� �������������� ���� ��� ������ ����������� ������.
	for (int i=1; i < sn.snake.size()-2; i++)
		if (sn.snake[0].x==sn.snake[i].x && sn.snake[0].y==sn.snake[i].y)
				return TRUE;
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//������� ���������� ������ ���������
void UpdateStatusBar(HWND hwnd, int Score, int HighScore) {									
	int paneWidth;
	int aWidths[STATUS_BAR_PARTS];
	string text = "��� ����: ";
	RECT rect;

	GetClientRect(hwnd, &rect);

	text += to_string((long long)Score);
	paneWidth = rect.right / STATUS_BAR_PARTS;
	aWidths [0] = paneWidth;
	aWidths [1] = paneWidth * 2;

	SendMessage(hwndStatusBar, SB_SETPARTS, STATUS_BAR_PARTS, (LPARAM)aWidths);	//����� ������ �� ��� �����

	SendMessage(hwndStatusBar, SB_SETTEXT, 0, (LPARAM)text.c_str());			//������� ������� ���� � 1�� �����
	text = "������ ����: " + to_string((long long)HighScore);
	SendMessage(hwndStatusBar, SB_SETTEXT, 1, (LPARAM)text.c_str());			//������� ������ ���� �� 2�� �����
}