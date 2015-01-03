#pragma once
#include <vector>
#include "Util.h"


class LAND
{
public:

	struct LAND_TRIANGLE
	{
		LAND_TRIANGLE(Vector2D* _p1, Vector2D* _p2, Vector2D* _p3 );
		Vector2D* P1;
		Vector2D* P2;
		Vector2D* P3;
		Color C;
	};

	LAND(float x1, float x2, float y1, float y2);

	void DrawLand();

	bool AddPoint( float x, float y );
	bool RemovePoint( float x, float y );

	void SerializeLand(char* FileName);
	void DeserializeLand(char* FileName);


private:

	std::vector<Vector2D*>* Land;
	std::vector<LAND_TRIANGLE*>* LandTriangles;

	void ConstructTriangles();
};
