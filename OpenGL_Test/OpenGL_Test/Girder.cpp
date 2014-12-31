#include "Bolt.h"
#include "Girder.h"
#include <math.h>
#include "Util.h"

float GIRDER::GirderStrength = 2500.0f;
float GIRDER::GirderSpringContsant = 500.0f;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GIRDER::GetDrawColor( Color* color )
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
			color->green = maxf((GirderStrength - fabs(UsedStressor) ) / GirderStrength, 0 );
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

GIRDER::GIRDER()
{
	Bolt1 = NULL;
	Bolt2 = NULL;
	Highlighted = false;
	isFinished = false;
	isRoad = false;
	CurrentStress = 0;
	MaxStress = 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

float GIRDER::GetStressForce( float currentLength )
{
    // f = k x
    float dx = currentLength - StartingLength;
	return GirderSpringContsant * dx;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

float GIRDER::getCurrentLength()
{
	return sqrt(pow(Bolt2->x - Bolt1->x, 2) + pow(Bolt2->y - Bolt1->y, 2));

}