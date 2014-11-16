#include "Bolt.h"
#include "Girder.h"
#include <math.h>
#include "Util.h"


void Girder::GetDrawColor( Color* color )
{
	if( Highlighted )
	{
		color->red = 0.7f;
		color->green = 0.7f;
		color->blue = 0.2f;
	}
	else
	{
		if( ShowStress() )
		{
			float UsedStressor = 0;
			if( IsSimulating() )
			{
				UsedStressor = CurrentStress;
			}
			else
			{
				UsedStressor = MaxStress;
			}
			color->red = maxf(-1.0f*UsedStressor/GirderStrength,0);
			color->green = ( GirderStrength - UsedStressor*2 ) / GirderStrength;
			color->blue = maxf(UsedStressor/GirderStrength,0);
		}
		else if( isRoad )
		{
			color->red = 1.0f;
			color->green = 1.0f;
			color->blue = 1.0f;
		}
		else
		{
			color->red = 0.75f;
			color->green = 0.75f;
			color->blue = 0.75f;
		}
	}
}

Girder::Girder()
{
	Bolt1 = NULL;
	Bolt2 = NULL;
	Highlighted = false;
	isFinished = false;
	isRoad = false;
	CurrentStress = 0;
	MaxStress = 0;
}

float Girder::GetStressForce( float currentLength )
{
    // f = k x
    float dx = currentLength - StartingLength;
    return 100 * dx;
}