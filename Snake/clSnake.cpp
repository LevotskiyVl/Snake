#include <Windows.h>
#include <vector>
#include "clSnake.h"
using namespace std;

Snake::Snake() {			//�����������
	Dir = STOP;				//����������� �������� �� ������
	int nTale = 4;			//����� ������+1 (��������� ������� �� ������������ � ������������ ��� ����������� ����)
	POINT p;				//��������� ����������
	for (int i=0; i < nTale; i++) {
		p.x = 10;
		p.y = 15+i;
		snake.push_back(p);
	}
}
//������� ������ ������ � ������������� �� ����������� �������� � SnakeSkin, 
//����������� FieldSkin ����������� ��� �������������� ���������� ��������,
//��� ������� ��������� ��� �������� ���� � ��������� �� �������������� ����
void Snake::DrawSnake(HWND hwnd, HDC hdc, HBITMAP SnakeSkin, HBITMAP FieldSkin) const {
	const int Scale = 25;
	HBRUSH SnakeBrush = CreatePatternBrush(SnakeSkin);				//������� ����� ��� ������
	HBRUSH FieldBrush = CreatePatternBrush(FieldSkin);				//������� ����� ��� ����
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, SnakeBrush);		//����������� ����� ����� � ���������� ������
	//������� ������ ���� ��� ������, ����� ��� �� ��������� ��������
	HPEN hPen = CreatePen(PS_NULL, 1, RGB(0,0,0));					
	//HPEN hFieldPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);					//������������� ����� ���� � ���������� ������

	int i;
	for (i=0; i < snake.size()-1; i++)								//������ ������ (����� ���������� ��������)
		Rectangle(hdc,snake[i].x * Scale, snake[i].y * Scale, 
				  (snake[i].x + 1) * Scale, (snake[i].y + 1) * Scale);
	//�������� ����� ��� ���������� ��������
	SelectObject(hdc, FieldBrush);
	//������ ��������� �������
	Rectangle(hdc,(snake[i].x)*Scale, (snake[i].y)*Scale, (snake[i].x+1)*Scale, (snake[i].y+1)*Scale);

	//������� ��������� ����� � ����, ������ ������������� ��������������
	DeleteObject(SnakeBrush);	
	DeleteObject(SelectObject(hdc, hOldBrush));
	DeleteObject(SelectObject(hdc, hOldPen));
}

//�������� ������
void Snake::Move(){
	if(GetDir() != STOP) {		//���� ����������� ������
		 //����������, � �����, ��� �������� �� ����� �������������� �� 
		for (int i = snake.size() - 1; i > 0; --i) {   
			snake[i].x = snake[i-1].x;
			snake[i].y = snake[i-1].y;
		}
	}
	//� ����������� �� ����������� ���������� "������"
	switch(GetDir()) {
	case LEFT:
		snake[0].x -= 1;
		break;
	case RIGHT: 
		snake[0].x += 1;
		break;
	case UP:
		snake[0].y -= 1;
		break;
	case DOWN:
		snake[0].y += 1;
		break;
	case STOP:
		snake[0].x = snake[0].x; 
		snake[0].y = snake[0].y; 
		break;
	default: break;
	}
}

//�������� ������
void Snake::Restart(){
	snake.clear();
	Dir = STOP;
	int nTale = 4;
	POINT p;
	for (int i=0; i < nTale; i++) {
		p.x = 15;
		p.y = 15+i;
		snake.push_back(p);
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//������� ������ Fructs
//������ ����� � ���������, ����� �� �� �������� �� ������
void Fructs::DrawApple(HWND hwnd, HDC hdc, FieldSize &fs, HBITMAP AppleSkin, Snake& const sn){
	RECT rect;
	GetClientRect(hwnd,&rect);									
	//FrameRect(hdc,&rect,(HBRUSH)GetStockObject(BLACK_BRUSH));

	HBRUSH AppleBrush = CreatePatternBrush(AppleSkin);			//������� ����� ��� ������
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, AppleBrush);	//�������� ����� � ���������� ������

	HPEN hPen = CreatePen(PS_NULL, 1, RGB(0,0,0));				//������� ������ ����
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);				
	
	//const int Width = rect.right - rect.left;
	//const int Height = rect.bottom - rect.top;
	//const int Scale = 25;
	BOOL NotOnSnake = FALSE;
	/*int M=1, N=1;
	if (fs.Width > 0 && fs.Height > 0) {
		N = (int)fs.Width;
		M = (int)fs.Height;
	}*/		
	
	//SelectObject(hdc, AppleBrush);
	//���� ����� �� �������� �� ���� ������� ��� � ��������� �����
	if (!IsPlaced) {
		//���� �� ��� ���,���� ����� �� ����� �� ��������� �� ������ ������
		do {
			point.x = rand() % fs.Width;
			point.y = rand() % fs.Height;
			NotOnSnake = TRUE;
			for (int i = sn.snake.size() - 1; i >= 0; --i) {
				if (sn.snake[i].x == point.x || sn.snake[i].y == point.y){ 
					NotOnSnake = FALSE;
					break;
				}
			}
		}while(!NotOnSnake);
	}
	//������ �����
	Rectangle(hdc, point.x*Scale, point.y*Scale, (point.x+1)*Scale, (point.y+1)*Scale);
	IsPlaced = TRUE;
	//������� ����� � ����
	DeleteObject(SelectObject(hdc, hOldBrush));
	DeleteObject(SelectObject(hdc, hOldPen));
}
