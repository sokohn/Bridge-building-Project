#pragma once
#include <vector>

class Girder;
class BOLT;

class LEVEL
{
public:
	std::vector<Girder*>* Girders;
	std::vector<BOLT*>* Bolts;

	LEVEL();

	void Update(float DeltaTime);

	void SimulatePhysics( float DeltaTime );

	void Draw();

	Girder* AddGirder( BOLT* Bolt1, float x, float y );
	Girder* AddGirder( BOLT* Bolt1, BOLT* Bolt2 );
	bool RemoveGirder();
	bool RemoveBolt( BOLT* bolt );

	void StartSimulation();
	void EndSimulation();

private:
	int MaxMoney;
	int CurrentlySpentMoney;
	float RoadLevel;

	bool CheckIfRoad( Girder* girder );
	void DrawRoad();


};
