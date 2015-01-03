#include "..\Util.h"
#include "UI_Base.h"

typedef void(*ClickFunc)(void);
typedef void(*ToggleFunc)(bool);

class TEXT_NODE : public UI_BASE
{
public:
	TEXT_NODE(char* _Title, float X, float Y);
	TEXT_NODE(char* _Title, Vector2D Loc);

	virtual void Draw();
protected:
	TEXT_NODE()
	{
		Title = 0;
		Location.x = 0;
		Location.y = 0;
	}
	Vector2D Location;
	char* Title;
};

class BUTTON_BASE : public TEXT_NODE
{
public:
	virtual bool Intersects(float X, float Y);
	virtual void Hover();
	virtual void Unhover();
protected:
	BUTTON_BASE()
	{
		Title = 0;
		Location.x = 0;
		Location.y = 0;
		TextWidth = 0;
		TextHeight = 0;
		MouseHover = false;
	}
	bool MouseHover;
	const int TextPaddingSpace = 5;
	float TextWidth;
	float TextHeight;
};

class BUTTON : public BUTTON_BASE
{
public:
	BUTTON(char* _Title, float X, float Y, ClickFunc _Func);
	BUTTON(char* _Title, Vector2D Loc, ClickFunc _Func);

	virtual void Draw();
protected:
	ClickFunc clickFunc;
};

class TOGGLE : public BUTTON_BASE
{
public:
	TOGGLE(char* _Title, float X, float Y, ToggleFunc _Func);
	TOGGLE(char* _Title, Vector2D Loc, ToggleFunc _Func);

	virtual void Draw();
protected:
	ToggleFunc clickFunc;
};
