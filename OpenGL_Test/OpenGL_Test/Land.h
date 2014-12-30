#pragma once
#include <vector>
#include "Util.h"

class LAND
{
public:
	class LAND_POINT
	{
	public:
		LAND_POINT(float _x, float _y)
		{
			x = _x;
			y = _y;
		}
		float x;
		float y;

		bool Equals(LAND_POINT* Point);
	};

	struct LAND_TRIANGLE
	{
		LAND_TRIANGLE(LAND_POINT* _p1, LAND_POINT* _p2, LAND_POINT* _p3 );
		LAND_POINT* P1;
		LAND_POINT* P2;
		LAND_POINT* P3;
		Color C;
	};

	LAND(float x1, float x2, float y1, float y2);

	void DrawLand();

	bool AddPoint( float x, float y );
	bool RemovePoint( float x, float y );

	void SerializeLand(char* FileName);
	void DeserializeLand(char* FileName);


private:

	std::vector<LAND_POINT*>* Land;
	std::vector<LAND_TRIANGLE*>* LandTriangles;

	void ConstructTriangles();
};
