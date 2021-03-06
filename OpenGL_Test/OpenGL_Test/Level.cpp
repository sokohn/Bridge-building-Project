//credit to fdachille who's project bridge fun helped me out a lot with the physics handling
#include <assert.h> 
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>
#include "Bolt.h"
#include "Girder.h"
#include "Land.h"
#include "Level.h"
#include "Util.h"

extern MOUSE Mouse;
extern float CameraX;
extern float CameraY;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

LEVEL::LEVEL()
{
	Girders = new std::vector<GIRDER*>();
	Bolts = new std::vector<BOLT*>();
	Land = new LAND(-2000, 2000, 480, -2000 );

	Land->AddPoint(700,240);
	Land->AddPoint(1220,240);

	AddBolt(640,480, true);
	AddBolt(1280,480, true);
	AddBolt(960, 320, true);

	RoadLevel = 0;

	MaxMoney = 20000;
	CurrentlySpentMoney = 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

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

	for (int i = 0; i < Girders->size(); i++)
	{
		(*Girders)[i]->Highlighted = false;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

float GetBoltDistance( BOLT* b1, BOLT* b2 )
{
    return sqrt( pow( b1->x - b2->x, 2 ) + pow( b1->y - b2->y, 2 ) );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void LEVEL::SimulatePhysics( float DeltaTime )
{
	// damped out the force a bit
	const float damping = (1.0 - 1.0*DeltaTime);
	for( int i = 0; i < Bolts->size(); i++ )
	{
		BOLT* Bolt = (*Bolts)[i];
		Bolt->forceX = 0;
		Bolt->forceY = 0;
		Bolt->JustAppliedForceX = 0;
		Bolt->JustAppliedForceY = 0;
		Bolt->VelocityX*= damping;
		Bolt->VelocityY*= damping;
	}
	// apply gravity to bolts
	//todo: add in truck weight, via clever algorithm
	float weight = -1.0f*GIRDER::GirderWeight;
	for( int i = 0; i < Girders->size(); i++ )
	{
		GIRDER* girder = (*Girders)[i];
		if( girder->isActive )
		{
			if( girder->Bolt1->CanMove() )
			{
				girder->Bolt1->forceY += weight / 2;
			}
			if( girder->Bolt2->CanMove() )
			{
				girder->Bolt2->forceY += weight / 2;
			}
		}
		//also update the stresses while we're at it
		if (girder->isActive)
		{
			girder->CurrentStress = girder->GetStressForce(girder->getCurrentLength());
			if (fabs(girder->CurrentStress) > fabs(girder->MaxStress))
			{
				girder->MaxStress = girder->CurrentStress;
			}
			if (fabs(girder->CurrentStress) > GIRDER::GirderStrength)
			{
				girder->isActive = false;
			}
		}
	}
	for (int i = 0; i < Bolts->size(); i++)
	{
		BOLT* Bolt = (*Bolts)[i];
		if (!Bolt->CanMove())
		{
			continue;
		}
		//now go through all of the girders and add in their contributing forces
		for (int j = 0; j < Bolt->AttachedGirders.size(); j++)
		{
			if (!Bolt->AttachedGirders[j]->isActive)
			{
				continue;
			}
			if (Bolt->AttachedGirders[j]->getCurrentLength() != 0 )
			{
				BOLT* OtherBolt = NULL;
				if (Bolt->AttachedGirders[j]->Bolt1 == Bolt)
				{
					OtherBolt = Bolt->AttachedGirders[j]->Bolt2;
				}
				else
				{
					OtherBolt = Bolt->AttachedGirders[j]->Bolt1;
				}
				float DiffY = OtherBolt->y - Bolt->y;
				float DiffX = OtherBolt->x - Bolt->x;
				float theta = atan2(DiffY, DiffX);
				float ForceY = (Bolt->AttachedGirders[j]->GetStressForce(Bolt->AttachedGirders[j]->getCurrentLength()))
				* sin(theta);
				Bolt->forceX += (Bolt->AttachedGirders[j]->GetStressForce(Bolt->AttachedGirders[j]->getCurrentLength()))
				* cos(theta);
				Bolt->forceY += ForceY;
				//fprintf(stderr, "theta: %f\n", theta);
			}
		}
	}
	//move move the bolts
	for( int i = 0; i < Bolts->size(); i++ )
	{
		BOLT* Bolt = (*Bolts)[i];
		if(Bolt->CanMove() )
		{
			float NetForce = 0;
			if(Bolt->GetNumActiveGirders() > 0 )
			{
				float StartingX = Bolt->x;
				float StartingY = Bolt->y;
				float newX = 0.0f;
				float newY = 0.0f;
				GIRDER* LastActiveGirder = NULL;
				Bolt->VelocityX += Bolt->forceX * DeltaTime;
				Bolt->VelocityY += Bolt->forceY * DeltaTime;
				Bolt->x += Bolt->VelocityX* DeltaTime;
				Bolt->y += Bolt->VelocityY* DeltaTime;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void drawGrid()
{
	int NumLines = 2*( Screen.width/ ZoomLevel )/GridSpacing;
	//vertical lines
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0.0f,0.0f,0.5f);
	for(int i = 0; i< NumLines; i++ )
	{
		glLoadIdentity();
		glPushMatrix();
		int GridLoc = (CameraX- (int)CameraX%GridSpacing )+(int) ((Screen.width / ZoomLevel )/-2.0-fmodf( (Screen.width / ZoomLevel )/-2.0, (float)GridSpacing )) + i*GridSpacing;
		GridLoc = GridLoc - GridLoc%GridSpacing;
		if( GridLoc % 160 == 0 )
		{
			//major line
			glLineWidth(0.25f);			
			glColor3f( 0.55f,0.555f, 0.35f );
		}
		else if( GridLoc % 40 == 0 )
		{
			//medium line
			glLineWidth(0.15f);			
			glColor3f( 0.35f,0.35f, 0.35f );
		}
		else
		{
			//minor lines
			glLineWidth(0.15f);			
			glColor3f( 0.0f,0.0f, 0.0f );
		}
		
		glBegin(GL_LINES);
		glVertex3f( GridLoc , CameraY+(Screen.height / ZoomLevel )/-2.0 , 0.0f);
		glVertex3f( GridLoc, CameraY+(Screen.height / ZoomLevel )/2.0, 0.0f);
		glEnd();
		
		glPopMatrix();
	}

	NumLines = 2*( Screen.height/ ZoomLevel )/GridSpacing;
	for(int i = 0; i< NumLines; i++ )
	{
		glLoadIdentity();
		glPushMatrix();
		int GridLoc = (CameraY- (int)CameraY%GridSpacing )+(int) ((Screen.height / ZoomLevel )/-2.0-fmodf( (Screen.height / ZoomLevel )/-2.0, (float)GridSpacing )) + i*GridSpacing;
		GridLoc = GridLoc - GridLoc%GridSpacing;
		if( GridLoc % 160 == 0 )
		{
			//major line
			glLineWidth(0.25f);
			glColor3f(0.55f, 0.555f, 0.35f);
		}
		else if( GridLoc % 40 == 0 )
		{
			//medium line
			glLineWidth(0.25f);			
			glColor3f(0.35f, 0.35f, 0.35f);
		}
		else
		{
			//minor lines
			glLineWidth(0.15f);			
			glColor3f( 0.0f,0.0f, 0.0f );
		}
		
		glBegin(GL_LINES);
		glVertex3f( CameraX-Screen.width/ ZoomLevel , GridLoc, 0.0f);
		glVertex3f( CameraX+Screen.width/ ZoomLevel, GridLoc, 0.0f);
		glEnd();  
		
		glPopMatrix();
	}
	glPopMatrix();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void LEVEL::drawLand()
{
	Land->DrawLand();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void LEVEL::DrawRoad()
{
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0.0f,0.0f,0.4f);
	glLineWidth(0.25f);			
	glColor3f( 0.75f,0.75f, 0.0f );
	glBegin(GL_LINES);
	glVertex3f( CameraX-Screen.width/ ZoomLevel, RoadLevel, 0.0f);
	glVertex3f( CameraX+Screen.width/ ZoomLevel, RoadLevel, 0.0f);
	glEnd();  
	glPopMatrix();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void LEVEL::Draw()
{
	drawGrid();

	drawLand();

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

		glPushMatrix();
		glLoadIdentity();
		glColor3f(1.0f, 0.0f, 0.0f);
		glRasterPos2f(Bolt->GetDrawX()+10, Bolt->GetDrawY()+10);
		char BoltNum[5];
		int len = sprintf(BoltNum,"%d",i);
		for (int i = 0; i < len; i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, BoltNum[i]);
		}
		glPopMatrix();

		//now draw the force diagrams
		if (IsSimulating() )// !IsPaused() )
		{
			glLoadIdentity();
			glPushMatrix();
			glLineWidth(0.75f);
			glColor3f( 1.0f ,1.0f ,1.0f ); 
			glBegin(GL_LINES);
			glVertex3f( Bolt->GetDrawX(), Bolt->GetDrawY(), 0.0f);
			if (Bolt->CanMove() && !IsPaused())
			{
				//fprintf(stderr, "Force X: %f \t Force Y: %f \n", Bolt->forceX, Bolt->forceY);
			}
			glVertex3f( Bolt->GetDrawX()+Bolt->forceX, Bolt->GetDrawY()+Bolt->forceY, 0.0f);
			glEnd();  
		}
	}

	for(int i =0; i<Girders->size(); i++ )
	{
		glLoadIdentity();
		glPushMatrix();
		GIRDER* girder = (*Girders)[i];

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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool LEVEL::CheckIfRoad( GIRDER* girder )
{
	if( girder->Bolt1->y == girder->Bolt2->y )
	{
		return girder->Bolt1->y == RoadLevel;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

GIRDER* LEVEL::AddGirder( BOLT* Bolt1, float x, float y )
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

	GIRDER* girder = new GIRDER();
	girder->Bolt1 = Bolt1;
	girder->Bolt2 = AddBolt(BoltLocX,BoltLocY, false);
	girder->isFinished = true;
	Girders->push_back(girder);
	girder->Index = Girders->size() - 1;
	CurrentlySpentMoney += 100;
	
	//now add it to the its bolts
	girder->Bolt1->AttachedGirders.push_back(girder);
	girder->Bolt2->AttachedGirders.push_back(girder);
	if( CheckIfRoad(girder) )
	{
		girder->isRoad = true;
	}
	return girder;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

GIRDER* LEVEL::AddGirder( BOLT* Bolt1, BOLT* Bolt2 )
{
	if(IsSimulating())
	{
		return NULL;
	}
	//add in code to check for is these two bolts are already linked
	for( int i = 0; i < Girders->size(); i++ )
	{
		GIRDER* girder = (*Girders)[i];
		if( ( girder->Bolt1 == Bolt1 && girder->Bolt2 == Bolt2 ) || ( girder->Bolt1 == Bolt2 && girder->Bolt2 == Bolt1 ) )
		{
			fprintf(stderr, "This girder already exists\n");
			return NULL;
		}
	}

	//ok, we're good
	GIRDER* girder = new GIRDER();
	girder->Bolt1 = Bolt1;
	girder->Bolt2 = Bolt2;
	girder->isFinished = true;
	Girders->push_back(girder);
	girder->Index = Girders->size() - 1;
	CurrentlySpentMoney += 100;
	
	//now add it to the its bolts
	girder->Bolt1->AttachedGirders.push_back(girder);
	girder->Bolt2->AttachedGirders.push_back(girder);
	if( CheckIfRoad(girder) )
	{
		girder->isRoad = true;
	}
	return girder;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BOLT* LEVEL::AddBolt(float x, float y, bool IsAnchor)
{
	float BoltLocX = RoundToNearestGridMarker(x); 
	float BoltLocY = RoundToNearestGridMarker(y);
	BOLT* Bolt = NULL;

	for(int i =0; i < Bolts->size(); i++ )
	{
		if( (*Bolts)[i]->x == BoltLocX && (*Bolts)[i]->y == BoltLocY )
		{
			fprintf(stderr, "Bolts merged\n");
			return (*Bolts)[i];
		}
	}
	if( IsAnchor )
	{
		Bolt = new Anchor( BoltLocX , BoltLocY );
		//also add land point at the anchor
		Land->AddPoint(BoltLocX,BoltLocY);
	}
	else
	{
		Bolt = new BOLT( BoltLocX , BoltLocY );
	}
	Bolts->push_back(Bolt);
	Bolt->Index = Bolts->size() -1;
	return Bolt;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool LEVEL::RemoveGirder(GIRDER* Girder)
{
	for (std::vector<GIRDER*>::iterator it = Girder->Bolt1->AttachedGirders.begin(); it != Girder->Bolt1->AttachedGirders.end(); ++it)
	{
		if ((*it) == Girder)
		{
			Girder->Bolt1->AttachedGirders.erase(it);
			break;
		}
	}

	for (std::vector<GIRDER*>::iterator it = Girder->Bolt2->AttachedGirders.begin(); it != Girder->Bolt2->AttachedGirders.end(); ++it)
	{
		if ((*it) == Girder)
		{
			Girder->Bolt2->AttachedGirders.erase(it);
			break;
		}
	}

	for (std::vector<GIRDER*>::iterator it = Girders->begin(); it != Girders->end(); ++it)
	{
		if ((*it) == Girder)
		{
			Girders->erase(it);
			CurrentlySpentMoney -= 100;
			break;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

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
		for( std::vector<GIRDER*>::iterator it = Girders->begin() ; it != Girders->end(); ++it )
		{
			if( (*it) == bolt->AttachedGirders[i] )
			{
				Girders->erase(it);
				CurrentlySpentMoney -= 100;
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

		for( std::vector<GIRDER*>::iterator it = OtherBolt->AttachedGirders.begin() ; it != OtherBolt->AttachedGirders.end(); ++it )
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
	for( std::vector<BOLT*>::iterator it = Bolts->begin() ; it != Bolts->end(); it++ )
	{
		if( (*it) == bolt )
		{
			Bolts->erase(it);
			break;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BOLT* LEVEL::FindBolt(float x, float y)
{
	for (int i = 0; i < Bolts->size(); i++)
	{
		if ((*Bolts)[i]->x == x && (*Bolts)[i]->y == y)
		{
			return (*Bolts)[i];
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

GIRDER* LEVEL::FindGirder(float x1, float y1, float x2, float y2)
{
	for (int i = 0; i < Bolts->size(); i++)
	{
		BOLT* Bolt = (*Bolts)[i];
		if (Bolt->x == x1 && Bolt->y == y1)
		{
			for (int j = 0; j < Bolt->AttachedGirders.size(); j++)
			{
				if (Bolt->AttachedGirders[j]->getCurrentLength() != 0)
				{
					BOLT* OtherBolt = NULL;
					if (Bolt->AttachedGirders[j]->Bolt1 == Bolt)
					{
						OtherBolt = Bolt->AttachedGirders[j]->Bolt2;
					}
					else
					{
						OtherBolt = Bolt->AttachedGirders[j]->Bolt1;
					}

					if (OtherBolt->x == x2 && OtherBolt->y == y2)
					{
						return Bolt->AttachedGirders[j];
					}
				}
			}
			return NULL;
		}
	}
	return NULL;
}

GIRDER* LEVEL::FindGirder(float x, float y)
{
	//find the nearest girder to specified location, assuming any are within range

	Vector2D MouseLoc(x, y);
	GIRDER* ClosestGirder = NULL;
	float BestDistance = 10000000;
	for (int i = 0; i < Girders->size(); i++)
	{
		float CurrDistance = 0;
		GIRDER* Temp = (*Girders)[i];
		Vector2D End1(Temp->Bolt1->x, Temp->Bolt1->y);
		Vector2D End2(Temp->Bolt2->x, Temp->Bolt2->y);

		Vector2D ab = End2 - End1;
		Vector2D ac = MouseLoc - End1;
		Vector2D bc = MouseLoc - End2;
		float e = ac.Dot(ab);
		if (e <= 0)
		{
			CurrDistance = ac.Dot(ac);
		}
		else
		{
			float f = ab.Dot(ab);
			if (e >= f)
			{
				CurrDistance = bc.Dot(bc);
			}
			else
			{
				CurrDistance = ac.Dot(ac) - (e *e / f);
			}
		}

		if (CurrDistance < BestDistance)
		{
			ClosestGirder = Temp;
			BestDistance = CurrDistance;
		}
	}

	if (BestDistance <= 100)
	{
		return ClosestGirder;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void LEVEL::StartSimulation()
{
	for(int i =0; i< Bolts->size(); i++ )
	{
		(*Bolts)[i]->startX = (*Bolts)[i]->x;
		(*Bolts)[i]->startY = (*Bolts)[i]->y;
		(*Bolts)[i]->forceX = 0.0;
		(*Bolts)[i]->forceY = 0.0;
		(*Bolts)[i]->VelocityX =0.0f;
		(*Bolts)[i]->VelocityY =0.0f;
		(*Bolts)[i]->JustAppliedForceX =0.0f;
		(*Bolts)[i]->JustAppliedForceY =0.0f;
	}

	for(int i =0; i < Girders->size(); i++ )
	{
		(*Girders)[i]->StartingLength = GetBoltDistance((*Girders)[i]->Bolt1, (*Girders)[i]->Bolt2 );
		(*Girders)[i]->isActive = true;
		(*Girders)[i]->CurrentStress = 0;
		(*Girders)[i]->MaxStress = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void LEVEL::DeserializeLevel(char* FileName)
{
	if(IsSimulating() )
	{
		SetSimulating(false);
	}

	//now clear out the existing level
	while(!Bolts->empty())
	{
		BOLT* Bolt = Bolts->back();
		RemoveBolt(Bolt);
	}

	if(!Bolts->empty())
	{
		fprintf(stderr, "Didn't delete all of the bolts\n");
		Bolts->clear();
	}
	if(!Girders->empty())
	{
		fprintf(stderr, "Didn't delete all of the Girders\n");
		Girders->clear();
	}

	std::ifstream Deserializer(FileName);
	if( !Deserializer.good() )
	{
		fprintf(stderr, "Couldn't open the file\n");
		return;
	}

	char Text[50];
	//Todo: Convert to enum?
	bool IsBolt = false;
	bool IsAnchor = false;
	bool IsGirder = false;
	while( !Deserializer.eof() )
	{
		IsBolt = false;
		IsAnchor = false;
		IsGirder = false;
		Deserializer.getline(Text,50,':');
		if(strcmp(Text,"ANCHOR")==0)
		{
			IsAnchor = true;
		}
		else if( strcmp(Text,"BOLT")==0 )
		{
			IsBolt = true;
		}
		else if( strcmp(Text,"GIRDER")==0 )
		{
			IsGirder = true;
		}
		else
		{
			//something is weird, skip the line
			if( Deserializer.fail() )
			{
				//we're on the next line, I think
			}
			else
			{
				//we found something unknown
				//just go to the next line
				Deserializer.getline(Text,50);
			}
		}

		if( IsBolt || IsAnchor )
		{
			float x = 0;
			float y = 0;
			//this seems kind of sketchy
			Deserializer.getline(Text,50,',');
			x = atof(Text);
			Deserializer.getline(Text,50);
			y = atof(Text);
			AddBolt(x,y,IsAnchor);
		}
		else if( IsGirder )
		{
			float x = 0;
			float y = 0;
			//this seems kind of sketchy
			Deserializer.getline(Text,50,',');
			x = atof(Text);
			Deserializer.getline(Text,50,':');
			y = atof(Text);
			BOLT* Bolt = AddBolt(x,y,IsAnchor);
			Deserializer.getline(Text,50,',');
			x = atof(Text);
			Deserializer.getline(Text,50);
			y = atof(Text);
			AddGirder(Bolt, x, y );
		}
	}
	Deserializer.close();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void LEVEL::SerializeLevel(char* FileName)
{
	if(IsSimulating() )
	{
		SetSimulating(false);
	}
	//serializes the currently loaded level out to a file
	FILE* File = std::fopen(FileName, "w");
	std::fclose(File);
	std::ofstream Serializer(FileName);

	if(!Serializer.good())
	{
		fprintf(stderr, "Couldn't open the file\n");
		return;
	}

	//go through the bolts and output their locations
	for(int i =0; i < Bolts->size(); i++ )
	{
		BOLT* Bolt = (*Bolts)[i];
		if(Bolt->CanMove() )
		{
			Serializer<<"BOLT:";
		}
		else
		{
			Serializer<<"ANCHOR:";
		}
		Serializer<<std::setprecision(0);
		
		Serializer<<Bolt->x<<","<<Bolt->y<<std::endl;
	}

	//and now the girders
	for(int i =0; i < Girders->size(); i++ )
	{
		GIRDER* girder = (*Girders)[i];
		
		Serializer<<"GIRDER:";
		Serializer<<std::setprecision(0);
		Serializer<<girder->Bolt1->x<<","<<girder->Bolt1->y<<":"<<girder->Bolt2->x<<","<<girder->Bolt2->y<<std::endl;
	}
	Serializer.flush();
	Serializer.close();
}