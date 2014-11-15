#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include <math.h>
#include <vector>
#include "Bolt.h"
#include "Util.h"

using namespace std;


static MOUSE Mouse;
SCREEN Screen;

class Girder
{
public:
	Bolt* Bolt1;
	Bolt* Bolt2;
	bool isFinished;

	Girder()
	{
		Bolt1 = NULL;
		Bolt2 = NULL;
		isFinished = false;
	}
};


static vector<Girder*>* Girders;
static vector<Bolt*>* Bolts;

void changeViewport(int w, int h)
{
	glViewport(0,0,w,h);
	Screen.height = (float) h;
	Screen.width = (float) w;
}

void updateGame()
{
    glutPostRedisplay();

	//iterate through all of the bolts and see if we are within the collision radius of one of them
	for(int i =0; i< Bolts->size(); i++ )
	{
		Bolt* Bolt = (*Bolts)[i];
		if( Bolt != NULL )
		{
			float distance = ( pow( Mouse.x - Bolt->x,2.0f) + pow( Mouse.y - Bolt->y,2.0f) );
			if( distance <= Bolt::collisionRadius * Bolt::collisionRadius )
			{
				Bolt->Highlighted = true;
			}
			else
			{
				Bolt->Highlighted = false;
			}
		}
	}
}

void drawGrid()
{
	int NumLines = Screen.width/10;
	//vertical lines
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0.0f,0.0f,0.5f);
	for(int i = 0; i< NumLines; i++ )
	{
		glLoadIdentity();
		glPushMatrix();
		if( i % 10 == 0 )
		{
			//major line
			glLineWidth(0.25f);			
			glColor3f( 0.75f,0.75f, 0.75f );
		}
		else if( i % 5 == 0 )
		{
			//medium line
			glLineWidth(0.25f);			
			glColor3f( 0.5f,0.5f, 0.5f );
		}
		else
		{
			//minor lines
			glLineWidth(0.25f);			
			glColor3f( 0.0f,0.0f, 0.0f );
		}
		
		glBegin(GL_LINES);
		glVertex3f( (i - NumLines/2.0f) /(NumLines/2.0f) , 1.0f , 0.0f);
		glVertex3f( (i - (NumLines/2.0f)) /(NumLines/2.0f), -1.0f, 0.0f);
		glEnd();  
		
		glPopMatrix();
	}

	NumLines = Screen.height/10;
	for(int i = 0; i< NumLines; i++ )
	{
		glLoadIdentity();
		glPushMatrix();
		if( i % 10 == 0 )
		{
			//major line
			glLineWidth(0.25f);			
			glColor3f( 0.75f,0.75f, 0.75f );
		}
		else if( i % 5 == 0 )
		{
			//medium line
			glLineWidth(0.25f);			
			glColor3f( 0.5f,0.5f, 0.5f );
		}
		else
		{
			//minor lines
			glLineWidth(0.25f);			
			glColor3f( 0.0f,0.0f, 0.0f );
		}
		
		glBegin(GL_LINES);
		glVertex3f( 1.0f , (i - (NumLines/2.0f)) /(NumLines/2.0f), 0.0f);
		glVertex3f( -1.0f, (i - (NumLines/2.0f)) /(NumLines/2.0f), 0.0f);
		glEnd();  
		
		glPopMatrix();
	}
	glPopMatrix();
}

