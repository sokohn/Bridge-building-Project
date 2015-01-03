#pragma once

class UI_BASE
{
public:
	virtual void Draw() { ; }
	virtual bool Intersects(float X, float Y) { return false; }
	virtual void Hover() { ; }
	virtual void Unhover() { ; }
	virtual void Click() { ; }
};