#include "Bolt.h"
#include "Util.h"

float Bolt::GetDrawX()
{
	return x;//Screen.width;
}

float Bolt::GetDrawY()
{
	return y;//Screen.height;
}

void Bolt::GetDrawColor( Color* color )
{
	color->Reset();

	if( Highlighted )
	{
		color->red = 0.7f;
		color->green = 0.7f;
		color->blue = 0.2f;
	}
	else
	{
		color->red = 0.5f;
		color->green = 0.5f;
		color->blue = 0.5f;
	}
}

void Anchor::GetDrawColor( Color* color )
{
	color->Reset();

	if( Highlighted )
	{
		color->red = 1.0f;
		color->green = 1.0f;
		color->blue = 0.5f;
	}
	else
	{
		color->red = 0.75f;
		color->green = 0.75f;
		color->blue = 0.75f;
	}
}