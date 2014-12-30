#include <GL\glew.h>
#include <GL\freeglut.h>
#include <assert.h> 
#include "Land.h"
#include <iostream>
#include <stdlib.h> 

class EarNode
{
public:
	EarNode( LAND::LAND_POINT* _Point)
	{
		Point = _Point;
		Prev = NULL;
		Next = NULL;
	}
	LAND::LAND_POINT* Point;
	EarNode* Prev;
	EarNode* Next;
};

class EarList
{
public:
	EarList()
	{
		Head = NULL;
		NumNodes = 0;
	}
	~EarList();
	//doubly linked circular list
	EarNode* Head;
	int NumNodes;
	void AddNode(LAND::LAND_POINT* Point);
	void RemoveNode(EarNode* Node);
};

bool LAND::LAND_POINT::Equals(LAND_POINT* Point)
{
	return Point->x == x && Point->y == y;
}

LAND::LAND_TRIANGLE::LAND_TRIANGLE(LAND_POINT* _p1, LAND_POINT* _p2, LAND_POINT* _p3 )
{
	P1 = _p1;
	P2 = _p2;
	P3 = _p3;
	C.red = (rand()%256)/256.0;
	C.green = (rand()%256)/256.0;
	C.blue = (rand()%256)/256.0;
}

LAND::LAND(float x1, float x2, float y1, float y2)
{
	Land = new std::vector<LAND_POINT*>();
	LandTriangles = new std::vector<LAND_TRIANGLE*>();

	AddPoint(x1,y1);
	AddPoint(x2,y1);
	AddPoint(x2,y2);
	AddPoint(x1,y2);

}

void LAND::DrawLand()
{
	glLoadIdentity();
	glPushMatrix();
	for(int i =0; i<LandTriangles->size(); i++ )
	{
		glPushMatrix();
		glBegin(GL_TRIANGLES);
		glLineWidth(10.0f);	
		//glBegin(GL_LINE_LOOP);
		glColor3f( ((*LandTriangles)[i])->C.red, ((*LandTriangles)[i])->C.green ,((*LandTriangles)[i])->C.blue );
		glVertex3f( ((*LandTriangles)[i])->P1->x, ((*LandTriangles)[i])->P1->y, 0.0f );
		glVertex3f( ((*LandTriangles)[i])->P2->x, ((*LandTriangles)[i])->P2->y, 0.0f );
		glVertex3f( ((*LandTriangles)[i])->P3->x, ((*LandTriangles)[i])->P3->y, 0.0f );
		glEnd();       
		glPopMatrix();
	}
	glPopMatrix();

	/*if( Land->size() > 2 )
	{
		glLoadIdentity();
		glPushMatrix();
		glLineWidth(1.0f);	
		glBegin(GL_LINE_LOOP);
		glColor3f( 1.0f, 1.0f, 1.0f );  
		for(int i =0; i<Land->size(); i++ )
		{
			glVertex3f( ((*Land)[i])->x, ((*Land)[i])->y, 0.0f );      
		}
		glEnd();
		glPopMatrix();
	}*/
}


bool LAND::AddPoint( float x, float y )
{
	if( Land->size() < 4 )
	{
		//these are the anchor points, just add them
		Land->push_back(new LAND_POINT(x,y));
	}
	else
	{
		//find the index of this point based where it lies on the x direction between [0] and [size-3]

		//don't allow points to be added that are outisde of the bounds of the anchors
		if( x <= (*Land)[0]->x || x >= (*Land)[Land->size()-3]->x || y <= (*Land)[Land->size()-1]->y )
		{
			return false;
		}

		bool AddedPoint = false;
		for (std::vector<LAND_POINT*>::iterator it=Land->begin(); it<Land->end()-2; it++)
		{
			if( x < (*it)->x )
			{
				Land->insert(it, new LAND_POINT(x,y) );
				AddedPoint = true;
				break;
			}
			if( x == (*it)->x )
			{
				//no vertical lines allowed
				return false;
				/*if( y == (*it)->y )
				{
					return false;
				}
				
				//if these points share the same x, we want to sort by least Y travel from the preceding point
				if( fabs( (*(it-1))->y - (*it)->y ) > fabs( (*(it-1))->y - y ) )
				{
					Land->insert(it, new LAND_POINT(x,y) );
					AddedPoint = true;
					break;
				}*/
			}
		}
		if( !AddedPoint )
		{
			return false;
		}
	}
	if( Land->size() > 2 )
	{
		//triangulate the land
		for( int i =0; i < LandTriangles->size(); i++ )
		{
			delete (*LandTriangles)[i];
		}
		LandTriangles->clear();
		ConstructTriangles();
		return true;
	}
}

