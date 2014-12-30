#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include <math.h>
#include <stack>
#include <vector>
#include <time.h>

#include "Action.h"
#include "Bolt.h"
#include "Girder.h"
#include "Level.h"
#include "Util.h"

using namespace std;


MOUSE Mouse;
SCREEN Screen;
LEVEL Level;
float ZoomLevel;
clock_t PrevTime;
float CameraX;
float CameraY;


//used to show where the next girder would be located
static GIRDER DrawGirder;
static BOLT DrawBolt(0,0);
static bool isDrawingGirder = false;

stack<ACTION*> UndoStack;
stack<ACTION*> RedoStack;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void changeViewport(int w, int h)
{
	glViewport(0,0,w,h);
	Screen.height = (float) h;
	Screen.width = (float) w;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void updateGame()
{
    glutPostRedisplay();
	clock_t CurTime;
	CurTime = clock();
	Level.Update( minf( (( (float)(CurTime - PrevTime) ) / CLOCKS_PER_SEC ),0.1) );
	PrevTime=CurTime;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void render()
{
	glClearColor(0.15f,0.15f,0.175f,0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	glOrtho( CameraX+(Screen.width / ZoomLevel )/-2.0 , CameraX+(Screen.width / ZoomLevel )/2.0, CameraY+(Screen.height / ZoomLevel )/-2.0, CameraY+(Screen.height / ZoomLevel )/2.0, -1, 1 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glPushMatrix();

	glEnable(GL_MULTISAMPLE_ARB);
	
	Level.Draw();

	//now draw the drawgirder and drawbolt
	if( isDrawingGirder )
	{
		//draw the bolt
		Color color;
		DrawBolt.GetDrawColor( &color );
		glLoadIdentity();
		glPushMatrix();
		glPointSize(5.0);
		glBegin(GL_POINTS);
		glColor3f( color.red ,color.green ,color.blue );  
		glVertex3f( DrawBolt.GetDrawX(), DrawBolt.GetDrawY(), 0.0f );
		glEnd();       
		glPopMatrix();

		//and then the girder
		glPushMatrix();
			//now draw the connection line
		glLineWidth(1.5f);
		glBegin(GL_LINES);
		glColor3f( 0.0f,1.0f,0.0f );
		glVertex3f( DrawGirder.Bolt1->GetDrawX(), DrawGirder.Bolt1->GetDrawY(), 0.0f);
		glVertex3f( DrawGirder.Bolt2->GetDrawX(), DrawGirder.Bolt2->GetDrawY(), 0.0f);
		glEnd();  
		glPopMatrix();

	}

	glPopMatrix();
    glutSwapBuffers();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MouseMoveHandler( int x, int y)
{
	Mouse.x = ((x - Screen.width/2.0) / ZoomLevel)+CameraX;
	Mouse.y = -1.0f*((y - Screen.height/2.0) / ZoomLevel)+CameraY;

	if( isDrawingGirder )
	{
		//update the draw location for the new girder
		
		//firstly, reproject the vector if it is too long
		int distance = ( pow( DrawGirder.Bolt1->x - Mouse.x,2.0f) + pow( DrawGirder.Bolt1->y - Mouse.y,2.0f) );
		if( distance > MaxGirderLength * MaxGirderLength )
		{
			float theta = atan2((DrawGirder.Bolt1->y - Mouse.y),(DrawGirder.Bolt1->x - Mouse.x ));
			Mouse.x = DrawGirder.Bolt1->x - cos(theta)*MaxGirderLength;
			if (MaxGirderLengthDirectional < fabs(cos(theta)*MaxGirderLength))
			{
				if (cos(theta) > 0)
				{
					Mouse.x = DrawGirder.Bolt1->x - MaxGirderLengthDirectional;
				}
				else
				{
					Mouse.x = DrawGirder.Bolt1->x + MaxGirderLengthDirectional;
				}
			}
			Mouse.y = DrawGirder.Bolt1->y - sin(theta)*MaxGirderLength;
			if (MaxGirderLengthDirectional < fabs(sin(theta)*MaxGirderLength))
			{
				if (sin(theta) > 0)
				{
					Mouse.y = DrawGirder.Bolt1->y - MaxGirderLengthDirectional;
				}
				else
				{
					Mouse.y = DrawGirder.Bolt1->y + MaxGirderLengthDirectional;
				}
			}
		}

		for(int i =0; i< Level.Bolts->size(); i++ )
		{
			BOLT* Bolt = (*Level.Bolts)[i];
			if( Bolt != NULL )
			{
				float distance = ( pow( Mouse.x - Bolt->x,2.0f) + pow( Mouse.y - Bolt->y,2.0f) );
				//fprintf(stderr, "distance: %f \n", distance);
				if( distance <= BOLT::collisionRadius * BOLT::collisionRadius )
				{
					Bolt->Highlighted = true;
					DrawBolt.x = Bolt->x;
					DrawBolt.y = Bolt->y;
					return;
				}
			}
		}

		DrawBolt.x = RoundToNearestGridMarker( Mouse.x );
		DrawBolt.y = RoundToNearestGridMarker( Mouse.y );
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MouseClickHandler(int button, int state, int x, int y)
{
	//mouse scroll for zooming
	if ( state == GLUT_UP && button == 3 )
	{
		ZoomLevel *= 1.1;
		if(ZoomLevel > 1)
		{
			ZoomLevel = 1;
		}
	}
	else if( state == GLUT_UP && button == 4 )
	{
		ZoomLevel /= 1.1;
		if(ZoomLevel < 0.25 )
		{
			ZoomLevel = 0.25;
		}
	}

	if(IsSimulating())
	{
		//TODO: Once I get some menu UI in, make sure they can still function
		return;
	}
	float NormMouseLocX = ((x - Screen.width/2.0) / ZoomLevel)+CameraX;
	float NormMouseLocY = -1.0f*((y - Screen.height/2.0) / ZoomLevel)+CameraY;
	BOLT* HitBolt = NULL;

	//iterate through all of the bolts and see if we are within the collision radius of one of them
	for(int i =0; i< Level.Bolts->size(); i++ )
	{
		BOLT* Bolt = (*Level.Bolts)[i];
		if( Bolt != NULL )
		{
			float distance = ( pow( NormMouseLocX - Bolt->x,2.0f) + pow( NormMouseLocY - Bolt->y,2.0f) );
			//fprintf(stderr, "distance: %f \n", distance);
			if( distance <= BOLT::collisionRadius * BOLT::collisionRadius )
			{
				Bolt->Highlighted = true;
				HitBolt = Bolt;
				break;
			}
		}
	}

	if( button == GLUT_RIGHT_BUTTON  && state == GLUT_UP )
	{
		if( isDrawingGirder )
		{
			isDrawingGirder = false;
		}
		else
		{
			//we might be removing a bolt
			if( HitBolt != NULL )
			{
				//check if we can delete this bolt
				if( !HitBolt->CanBeDeleted() )
				{
					return;
				}

				REMOVE_BOLT* RemoveBolt = new REMOVE_BOLT(HitBolt->x, HitBolt->y);

				UndoStack.push(RemoveBolt);
				RemoveBolt->Perform(&Level);
				while (!RedoStack.empty())
				{
					RedoStack.pop();
				}

				//now finally delete the bolt
				delete HitBolt;
				return;

			}
		}
	}

	if(button == GLUT_LEFT_BUTTON  && state == GLUT_UP )
	{
		GIRDER* girder = NULL;
		//check to see if we need to add a new girder, or if we are finishing the current one

		if( !isDrawingGirder )
		{
			if( HitBolt == NULL )
			{
				//can't start without a starting bolt, so bail
				return;
			}
			isDrawingGirder = true;
			DrawGirder.Bolt1 = HitBolt;
		}
		else
		{
			//make sure the start and end points aren't the same 
			if( HitBolt == DrawGirder.Bolt1 )
			{
				return;
			}

			if( HitBolt == NULL )
			{
				//round the bolt's location to land on a grid spot
				ADD_GIRDER* AddGirder = new ADD_GIRDER(DrawGirder.Bolt1->x, DrawGirder.Bolt1->y, DrawBolt.x, DrawBolt.y);
				UndoStack.push(AddGirder);
				AddGirder->Perform(&Level);
				DrawGirder.Bolt1 = Level.FindBolt(DrawBolt.x, DrawBolt.y);
				while (!RedoStack.empty())
				{
					RedoStack.pop();
				}
			}
			else
			{
				//eventually put in a check here to make sure there isn't already a girder that connects these two bolts
				girder = Level.AddGirder(DrawGirder.Bolt1, HitBolt);
				ADD_GIRDER* AddGirder = new ADD_GIRDER(DrawGirder.Bolt1->x, DrawGirder.Bolt1->y, HitBolt->x, HitBolt->y);
				UndoStack.push(AddGirder);
				AddGirder->Perform(&Level);
				DrawGirder.Bolt1 = HitBolt;
				while (!RedoStack.empty())
				{
					RedoStack.pop();
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void HandleKeyboard(unsigned char key, int x, int y) 
{
	if (key == 27)
	{
		exit(0);
	}
	else if( key == 't' || key == 'T' )
	{
		SetSimulating(!IsSimulating());
		isDrawingGirder = false;
	}
	else if( key == 'p' || key == 'P' )
	{
		if( IsSimulating() )
		{
			SetPaused( !IsPaused() );
		}
	}
	else if( key == 'v' || key == 'V' )
	{
		Level.SerializeLevel("TestFile.txt");
	}
	else if( key == 'l' || key == 'L' )
	{
		Level.DeserializeLevel("TestFile.txt");
	}
	else if( key == 'w' || key == 'W' )
	{
		CameraY+=10/ZoomLevel;
		if(CameraY > 1000)
		{
			CameraY = 1000;
		}
	}
	else if( key == 'a' || key == 'A' )
	{
		CameraX-=10/ZoomLevel;
		if(CameraX < -2000)
		{
			CameraX = -2000;
		}
	}
	else if( key == 's' || key == 'S' )
	{
		CameraY-=10/ZoomLevel;
		if(CameraY <  -750 )
		{
			CameraY = -750;
		}
	}
	else if( key == 'd' || key == 'D' )
	{
		CameraX+=10/ZoomLevel;
		if(CameraX > 2000)
		{
			CameraX = 2000;
		}
	}
	else if (key == 'u' || key == 'U')
	{
		if (UndoStack.size() > 0)
		{
			UndoStack.top()->Undo(&Level);
			RedoStack.push(UndoStack.top());
			UndoStack.pop();
		}
	}
	else if (key == 'r' || key == 'R')
	{
		if (RedoStack.size() > 0)
		{
			RedoStack.top()->Perform(&Level);
			UndoStack.push(RedoStack.top());
			RedoStack.pop();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int main( int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE );

	glutInitWindowSize( 1024, 768 );
	Screen.width = 1024;
	Screen.height = 768;
	CameraX = 0;
	CameraY = 0;

	glutCreateWindow( "Bridge clone" );
	glEnable(GL_CLIP_DISTANCE5);
	glEnable(GL_MULTISAMPLE_ARB);

	glutReshapeFunc( changeViewport );
	glutDisplayFunc( render );
	glutMouseFunc( MouseClickHandler );
	glutPassiveMotionFunc( MouseMoveHandler );
	glutKeyboardFunc(HandleKeyboard);
	glutIdleFunc(updateGame);
	PrevTime = clock();

	DrawGirder.Bolt2 = &DrawBolt;

	ZoomLevel = 0.5;

	SetSimulating(false);

	GLenum err = glewInit();
	if( err != GLEW_OK )
	{
		fprintf(stderr, "GLEW error");
		return 1;
	}

	glutMainLoop();
	return 0;

}