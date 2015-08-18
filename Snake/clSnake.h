#include <Windows.h>
#include <vector>

using namespace std;

const int Scale = 25;		// размер клекти

typedef struct {			//размер поля в клетках
 	int Width;
 	int Height;
	bool Calculated;
 } FieldSize;

enum Directon {LEFT = 0, RIGHT, UP, DOWN, STOP};

class Snake {
private:
	int Dir;				//Направление движения
public:
	vector<POINT> snake;	//"Хвост" змейки
	Snake();
	int GetDir() const{		
		return Dir;
	}
	void SetDir(int direction){
		Dir = direction;
	}
	void DrawSnake(HWND, HDC, HBITMAP, HBITMAP) const;	//Рисует змейку
	void Move();										//Движение змейки
	void Restart();											
};	

class Fructs
{
private:
	BOOL IsPlaced;		//Размещен ли фрукт на поле
public:
	POINT point;		//Координаты фрукта
	Fructs():IsPlaced(FALSE) 
	{}
    void DrawApple(HWND, HDC, FieldSize &fs, HBITMAP, Snake& const sn);		//Рисуем фрукт
	BOOL Taken(){												//Указывает что фрукт взят
		IsPlaced = FALSE;
		return IsPlaced;
	}
}; 
