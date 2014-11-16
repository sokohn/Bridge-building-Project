#include "Util.h"

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