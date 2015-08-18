#include <Windows.h>
#include <vector>

using namespace std;

const int Scale = 25;		// ������ ������

typedef struct {			//������ ���� � �������
 	int Width;
 	int Height;
	bool Calculated;
 } FieldSize;

enum Directon {LEFT = 0, RIGHT, UP, DOWN, STOP};

class Snake {
private:
	int Dir;				//����������� ��������
public:
	vector<POINT> snake;	//"�����" ������
	Snake();
	int GetDir() const{		
		return Dir;
	}
	void SetDir(int direction){
		Dir = direction;
	}
	void DrawSnake(HWND, HDC, HBITMAP, HBITMAP) const;	//������ ������
	void Move();										//�������� ������
	void Restart();											
};	

class Fructs
{
private:
	BOOL IsPlaced;		//�������� �� ����� �� ����
public:
	POINT point;		//���������� ������
	Fructs():IsPlaced(FALSE) 
	{}
    void DrawApple(HWND, HDC, FieldSize &fs, HBITMAP, Snake& const sn);		//������ �����
	BOOL Taken(){												//��������� ��� ����� ����
		IsPlaced = FALSE;
		return IsPlaced;
	}
}; 
