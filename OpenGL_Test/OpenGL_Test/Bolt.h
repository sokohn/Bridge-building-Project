#pragma once
#include <vector>


struct Color;
class Girder;

class BOLT
{
public:
	float x;
	float y;
	bool Highlighted;
	std::vector<Girder*> AttachedGirders;

	BOLT(float _x, float _y);
	~BOLT();

	static const int collisionRadius = 15;

	float GetDrawX();
	float GetDrawY();
	virtual void GetDrawColor( Color* color );
	virtual bool CanBeDeleted();
};

class Anchor : public BOLT
{
public:
	Anchor(float _x, float _y) : BOLT(_x, _y)
	{
	}
	void GetDrawColor( Color* color );
	bool CanBeDeleted();
};