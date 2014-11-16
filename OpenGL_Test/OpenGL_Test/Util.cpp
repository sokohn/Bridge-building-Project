#include "Util.h"
#include "Level.h"

extern LEVEL Level;

bool Simulating = false;
bool Paused = false;

bool ShowStress()
{
	return true;
}

float RoundToNearestGridMarker(float loc)
{
	int num = (int)loc;
	if( num % GridSpacing < GridSpacing/2)
	{
		return num - num % GridSpacing;
	}
	else
	{
		return num + GridSpacing - num % GridSpacing;
	}
}

bool IsSimulating()
{
	return Simulating;
}

void SetSimulating(bool Sim)
{
	if( Simulating != Sim )
	{
		if( Sim )
		{
			Level.StartSimulation();
		}
		else
		{
			Level.EndSimulation();
		}
	}
	Simulating = Sim;
}

bool IsPaused()
{
	return Paused;
}

void SetPaused(bool Pause)
{
	Paused = Pause;
}

float maxf(float x, float y)
{
	if(x > y )
	{
		return x;
	}

	return y;
}