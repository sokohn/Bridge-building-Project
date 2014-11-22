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
#include "Level.h"
#include "Util.h"

extern MOUSE Mouse;

LEVEL::LEVEL()
{
	Girders = new std::vector<Girder*>();
	Bolts = new std::vector<BOLT*>();

	AddBolt(480,480, true);
	AddBolt(640,480, true);

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
		Bolt->JustAppliedForceX = 0;
		Bolt->JustAppliedForceY = 0;
		Bolt->VelocityX*= damping;
		Bolt->VelocityY*= damping;
	}

	// apply gravity to bolts
	//todo: add in truck weight, via clever algorithm
	  
	float weight = -1.0f*Girder::GirderWeight;
	for( int i = 0; i < Girders->size(); i++ )
	{
		Girder* girder = (*Girders)[i];
		if( girder->isActive )
		{
			if( girder->Bolt1->CanMove() )
			{
				girder->Bolt1->forceY +=DeltaTime* weight / 2;
			}
			if( girder->Bolt2->CanMove() )
			{
				girder->Bolt2->forceY += DeltaTime*weight / 2; 
			}
		}
	}

	//go through the bolts and apply forces to their neighbors
	for( int z =0; z < 1; z++ )
	{
		for( int i = 0; i < Bolts->size(); i++ )
		{
			BOLT* Bolt = (*Bolts)[i];
			if( !Bolt->CanMove() )
			{
				//they don't need to transfer force to others
				Bolt->JustAppliedForceX = 0;
				Bolt->JustAppliedForceY = 0;
				Bolt->forceX =0;
				Bolt->forceY = 0;
				continue;
			}
			if( Bolt->AttachedGirders.size() == 0 )
			{
				continue;
			}
			float TotalContributionX= 0;
			float TotalContributionY= 0;
			for(int j = 0; j < Bolt->AttachedGirders.size(); j++ )
			{
				if( !Bolt->AttachedGirders[j]->isActive )
				{
					continue;
				}
				//find the other bolt
				BOLT* OtherBolt = NULL;
				if( Bolt->AttachedGirders[j]->Bolt1 == Bolt )
				{
					OtherBolt = Bolt->AttachedGirders[j]->Bolt2;
				}
				else
				{
					OtherBolt = Bolt->AttachedGirders[j]->Bolt1;
				}
			
				//now vector project my force onto the neighbor
				float currentLength = GetBoltDistance( Bolt, OtherBolt );
				float NormalizedX = (  OtherBolt->x - Bolt->x )/ currentLength;
				float NormalizedY = (  OtherBolt->y - Bolt->y )/ currentLength;
				if( NormalizedY != 0 || currentLength == 0 )
				{
					TotalContributionY+=NormalizedY;
				}
				if( NormalizedX != 0 || currentLength == 0 )
				{
					TotalContributionX+=NormalizedX;
				}
			}
			//fprintf(stderr, "Bolt %d\tContribution X: %f Contribution X: %f\n",i,TotalContributionX, TotalContributionY );
			//TotalContribution = sqrt(TotalContribution);

			for(int j = 0; j < Bolt->AttachedGirders.size(); j++ )
			{
				if( !Bolt->AttachedGirders[j]->isActive )
				{
					continue;
				}
				//find the other bolt
				BOLT* OtherBolt = NULL;
				if( Bolt->AttachedGirders[j]->Bolt1 == Bolt )
				{
					OtherBolt = Bolt->AttachedGirders[j]->Bolt2;
				}
				else
				{
					OtherBolt = Bolt->AttachedGirders[j]->Bolt1;
				}

				float TotalForce = sqrt(Bolt->forceX*Bolt->forceX+Bolt->forceY*Bolt->forceY);
			
				//now vector project my force onto the neighbor
				float currentLength = GetBoltDistance( Bolt, OtherBolt );
				float NormalizedX = (  OtherBolt->x - Bolt->x )/ currentLength;
				float NormalizedY = (  OtherBolt->y - Bolt->y )/ currentLength;
				float ForceX = TotalForce*NormalizedX;
				float ForceY = TotalForce*NormalizedY;


				//figure which direction has can handle less 


				OtherBolt->JustAppliedForceX+= ForceX;//ForceMagnitude*NormalizedX;
				OtherBolt->JustAppliedForceY+= ForceY;//ForceMagnitude*NormalizedY;
				Bolt->JustAppliedForceX -= ForceX;//ForceMagnitude*NormalizedX;
				Bolt->JustAppliedForceY -= ForceY;//ForceMagnitude*NormalizedY;
				if( Bolt->CanMove() )
				{
					//fprintf(stderr, "Bolt %d\nNormal X: %f Force X: %f \tNormal Y: %f Force Y: %f \nForce Magnitude: %f\n",j,NormalizedX, Bolt->forceX,NormalizedY, Bolt->forceY, ForceMagnitude );
					//fprintf(stderr, "AppliedForce X: %f \tAppliedForce Y: %f\n",ForceMagnitude*NormalizedX, ForceMagnitude*NormalizedY );
				}
				if( !OtherBolt->CanMove() )
				{
					//anchors eat all force from their neighbors
					//Bolt->JustAppliedForceX = 0;
					//Bolt->JustAppliedForceY = 0;
				}

			}
		}
	}
	//run through all of the bolts and print out the forces acting on them
	for( int i = 0; i < Bolts->size(); i++ )
	{
		BOLT* Bolt = (*Bolts)[i];
		Bolt->forceX+=Bolt->JustAppliedForceX;
		Bolt->forceY+=Bolt->JustAppliedForceY;
		Bolt->JustAppliedForceX = 0;
		Bolt->JustAppliedForceY = 0;
		//fprintf(stderr, "Bolt %d\nForce X: %f \t Force Y: %f \n",i, Bolt->forceX, Bolt->forceY );
	}
	//fprintf(stderr, "\n");

	//move move the bolts
	for( int i = 0; i < Bolts->size(); i++ )
	{
		BOLT* Bolt = (*Bolts)[i];
		if(Bolt->CanMove() )
		{
			//Bolt->VelocityX+= Bolt->forceX*DeltaTime;
			//Bolt->VelocityY+= Bolt->forceY*DeltaTime;
			//Bolt->x += Bolt->forceX;//*DeltaTime;
			//Bolt->y += Bolt->forceY;//*DeltaTime;
			//fprintf(stderr, "Loc X: %f\tLoc Y: %f\n",Bolt->x, Bolt->y );

			//instead, lets try to treat this movement as springs
			//*
			float NetForce = 0;
			if(Bolt->GetNumActiveGirders() > 0 )
			{
				float StartingX = Bolt->x;
				float StartingY = Bolt->y;
				float TotalDisplacementX = 0;
				float TotalDisplacementY = 0;
				int Iterations = 0;
				do
				{
					int BestGirderIndex = 0;
					float BestDotProduct = 0;
					float ForceDispersed = 0;
					float BestNormalizedX = 0;
					float BestNormalizedY = 0;
					for(int j = 0; j < Bolt->AttachedGirders.size(); j++ )
					{
						if( !Bolt->AttachedGirders[j]->isActive )
						{
							continue;
						}
						//find the other bolt
						BOLT* OtherBolt = NULL;
						if( Bolt->AttachedGirders[j]->Bolt1 == Bolt )
						{
							OtherBolt = Bolt->AttachedGirders[j]->Bolt2;
						}
						else
						{
							OtherBolt = Bolt->AttachedGirders[j]->Bolt1;
						}
						float currentLength = GetBoltDistance( Bolt, OtherBolt );
						float NormalizedX = (  OtherBolt->x - Bolt->x )/ currentLength;
						float NormalizedY = (  OtherBolt->y - Bolt->y )/ currentLength;
						float ForceMagnitude = Bolt->forceX*NormalizedX + Bolt->forceY*NormalizedY;
						if( fabs(ForceMagnitude) > fabs(BestDotProduct) )
						{
							BestGirderIndex = j;
							BestNormalizedX = NormalizedX;
							BestNormalizedY = NormalizedY;
						}
					}
					//now lets offload as much off of force onto this girder as we can	
					if( abs(Bolt->forceX*BestNormalizedX) > abs(Bolt->forceY*BestNormalizedY) )
					{
						ForceDispersed = Bolt->forceX;
					}
					else
					{
						ForceDispersed = Bolt->forceY;
					}
					float DisplacementX = BestNormalizedX * ForceDispersed / 11;
					float DisplacementY = BestNormalizedY * ForceDispersed / 11;
					if( DisplacementX != DisplacementX )
					{
						DisplacementX = 0;
					}
					if( DisplacementY != DisplacementY )
					{
						DisplacementY = 0;
					}
					Bolt->forceX -= BestNormalizedX * ForceDispersed;
					Bolt->forceY -= BestNormalizedY * ForceDispersed;
					Bolt->x +=DisplacementX;
					Bolt->y +=DisplacementY;
					TotalDisplacementX +=DisplacementX;
					TotalDisplacementY +=DisplacementY;
					//now see how much force is now in the system with this new displacement
					for(int j = 0; j < Bolt->AttachedGirders.size(); j++ )
					{
						if( !Bolt->AttachedGirders[j]->isActive )
						{
							continue;
						}
						//find the other bolt
						BOLT* OtherBolt = NULL;
						if( Bolt->AttachedGirders[j]->Bolt1 == Bolt )
						{
							OtherBolt = Bolt->AttachedGirders[j]->Bolt2;
						}
						else
						{
							OtherBolt = Bolt->AttachedGirders[j]->Bolt1;
						}

						float currentLength = GetBoltDistance( OtherBolt, Bolt );
						float stress = Bolt->AttachedGirders[j]->GetStressForce( currentLength );
						Bolt->AttachedGirders[j]->CurrentStress = stress;

						float NormalizedX = ( OtherBolt->x - Bolt->x )/ currentLength;
						float NormalizedY = ( OtherBolt->y - Bolt->y )/ currentLength;
						float forceX = stress * NormalizedX;
						float forceY = stress * NormalizedY;
						Bolt->forceX+=forceX;
						Bolt->forceY+=forceY;
					}
					Iterations++;
					NetForce = Bolt->forceX*Bolt->forceX + Bolt->forceY*Bolt->forceY;
				}while(Iterations < 20 && fabs(NetForce) > 9 );
				if( NetForce > 9)
				{
					//things went to shit, so lets try to average it out
					Bolt->x = StartingX + TotalDisplacementX/Iterations;
					Bolt->y = StartingY + TotalDisplacementY/Iterations;
				}
				
				fprintf(stderr, "Bolt %d\tIterations: %d\tNet Force: %f\n",i, Iterations, NetForce);
				//now that we're final, set it on the girders
				for(int j = 0; j < Bolt->AttachedGirders.size(); j++ )
				{
					if( !Bolt->AttachedGirders[j]->isActive )
					{
						continue;
					}
					//find the other bolt
					BOLT* OtherBolt = NULL;
					if( Bolt->AttachedGirders[j]->Bolt1 == Bolt )
					{
						OtherBolt = Bolt->AttachedGirders[j]->Bolt2;
					}
					else
					{
						OtherBolt = Bolt->AttachedGirders[j]->Bolt1;
					}

					float currentLength = GetBoltDistance( OtherBolt, Bolt );
					float stress = Bolt->AttachedGirders[j]->GetStressForce( currentLength );
					Bolt->AttachedGirders[j]->CurrentStress = stress;
					//fprintf(stderr, "Girder %d: Stress: %f\n",Bolt->AttachedGirders[j]->Index, stress);
					if( fabs( Bolt->AttachedGirders[j]->CurrentStress ) > fabs( Bolt->AttachedGirders[j]->MaxStress ) )
					{
						Bolt->AttachedGirders[j]->MaxStress = Bolt->AttachedGirders[j]->CurrentStress;
					}

					float NormalizedX = ( OtherBolt->x - Bolt->x )/ currentLength;
					float NormalizedY = ( OtherBolt->y - Bolt->y )/ currentLength;
					float forceX = stress * NormalizedX;
					float forceY = stress * NormalizedY;
					Bolt->forceX+=forceX;
					Bolt->forceY+=forceY;
				}
			}
		//*/	
		}
	}

}