bool IsPointInTriangle(LAND::LAND_POINT* Point, LAND::LAND_POINT* t0, LAND::LAND_POINT* t1, LAND::LAND_POINT* t2 )
{
	float Area = 1.0f/2.0f*(-t1->y*t2->x + t0->y*(-t1->x + t2->x) + t0->x*(t1->y - t2->y) + t1->x*t2->y);

	float s = 1/(2*Area)*(t0->y*t2->x - t0->x*t2->y + (t2->y - t0->y)*Point->x + (t0->x - t2->x)*Point->y);
	float t = 1/(2*Area)*(t0->x*t1->y - t0->y*t1->x + (t0->y - t1->y)*Point->x + (t1->x - t0->x)*Point->y);
	if( s < 0 || s > 1 )
	{
		return false;
	}
	if( t < 0 || t > 1 )
	{
		return false;
	}

	return s + t <= 1;
}

void LAND::ConstructTriangles()
{
	EarList List;
	for( int i =0; i < Land->size(); i++ )
	{
		List.AddNode( (*Land)[i] );
	}

	//I know its a O(n^3) algorithm, but it runs so infrequently that I don't think it matters
	bool PrunedEar = false;
	EarNode* Node = NULL;
	while(List.NumNodes > 3 )
	{
		PrunedEar = false;
		Node = List.Head;
		do
		{
			bool FoundEar = true;
			//now go through all of the nodes and see if they are inside the triangle, if none are, then we add the triangle and prune the middle node
			for(int i =0; i < Land->size(); i++)
			{
				if( (*Land)[i] != Node->Point && (*Land)[i] != Node->Prev->Point && (*Land)[i] != Node->Next->Point )
				{
					if( IsPointInTriangle( (*Land)[i], Node->Prev->Point, Node->Point, Node->Next->Point) )
					{
						FoundEar = false;
						break;
					}
				}
			}

			if( FoundEar )
			{
				PrunedEar = true;
				LAND_TRIANGLE* Triangle = new LAND_TRIANGLE(Node->Prev->Point, Node->Point, Node->Next->Point);
				LandTriangles->push_back(Triangle);
				List.RemoveNode(Node);
				break;
			}

			Node = Node->Next;
		}while(Node!= List.Head);

		if( !PrunedEar )
		{
			//something failed
			assert(false);
			break;
		}
	}

	if( List.NumNodes == 3)
	{
		LAND_TRIANGLE* Triangle = new LAND_TRIANGLE(List.Head->Prev->Point, List.Head->Point, List.Head->Next->Point);
		LandTriangles->push_back(Triangle);
	}
}

EarList::~EarList()
{
	while( Head != NULL )
	{
		RemoveNode(Head);
	}
}

void EarList::AddNode(LAND::LAND_POINT* Point)
{
	EarNode* Node = new EarNode( Point );
	if( Head == NULL )
	{
		Head = Node;
		Head->Next = Head;
		Head->Prev = Head;
	}
	else
	{
		Node->Prev = Head->Prev;
		Head->Prev->Next = Node;
		Node->Next = Head;
		Head->Prev = Node;
	}
	NumNodes++;
}

void EarList::RemoveNode(EarNode* Node)
{
	if( Node == Head )
	{
		if( Head != Head->Next )
		{
			Head = Head->Next;
		}
		else
		{
			assert(NumNodes == 1 );
			Head = NULL;
		}
	}
	Node->Next->Prev= Node->Prev;
	Node->Prev->Next = Node->Next;
	NumNodes--;
	delete Node;
}