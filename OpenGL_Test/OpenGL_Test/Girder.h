#pragma once

class BOLT;
struct Color;

//TODO: capitalize this
//TODO: check if I spelled that right
class Girder
{
public:
	BOLT* Bolt1;
	BOLT* Bolt2;
	bool isFinished;
	bool Highlighted;
	bool isRoad;

	float StartingLength;

	float CurrentStress;
	float MaxStress;

	bool isActive;
	int Index;

	static const int GirderWeight = 100;
	static float GirderStrength;
	static float GirderSpringContsant;

	Girder();

	void GetDrawColor( Color* color );
	float GetStressForce( float currentLength );
	float getCurrentLength();
};
