#pragma once
#include <vector>

class Girder;
class BOLT;
class LAND;

class LEVEL
{
public:
	LEVEL();

	std::vector<Girder*>* Girders;
	std::vector<BOLT*>* Bolts;
	LAND* Land;

	void Update(float DeltaTime);

	void SimulatePhysics( float DeltaTime );
	void SimulatePhyscis2( float DeltaTime );

	void Draw();
	void drawLand();

	Girder* AddGirder( BOLT* Bolt1, float x, float y );
	Girder* AddGirder( BOLT* Bolt1, BOLT* Bolt2 );
	bool RemoveGirder();
	bool RemoveBolt( BOLT* bolt );

	void StartSimulation();
	void EndSimulation();

	void SerializeLevel(char* FileName);
	void DeserializeLevel(char* FileName);


private:
	int MaxMoney;
	int CurrentlySpentMoney;
	float RoadLevel;

	bool CheckIfRoad( Girder* girder );
	void DrawRoad();
	BOLT* AddBolt(float x, float y, bool IsAnchor);

};
