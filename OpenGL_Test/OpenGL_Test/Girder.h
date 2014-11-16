#pragma once

class BOLT;
struct Color;

class Girder
{
public:
	BOLT* Bolt1;
	BOLT* Bolt2;
	bool isFinished;
	bool Highlighted;
	bool isRoad;

	bool isActive;

	Girder();

	void GetDrawColor( Color* color );
};
