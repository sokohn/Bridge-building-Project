#include "Bolt.h"
#include "Girder.h"
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
			color->red = 0.0f;
			color->green = 1.0f;
			color->blue = 0.0f;
		}
		else if( isRoad )
		{
			color->red = 1.0f;
			color->green = 1.0f;
			color->blue = 1.0f;
		}
		else
		{
			color->red = 0.85f;
			color->green = 0.85f;
			color->blue = 0.85f;
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
}