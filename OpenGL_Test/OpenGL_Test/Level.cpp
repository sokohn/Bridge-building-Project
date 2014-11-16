//credit to fdachille who's project bridge fun helped me out a lot with the physics handling

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
	bolt = new Anchor(960,480);	
	Bolts->push_back(bolt);
	RoadLevel = 480;

	MaxMoney = 20000;
	CurrentlySpentMoney = 0;
}

void LEVEL::Update(float DeltaTime)
{
	//deal with the delta time later when we get to actually simulating physics
	if( IsSimulating() && !IsPaused() )
	{
		SimulatePhysics(DeltaTime);
	}

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

float GetBoltDistance( BOLT* b1, BOLT* b2 )
{
    return sqrt( pow( b1->x - b2->x, 2 ) + pow( b1->y - b2->y, 2 ) );
}

void LEVEL::SimulatePhysics( float DeltaTime )
{
	 // damped out the force a bit
	const float damping = 0.9f;
	for( int i = 0; i < Bolts->size(); i++ )
	{
		BOLT* Bolt = (*Bolts)[i];
		Bolt->forceX =0;//*= damping;
		Bolt->forceY =0;//*= damping;
	}

	// apply gravity to bolts
	//todo: add in truck weight, via clever algorithm
	  
	float weight = Girder::GirderWeight;
	for( int i = 0; i < Girders->size(); i++ )
	{
		Girder* girder = (*Girders)[i];
		if( girder->isActive )
		{
			girder->Bolt1->forceY += weight / 2;
			girder->Bolt2->forceY += weight / 2;
		}
	}

	// stretch/shrink girders and accumulate forces at bolts
	for( int i = 0; i < Girders->size(); i++ )
	{
		Girder* girder = (*Girders)[i];
		if( !girder->isActive )
		{
			continue;
		}

		// calculate force and direction
		float currentLength = GetBoltDistance( girder->Bolt1, girder->Bolt2 );
		float stress = -1.0f* girder->GetStressForce( currentLength );
		girder->CurrentStress = stress;
		if( fabs(girder->CurrentStress)>fabs(girder->MaxStress) )
		{
			girder->MaxStress = girder->CurrentStress;
		}
		float NormalizedX = ( girder->Bolt1->x - girder->Bolt2->x )/ currentLength;
		float NormalizedY = ( girder->Bolt1->y - girder->Bolt2->y )/ currentLength;
		float forceX = stress * NormalizedX;
		float forceY = stress * NormalizedY;

		// potentially break spring
		if( fabs( stress ) > Girder::GirderStrength )
		{
			girder->isActive = false;
			forceX = forceY = 0;
			continue;
		}

		// accumulate first end
		girder->Bolt1->forceX -= forceX;
		girder->Bolt1->forceX -= forceY;

		// accumulate second end
		girder->Bolt2->forceX -= forceX;
		girder->Bolt2->forceX -= forceY;
	}

	//move move the bolts
	for( int i = 0; i < Bolts->size(); i++ )
	{
		BOLT* Bolt = (*Bolts)[i];
		if(Bolt->CanMove() )
		{
			Bolt->x += Bolt->forceX*DeltaTime;
			Bolt->y -= Bolt->forceY*DeltaTime;//becuase our co-ordinates have lower numbers on the bottom
		}
	}

}

void drawGrid()
{
	int NumLines = ( Screen.width/ ZoomLevel )/GridSpacing;
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
		glVertex3f( i*GridSpacing, Screen.height / ZoomLevel, 0.0f);
		glEnd();  
		
		glPopMatrix();
	}

	NumLines = ( Screen.height/ ZoomLevel )/GridSpacing;
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
		glVertex3f( Screen.width/ ZoomLevel, i*GridSpacing, 0.0f);
		glEnd();  
		
		glPopMatrix();
	}
	glPopMatrix();
}

void LEVEL::DrawRoad()
{
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0.0f,0.0f,0.4f);
	glLineWidth(0.25f);			
	glColor3f( 0.75f,0.75f, 0.0f );
	glBegin(GL_LINES);
	glVertex3f( 0.0f , RoadLevel, 0.0f);
	glVertex3f( Screen.width/ ZoomLevel, RoadLevel, 0.0f);
	glEnd();  
	glPopMatrix();
}

