#pragma once
#include <vector>

class GIRDER;
class BOLT;
class LAND;

class LEVEL
{
public:
	LEVEL();

	std::vector<GIRDER*>* Girders;
	std::vector<BOLT*>* Bolts;
	LAND* Land;

	void Update(float DeltaTime);

	void SimulatePhysics( float DeltaTime );
	void SimulatePhyscis2( float DeltaTime );

	void Draw();
	void drawLand();

	GIRDER* AddGirder( BOLT* Bolt1, float x, float y );
	GIRDER* AddGirder( BOLT* Bolt1, BOLT* Bolt2 );
	bool RemoveGirder( GIRDER* Girder );
	bool RemoveBolt( BOLT* bolt );


	BOLT* FindBolt(float x, float y);
	GIRDER* FindGirder(float x1, float y1, float x2, float y2);

	void StartSimulation();
	void EndSimulation();

	void SerializeLevel(char* FileName);
	void DeserializeLevel(char* FileName);


private:
	int MaxMoney;
	int CurrentlySpentMoney;
	float RoadLevel;

	bool CheckIfRoad( GIRDER* girder );
	void DrawRoad();
	BOLT* AddBolt(float x, float y, bool IsAnchor);

	friend class ACTION;
	friend class ADD_GIRDER;
	friend class REMOVE_GIRDER;
	friend class REMOVE_BOLT;
};
