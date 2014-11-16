#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include <math.h>
#include <vector>
#include "Bolt.h"
#include "Girder.h"
#include "Level.h"
#include "Util.h"

using namespace std;


MOUSE Mouse;
SCREEN Screen;
LEVEL Level;
float ZoomLevel;


//used to show where the next girder would be located
static Girder DrawGirder;
static BOLT DrawBolt(0,0);
static bool isDrawingGirder = false;

void changeViewport(int w, int h)
{
	glViewport(0,0,w,h);
	Screen.height = (float) h;
	Screen.width = (float) w;
}

void updateGame()
{
    glutPostRedisplay();

	Level.Update(0.001f);
}


void render()
{
	glClearColor(0.15f,0.15f,0.175f,0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	glOrtho( 0, Screen.width / ZoomLevel, 0, Screen.height / ZoomLevel, -1, 1 );

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

void MouseMoveHandler( int x, int y)
{
	//float NormMouseLocX = ( ( 2*x - Screen.width )/Screen.width ) * Screen.width;
	//float NormMouseLocY = ( -1.0f*( ( 2*y - Screen.height )/Screen.height ) )* Screen.height;
	
	Mouse.x = x / ZoomLevel;
	Mouse.y = (Screen.height-y) / ZoomLevel;

	if( isDrawingGirder )
	{
		//update the draw location for the new girder
		
		//firstly, reproject the vector if it is too long
		int distance = ( pow( DrawGirder.Bolt1->x - Mouse.x,2.0f) + pow( DrawGirder.Bolt1->y - Mouse.y,2.0f) );
		if( distance > MaxGirderLength * MaxGirderLength )
		{
			float theta = atan2((DrawGirder.Bolt1->y - Mouse.y),(DrawGirder.Bolt1->x - Mouse.x ));
			Mouse.x = DrawGirder.Bolt1->x - cos(theta)*MaxGirderLength;
			Mouse.y = DrawGirder.Bolt1->y - sin(theta)*MaxGirderLength;
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

void MouseClickHandler(int button, int state, int x, int y)
{
	if(IsSimulating())
	{
		//TODO: Once I get some menu UI in, make sure they can still function
		return;
	}
	float NormMouseLocX = x/ ZoomLevel;
	float NormMouseLocY = (Screen.height-y) / ZoomLevel;
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

				Level.RemoveBolt(HitBolt);

				//now finally delete the bolt
				delete HitBolt;
				return;

			}
		}
	}

	if(button == GLUT_LEFT_BUTTON  && state == GLUT_UP )
	{
		Girder* girder = NULL;

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
				girder = Level.AddGirder(DrawGirder.Bolt1, DrawBolt.x, DrawBolt.y );
				if( girder == NULL )
				{
					//must have failed
					return;
				}
			}
			else
			{
				//eventually put in a check here to make sure there isn't already a girder that connects these two bolts
				girder = Level.AddGirder(DrawGirder.Bolt1, HitBolt);
				if( girder == NULL )
				{
					//must have failed
					return;
				}
			}

			//now set the most recently used bolt as the starting point for the next girder
			DrawGirder.Bolt1 = girder->Bolt2;
		}
	}
}

void HandleKeyboard(unsigned char key, int x, int y) 
{
	if (key == 27)
	{
		exit(0);
	}
	else if( key == 's' || key == 'S' )
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
}

int main( int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE );

	glutInitWindowSize( 1024, 768 );
	Screen.width = 1024;
	Screen.height = 768;

	glutCreateWindow( "Bridge clone" );
	glEnable(GL_CLIP_DISTANCE5);
	glEnable(GL_MULTISAMPLE_ARB);

	glutReshapeFunc( changeViewport );
	glutDisplayFunc( render );
	glutMouseFunc( MouseClickHandler );
	glutPassiveMotionFunc( MouseMoveHandler );
	glutKeyboardFunc(HandleKeyboard);
	glutIdleFunc(updateGame);

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