void LEVEL::Draw()
{
	drawGrid();

	DrawRoad();

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
		if( girder->isActive )
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

bool LEVEL::CheckIfRoad( Girder* girder )
{
	if( girder->Bolt1->y == girder->Bolt2->y )
	{
		return girder->Bolt1->y == RoadLevel;
	}
	return false;
}

Girder* LEVEL::AddGirder( BOLT* Bolt1, float x, float y )
{
	if(IsSimulating())
	{
		return NULL;
	}
	//add check for land collision once I add in land

	//check to see if there if a bolt already exists where we want to put this bolt
	float BoltLocX = RoundToNearestGridMarker(x); 
	float BoltLocY = RoundToNearestGridMarker(y); 

	for(int i =0; i < Bolts->size(); i++ )
	{
		if( (*Bolts)[i]->x == BoltLocX && (*Bolts)[i]->y == BoltLocY )
		{
			fprintf(stderr, "Bolts merged\n");
			return AddGirder( Bolt1, (*Bolts)[i] );
		}
	}

	Girder* girder = new Girder();
	girder->Bolt1 = Bolt1;
	girder->Bolt2 = new BOLT( BoltLocX , BoltLocY );
	girder->isFinished = true;
	Girders->push_back(girder);
	Bolts->push_back(girder->Bolt2);
	
	//now add it to the its bolts
	girder->Bolt1->AttachedGirders.push_back(girder);
	girder->Bolt2->AttachedGirders.push_back(girder);
	if( CheckIfRoad(girder) )
	{
		girder->isRoad = true;
	}
	return girder;
}

Girder* LEVEL::AddGirder( BOLT* Bolt1, BOLT* Bolt2 )
{
	if(IsSimulating())
	{
		return NULL;
	}
	//add in code to check for is these two bolts are already linked
	for( int i = 0; i < Girders->size(); i++ )
	{
		Girder* girder = (*Girders)[i];
		if( ( girder->Bolt1 == Bolt1 && girder->Bolt2 == Bolt2 ) || ( girder->Bolt1 == Bolt2 && girder->Bolt2 == Bolt1 ) )
		{
			fprintf(stderr, "This girder already exists\n");
			return NULL;
		}
	}

	//ok, we're good
	Girder* girder = new Girder();
	girder->Bolt1 = Bolt1;
	girder->Bolt2 = Bolt2;
	girder->isFinished = true;
	Girders->push_back(girder);

	
	//now add it to the its bolts
	girder->Bolt1->AttachedGirders.push_back(girder);
	girder->Bolt2->AttachedGirders.push_back(girder);
	if( CheckIfRoad(girder) )
	{
		girder->isRoad = true;
	}
	return girder;
}

bool LEVEL::RemoveGirder()
{
	return true;
}

bool LEVEL::RemoveBolt( BOLT* bolt )
{
	if(IsSimulating())
	{
		return false;
	}
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

void LEVEL::StartSimulation()
{
	for(int i =0; i< Bolts->size(); i++ )
	{
		(*Bolts)[i]->startX = (*Bolts)[i]->x;
		(*Bolts)[i]->startY = (*Bolts)[i]->y;
		(*Bolts)[i]->forceX = 0.0;
		(*Bolts)[i]->forceY = 0.0;
	}

	for(int i =0; i < Girders->size(); i++ )
	{
		(*Girders)[i]->StartingLength = GetBoltDistance((*Girders)[i]->Bolt1, (*Girders)[i]->Bolt2 );
		(*Girders)[i]->isActive = true;
		(*Girders)[i]->CurrentStress = 0;
		(*Girders)[i]->MaxStress = 0;
	}
}

void LEVEL::EndSimulation()
{
	for( int i = 0; i < Girders->size(); i++ )
	{
		(*Girders)[i]->isActive = true;
	}

	for(int i =0; i< Bolts->size(); i++ )
	{
		(*Bolts)[i]->x = (*Bolts)[i]->startX;
		(*Bolts)[i]->y = (*Bolts)[i]->startY;
		(*Bolts)[i]->forceX = 0.0;
		(*Bolts)[i]->forceY = 0.0;

	}
}