static bool Started =false;
//this physics system is based off of the one from the west point bridge design and contest
void LEVEL::SimulatePhyscis2( float DeltaTime )
{
	if( !Started )
	{
		Started = true;
	}
	else
	{
		//return;
	}
    int nEquations = 2 * Bolts->size();
		
   float* length = (float*) calloc(Girders->size() , sizeof(float) );
   float* cosX = (float*) calloc(Girders->size() , sizeof(float) );
   float* cosY = (float*) calloc(Girders->size() , sizeof(float) );
	

   for( int i = 0; i < Bolts->size(); i++ )
	{
		BOLT* Bolt = (*Bolts)[i];
		Bolt->forceX =0;
		Bolt->forceY =0;
	}

    for (int i = 0; i < Girders->size(); i++) 
	{
		float dx = (*Girders)[i]->Bolt2->x - (*Girders)[i]->Bolt1->x;
        float dy = (*Girders)[i]->Bolt2->y - (*Girders)[i]->Bolt1->y;
        length[i] = sqrt( dx*dx + dy*dy );
        cosX[i] = dx / length[i];
        cosY[i] = dy / length[i];

		if ( !(*Girders)[i]->isActive ) 
		{
            continue;
        }
        float deadLoad = DeltaTime;
		(*Girders)[i]->Bolt1->forceY -= deadLoad;
		(*Girders)[i]->Bolt2->forceY -= deadLoad;
    }
		
	//add train load here to the joints of the girders it is touching
		
	//determine the strength of each connection
	//it goes through each girder 
   // double stiffness[][] = new double[nEquations][nEquations];
	float* stiffness = (float*) calloc( nEquations*nEquations, sizeof(float) );
    for (int i = 0; i < Girders->size(); i++) 
	{
		Girder* girder = (*Girders)[i];
        float e = 1;
		if ( !girder->isActive ) 
		{
            e = 0;
        }
        double aEOverL = 100.0f;
        double xx = aEOverL * pow(cosX[i],2);
        double yy = aEOverL * pow(cosY[i],2);
        double xy = aEOverL * cosX[i] * cosY[i];
		int j1 = girder->Bolt1->Index;
        int j2 = girder->Bolt2->Index;
        int j1x = 2 * j1;
        int j1y = 2 * j1 + 1;
        int j2x = 2 * j2;
        int j2y = 2 * j2 + 1;
        stiffness[j1x * nEquations + j1x] += xx;
        stiffness[j1x * nEquations + j1y] += xy;
        stiffness[j1x * nEquations + j2x] -= xx;
        stiffness[j1x * nEquations + j2y] -= xy;
        stiffness[j1y * nEquations + j1x] += xy;
        stiffness[j1y * nEquations + j1y] += yy;
        stiffness[j1y * nEquations + j2x] -= xy;
        stiffness[j1y * nEquations + j2y] -= yy;
        stiffness[j2x * nEquations + j1x] -= xx;
        stiffness[j2x * nEquations + j1y] -= xy;
        stiffness[j2x * nEquations + j2x] += xx;
        stiffness[j2x * nEquations + j2y] += xy;
        stiffness[j2y * nEquations + j1x] -= xy;
        stiffness[j2y * nEquations + j1y] -= yy;
        stiffness[j2y * nEquations + j2x] += xy;
        stiffness[j2y * nEquations + j2y] += yy;
    }
	
	//goes through the anchors, sets the stiffness to 0 (except for maximizing the self connection) and sets the load to 0 in the fixed direction
	for (int i = 0; i < Bolts->size(); i++ ) 
	{
		if ( !(*Bolts)[i]->CanMove() )
		{
			int ix = 2 * i;
			int iy = 2 * i +1;
			for (int ie = 0; ie < nEquations; ie++) 
			{
				stiffness[ix * nEquations + ie] =0;
				stiffness[ie * nEquations + ix] = 0;
				stiffness[iy * nEquations + ie] = 0;
				stiffness[ie * nEquations + iy] = 0;
			}
			stiffness[ix * nEquations + ix] = 1;
			stiffness[iy * nEquations + iy] = 1;
			(*Bolts)[i]->forceX = 0.0f;
			(*Bolts)[i]->forceY = 0.0f;
		}
	}
		
	//we're doing something here, not quite sure what
    for (int ie = 0; ie < nEquations; ie++) 
	{
        double pivot = stiffness[ie * nEquations + ie];
		if(pivot == 0 )
		{
			continue;
		}
        double pivr = 1.0 / pivot;
        for (int k = 0; k < nEquations; k++)
		{
            stiffness[ie * nEquations + k] /= pivot;
        }
        for (int k = 0; k < nEquations; k++)
		{
            if (k != ie) 
			{
                pivot = stiffness[k * nEquations + ie];
                for (int j = 0; j < nEquations; j++) 
				{
                    stiffness[k * nEquations + j] -= stiffness[ie * nEquations + j] * pivot;
                }
                stiffness[k * nEquations + ie] = -pivot * pivr;
            }
        }
        stiffness[ie * nEquations + ie] = pivr;

		if( ie%2 == 0 )
		{
			//fprintf(stderr, "Bolt %d:  \tstiffness X: %f ", ie/2, pivr);
		}
		else
		{
			//fprintf(stderr, "stiffness Y: %f\n", pivr);
		}
    }

	float* jointDisplacementX = (float*) calloc(Bolts->size(), sizeof(float));
	float* jointDisplacementY = (float*) calloc(Bolts->size(), sizeof(float));

	for(int i = 0; i < Bolts->size(); i++ )
	{
		float tempX = 0;
		float tempY = 0;
		for(int j = 0; j < Bolts->size(); j++ )
		{
			tempX += stiffness[2*i* nEquations + j*2] * (*Bolts)[j]->forceX;
			tempY += stiffness[( ( 2*i ) + 1 ) * nEquations + ( j*2 ) +1 ] * (*Bolts)[j]->forceY;
		}
		jointDisplacementX[i] = tempX;
		jointDisplacementY[i] = tempY;
		
		fprintf(stderr, "Bolts %d:\tMovement X: %f\tMovement Y: %f\n",i,tempX,tempY);
	}
	for(int i =0; i <  Bolts->size() -2; i++ )
	{
		float Diff1x = jointDisplacementX[i + 1] - jointDisplacementX[i];
		float Diff2x = jointDisplacementX[i + 2] - jointDisplacementX[i +1];
		float Diff1y = jointDisplacementY[i + 1] - jointDisplacementY[i];
		float Diff2y = jointDisplacementY[i + 2] - jointDisplacementY[i +1];

		fprintf(stderr, "Diff %d:\tMovement X: %f\tMovement Y: %f\n", i, Diff2x - Diff1x,  Diff2y - Diff1y );
	}

    // Compute member forces.
    /*for (int i = 0; i < Girders->size(); i++) 
	{
        Girder* girder = (*Girders)[i];
        float e = 1;
		if ( !girder->isActive ) 
		{
            e = 0;
        }
        float aEOverL = 100.0f;
		int ija = girder->Bolt1->Index;
        int ijb = girder->Bolt2->Index;
		girder->CurrentStress = aEOverL* ( ( cosX[i] * ( jointDisplacementX[ijb] - jointDisplacementX[ija] ) ) + (cosY[i] * (jointDisplacementY[ijb] - jointDisplacementY[ija] ) ) );
		if( fabs(girder->CurrentStress) > fabs(girder->MaxStress) )
		{
			girder->MaxStress = girder->CurrentStress;
		}
    }*/

	for(int i = 0; i < Bolts->size(); i++ )
	{
		(*Bolts)[i]->x = (*Bolts)[i]->x + jointDisplacementX[i];
		(*Bolts)[i]->y = (*Bolts)[i]->y + jointDisplacementY[i];
	}

	for(int j = 0; j < Girders->size(); j++ )
	{
		Girder* girder = (*Girders)[j];
		if( !girder->isActive )
		{
			continue;
		}
		//find the other bolt

		float currentLength = GetBoltDistance( girder->Bolt1, girder->Bolt2 );
		float stress = girder->GetStressForce( currentLength );
		girder->CurrentStress = stress;
		if( fabs(girder->CurrentStress) > fabs(girder->MaxStress) )
		{
			girder->MaxStress = girder->CurrentStress;
		}
		if( fabs(girder->CurrentStress) > Girder::GirderStrength )
		{
			girder->isActive = false;
		}
	}

	delete length;
	delete cosX;
	delete cosY;
	delete stiffness;
	delete jointDisplacementX;
	delete jointDisplacementY;
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
		if( IsSimulating() )// !IsPaused() )
		{
			glLoadIdentity();
			glPushMatrix();
			glLineWidth(0.75f);
			glColor3f( 1.0f ,1.0f ,1.0f ); 
			glBegin(GL_LINES);
			glVertex3f( Bolt->GetDrawX(), Bolt->GetDrawY(), 0.0f);
			//fprintf(stderr, "Force X: %f \t Force Y: %f \n", Bolt->forceX, Bolt->forceY );
			glVertex3f( Bolt->GetDrawX()+Bolt->forceX, Bolt->GetDrawY()+Bolt->forceY, 0.0f);
			glEnd();  
		}
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
	girder->Bolt2 = AddBolt(BoltLocX,BoltLocY, false);
	girder->isFinished = true;
	Girders->push_back(girder);
	girder->Index = Girders->size() - 1;
	
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
	girder->Index = Girders->size() - 1;
	
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
	}
	else
	{
		Bolt = new BOLT( BoltLocX , BoltLocY );
	}
	Bolts->push_back(Bolt);
	Bolt->Index = Bolts->size() -1;
	return Bolt;
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
		Girder* girder = (*Girders)[i];
		
		Serializer<<"GIRDER:";
		Serializer<<std::setprecision(0);
		Serializer<<girder->Bolt1->x<<","<<girder->Bolt1->y<<":"<<girder->Bolt2->x<<","<<girder->Bolt2->y<<std::endl;
	}
	Serializer.flush();
	Serializer.close();
}