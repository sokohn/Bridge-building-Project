#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include <math.h>
#include "Bolt.h"
#include "Girder.h"
#include "Level.h"
#include "Util.h"

extern MOUSE Mouse;

LEVEL::LEVEL()
{
	Girders = new std::vector<Girder*>();
	Bolts = new std::vector<BOLT*>();

	
	BOLT* bolt = new Anchor(480,480);
	Bolts->push_back(bolt);
}

void LEVEL::Update(float DeltaTime)
{
	//deal with the delta time later when we get to actually simulating physics

	//iterate through all of the bolts and see if we are within the collision radius of one of them
	for(int i =0; i< Bolts->size(); i++ )
	{
		BOLT* Bolt = (*Bolts)[i];
		if( Bolt != NULL )
		{
			float distance = ( pow( Mouse.x - Bolt->x,2.0f) + pow( Mouse.y - Bolt->y,2.0f) );
			if( distance <= BOLT::collisionRadius * BOLT::collisionRadius )
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
	int NumLines = Screen.width/GridSpacing;
	//vertical lines
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0.0f,0.0f,0.5f);
	for(int i = 0; i< NumLines; i++ )
	{
		glLoadIdentity();
		glPushMatrix();
		if( i % 8 == 0 )
		{
			//major line
			glLineWidth(0.25f);			
			glColor3f( 0.55f,0.555f, 0.55f );
		}
		else if( i % 2 == 0 )
		{
			//medium line
			glLineWidth(0.25f);			
			glColor3f( 0.35f,0.35f, 0.35f );
		}
		else
		{
			//minor lines
			glLineWidth(0.25f);			
			glColor3f( 0.0f,0.0f, 0.0f );
		}
		
		glBegin(GL_LINES);
		glVertex3f( i*GridSpacing , 0 , 0.0f);
		glVertex3f( i*GridSpacing, Screen.height, 0.0f);
		glEnd();  
		
		glPopMatrix();
	}

	NumLines = Screen.height/GridSpacing;
	for(int i = 0; i< NumLines; i++ )
	{
		glLoadIdentity();
		glPushMatrix();
		if( i % 8 == 0 )
		{
			//major line
			glLineWidth(0.25f);			
			glColor3f( 0.75f,0.75f, 0.75f );
		}
		else if( i % 2 == 0 )
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
		glVertex3f( 0.0f , i*GridSpacing, 0.0f);
		glVertex3f( Screen.width, i*GridSpacing, 0.0f);
		glEnd();  
		
		glPopMatrix();
	}
	glPopMatrix();
}

void LEVEL::Draw()
{
	drawGrid();

	for(int i =0; i< Bolts->size(); i++ )
	{
		BOLT* Bolt = (*Bolts)[i];
		Color color;
		Bolt->GetDrawColor( &color );

		glLoadIdentity();
		glPushMatrix();
		glPointSize(8.0);
		glBegin(GL_POINTS);
		glColor3f( color.red ,color.green ,color.blue );  
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
			Color color;
			girder->GetDrawColor( &color );
			glLineWidth(1.5f);
			glBegin(GL_LINES);
			glColor3f( color.red ,color.green ,color.blue ); 
			glVertex3f( girder->Bolt1->GetDrawX(), girder->Bolt1->GetDrawY(), 0.0f);
			glVertex3f( girder->Bolt2->GetDrawX(), girder->Bolt2->GetDrawY(), 0.0f);
			glEnd();  
		}
		glPopMatrix();
	}
}

bool LEVEL::AddGirder( BOLT* Bolt1, float x, float y )
{
	return true;
}

bool LEVEL::AddGirder( BOLT* Bolt1, BOLT* Bolt2 )
{
	return true;
}

bool LEVEL::RemoveGirder()
{

	return true;
}

bool LEVEL::RemoveBolt( BOLT* bolt )
{
	//first, lets remove all of its girders from the girder vector
	for(int i = 0; i < bolt->AttachedGirders.size(); i++)
	{
		//now go through Girders and look for the this girder, then remove it
		for( std::vector<Girder*>::iterator it = Girders->begin() ; it != Girders->end(); ++it )
		{
			if( (*it) == bolt->AttachedGirders[i] )
			{
				Girders->erase(it);
				break;
			}
		}

		//now go through the girder's other bolt, and remove it from that bolt's vector
		BOLT* OtherBolt = NULL;
		if( bolt->AttachedGirders[i]->Bolt1 == bolt )
		{
			OtherBolt = bolt->AttachedGirders[i]->Bolt2;
		}
		else
		{
			OtherBolt = bolt->AttachedGirders[i]->Bolt1;
		}

		for( std::vector<Girder*>::iterator it = OtherBolt->AttachedGirders.begin() ; it != OtherBolt->AttachedGirders.end(); ++it )
		{
			if( (*it) == bolt->AttachedGirders[i] )
			{
				OtherBolt->AttachedGirders.erase(it);
				break;
			}
		}

		delete bolt->AttachedGirders[i];
	}
	bolt->AttachedGirders.clear();
				
	//now remove this bolt from the bolts vector
	for( std::vector<BOLT*>::iterator it = Bolts->begin() ; it != Bolts->end(); ++it )
	{
		if( (*it) == bolt )
		{
			Bolts->erase(it);
			break;
		}
	}
	return true;
}