void render()
{
	glClearColor(0.3f,0.3f,0.4f,1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	glOrtho( 0, Screen.width, -1*Screen.height, Screen.height, -1, 1 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glPushMatrix();

	glEnable(GL_MULTISAMPLE_ARB);
	drawGrid();

	for(int i =0; i< Bolts->size(); i++ )
	{
		
		Bolt* Bolt = (*Bolts)[i];
		Color color;
		Bolt->GetDrawColor( &color );

		glLoadIdentity();
		glPushMatrix();
		//draw the first point
		glPushMatrix();
		//glTranslatef( Bolt->GetDrawX(), Bolt->GetDrawY(), 0.0f );
		glPointSize(5.0);
		glBegin(GL_POINTS);
		glColor3f( color.red ,color.green ,color.blue );  
		//glVertex3f( 0.0f, 0.0f, 0.0f);              // Top
		glVertex3f( Bolt->GetDrawX(), Bolt->GetDrawY(), 0.0f );
		glEnd();       
		glPopMatrix();
	}

	for(int i =0; i<Girders->size(); i++ )
	{
		glLoadIdentity();
		glPushMatrix();
		Girder* girder = (*Girders)[i];

		//only draw the second end point and the connection if it is finished
		if( girder->isFinished )
		{
			//now draw the connection line
			glLineWidth(1.5f);
			glBegin(GL_LINES);
			glColor3f( 0.0f,1.0f,0.0f );
			glVertex3f( girder->Bolt1->GetDrawX(), girder->Bolt1->GetDrawY(), 0.0f);
			glVertex3f( girder->Bolt2->GetDrawX(), girder->Bolt2->GetDrawY(), 0.0f);
			glEnd();  
		}
		glPopMatrix();

	}
	glPopMatrix();
    glutSwapBuffers();
}

void MouseMoveHandler( int x, int y)
{
	float NormMouseLocX = ( ( 2*x - Screen.width )/Screen.width ) * Screen.width;
	float NormMouseLocY = ( -1.0f*( ( 2*y - Screen.height )/Screen.height ) )* Screen.height;
	
	Mouse.x = NormMouseLocX;
	Mouse.y = NormMouseLocY;
}

void MouseClickHandler(int button, int state, int x, int y)
{
	float NormMouseLocX = ( ( 2*x - Screen.width )/Screen.width ) * Screen.width;
	float NormMouseLocY = ( -1.0f*( ( 2*y - Screen.height )/Screen.height ) )* Screen.height;
	Bolt* HitBolt = NULL;

	//iterate through all of the bolts and see if we are within the collision radius of one of them
	for(int i =0; i< Bolts->size(); i++ )
	{
		Bolt* Bolt = (*Bolts)[i];
		if( Bolt != NULL )
		{
			float distance = ( pow( NormMouseLocX - Bolt->x,2.0f) + pow( NormMouseLocY - Bolt->y,2.0f) );
			if( distance <= Bolt::collisionRadius * Bolt::collisionRadius )
			{
				Bolt->Highlighted = true;
				HitBolt = Bolt;
				break;
			}
		}
	}

	if(button == GLUT_LEFT_BUTTON  && state == GLUT_UP )
	{
		Girder* girder = NULL;

		//check to see if we need to add a new girder, or if we are finishing the current one
		bool needStartNewGirder = false;
		if( Girders->size() == 0 )
		{
			needStartNewGirder = true;
		}
		else if( ( *Girders->rbegin() ) == NULL )
		{
			needStartNewGirder = true;
		}
		else if( ( *Girders->rbegin() )->isFinished )
		{
			needStartNewGirder = true;
		}

		if( needStartNewGirder )
		{
			if( HitBolt == NULL )
			{
				//can't start without a starting bolt, so bail
				return;
			}
			girder = new Girder();
			girder->Bolt1 = HitBolt;
			Girders->push_back(girder);
		}
		else
		{
			girder = ( *Girders->rbegin() );
			//check to see if the second end is on a bolt that exists
			if( HitBolt == NULL )
			{
				girder->Bolt2 = new Bolt( NormMouseLocX, NormMouseLocY );
				Bolts->push_back(girder->Bolt2);
				girder->isFinished = true;
			}
			else
			{
				//is it our first bolt?
				if( HitBolt == girder->Bolt1 )
				{
					//can't connect to itself
				}
				else
				{
					//eventually put in a check here to make sure there isn't already a girder that connects these two bolts
					girder->Bolt2 = HitBolt;
					girder->isFinished = true;
				}
			}
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

	Girders = new vector<Girder*>();
	Bolts = new vector<Bolt*>();

	glutReshapeFunc( changeViewport );
	glutDisplayFunc( render );
	glutMouseFunc( MouseClickHandler );
	glutPassiveMotionFunc( MouseMoveHandler );
	glutIdleFunc(updateGame);

	Bolt* bolt = new Anchor(Screen.width,100);
	Bolts->push_back(bolt);
	bolt = new Anchor(-Screen.width,500);
	Bolts->push_back(bolt);
	bolt = new Anchor(700,100);
	Bolts->push_back(bolt);


	GLenum err = glewInit();
	if( err != GLEW_OK )
	{
		fprintf(stderr, "GLEW error");
		return 1;
	}

	glutMainLoop();
	return 0;

}