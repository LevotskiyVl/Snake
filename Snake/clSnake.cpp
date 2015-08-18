#include <Windows.h>
#include <vector>
#include "clSnake.h"
using namespace std;

Snake::Snake() {			//конструктор
	Dir = STOP;				//Направление движения не задано
	int nTale = 4;			//Длина змейки+1 (последний элемент не отображается и используется для перерисовки поля)
	POINT p;				//Начальные координаты
	for (int i=0; i < nTale; i++) {
		p.x = 10;
		p.y = 15+i;
		snake.push_back(p);
	}
}
//Функции рисует змейку и разукрашивает ее изображение заданным в SnakeSkin, 
//изображение FieldSkin испльзуется для разукрашивания последнего элемента,
//что убирает оставляем при движении след и позволяет не перерисовывать поле
void Snake::DrawSnake(HWND hwnd, HDC hdc, HBITMAP SnakeSkin, HBITMAP FieldSkin) const {
	const int Scale = 25;
	HBRUSH SnakeBrush = CreatePatternBrush(SnakeSkin);				//Создаем кисть для змейки
	HBRUSH FieldBrush = CreatePatternBrush(FieldSkin);				//Создаем кисть для поля
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, SnakeBrush);		//Уставливаем новую кисть и запоминаем старую
	//Создаем пустое перо для змейки, чтобы она не выглядела сплошной
	HPEN hPen = CreatePen(PS_NULL, 1, RGB(0,0,0));					
	//HPEN hFieldPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);					//Устанавливаем новое перо и запоминаем старое

	int i;
	for (i=0; i < snake.size()-1; i++)								//Рисуем змейку (кроме последнего элемента)
		Rectangle(hdc,snake[i].x * Scale, snake[i].y * Scale, 
				  (snake[i].x + 1) * Scale, (snake[i].y + 1) * Scale);
	//Выбираем кисть для последнего элемента
	SelectObject(hdc, FieldBrush);
	//Рисуем последний элемент
	Rectangle(hdc,(snake[i].x)*Scale, (snake[i].y)*Scale, (snake[i].x+1)*Scale, (snake[i].y+1)*Scale);

	//удаляем созданные кисти и перо, заодно устанавливаем первоначальные
	DeleteObject(SnakeBrush);	
	DeleteObject(SelectObject(hdc, hOldBrush));
	DeleteObject(SelectObject(hdc, hOldPen));
}

//движение змейки
void Snake::Move(){
	if(GetDir() != STOP) {		//Если направление задано
		 //продвигаем, с конца, все элементы на места предшествующим им 
		for (int i = snake.size() - 1; i > 0; --i) {   
			snake[i].x = snake[i-1].x;
			snake[i].y = snake[i-1].y;
		}
	}
	//В зависимости от направления продвигаем "голову"
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

//Обнуляем змейку
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
//ФУНКЦИИ КЛАССА Fructs
//Рисуем фрукт и проверяем, чтобы он не появился на змейке
void Fructs::DrawApple(HWND hwnd, HDC hdc, FieldSize &fs, HBITMAP AppleSkin, Snake& const sn){
	RECT rect;
	GetClientRect(hwnd,&rect);									
	//FrameRect(hdc,&rect,(HBRUSH)GetStockObject(BLACK_BRUSH));

	HBRUSH AppleBrush = CreatePatternBrush(AppleSkin);			//Создаем кисть для фрукта
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, AppleBrush);	//Выбираем кисть и запоминаем старую

	HPEN hPen = CreatePen(PS_NULL, 1, RGB(0,0,0));				//Создаем пустое перо
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
	//Если фрукт не размещен на поле создаем его в случайной точке
	if (!IsPlaced) {
		//Цикл до тех пор,пока фрукт не будет на свободной от змейки клетке
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
	//Рисуем фрукт
	Rectangle(hdc, point.x*Scale, point.y*Scale, (point.x+1)*Scale, (point.y+1)*Scale);
	IsPlaced = TRUE;
	//удаляем кисть и перо
	DeleteObject(SelectObject(hdc, hOldBrush));
	DeleteObject(SelectObject(hdc, hOldPen));
}
