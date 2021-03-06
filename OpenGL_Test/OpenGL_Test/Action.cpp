#include "Action.h"
#include "Level.h"
#include "Bolt.h"
#include "Girder.h"

extern GIRDER DrawGirder;
extern bool isDrawingGirder;

ADD_GIRDER::ADD_GIRDER(float X1, float Y1, float X2, float Y2)
{
	B1.x = X1;
	B1.y = Y1;
	B2.x = X2;
	B2.y = Y2;
	AddedB1 = false;
	AddedB2 = false;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ADD_GIRDER::Perform(LEVEL* Level)
{
	if (Level->FindBolt(B1.x, B1.y) == NULL)
	{
		AddedB1 = true;
	}
	if (Level->FindBolt(B2.x, B2.y) == NULL)
	{
		AddedB2 = true;
	}

	BOLT* Bolt = Level->AddBolt(B1.x, B1.y, false);
	Level->AddGirder(Bolt, B2.x, B2.y);

	DrawGirder.Bolt1 = Level->FindBolt(B2.x, B2.y);
	RepositionGirderCursor();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ADD_GIRDER::Undo(LEVEL* Level)
{
	GIRDER* Girder = Level->FindGirder(B1.x, B1.y, B2.x, B2.y);
	Level->RemoveGirder(Girder);

	if (AddedB1)
	{
		Level->RemoveBolt(Level->FindBolt(B1.x, B1.y));
	}
	else
	{
		DrawGirder.Bolt1 = Level->FindBolt(B1.x, B1.y);
		isDrawingGirder = true;
		RepositionGirderCursor();
	}
	if (AddedB2)
	{
		Level->RemoveBolt(Level->FindBolt(B2.x, B2.y));
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ADD_GIRDER::FlipX()
{
	B1.x *= -1.0;
	B2.x *= -1.0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ADD_GIRDER::FlipY()
{
	B1.y *= -1.0;
	B2.y *= -1.0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

REMOVE_GIRDER::REMOVE_GIRDER(float X1, float Y1, float X2, float Y2)
{
	B1.x = X1;
	B1.y = Y1;
	B2.x = X2;
	B2.y = Y2;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void REMOVE_GIRDER::Perform(LEVEL* Level)
{
	GIRDER* Girder = Level->FindGirder(B1.x, B1.y, B2.x, B2.y);
	Level->RemoveGirder(Girder);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void REMOVE_GIRDER::Undo(LEVEL* Level)
{
	BOLT* Bolt = Level->AddBolt(B1.x, B1.y, false);
	Level->AddGirder(Bolt, B2.x, B2.y);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

REMOVE_BOLT::REMOVE_BOLT( float X, float Y )
{
	B1.x = X;
	B1.y = Y;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void REMOVE_BOLT::Perform(LEVEL* Level)
{
	BOLT* Bolt = Level->FindBolt(B1.x, B1.y);
	Level->RemoveBolt(Bolt);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void REMOVE_BOLT::Undo(LEVEL* Level)
{
	Level->AddBolt(B1.x, B1.y, false);
}