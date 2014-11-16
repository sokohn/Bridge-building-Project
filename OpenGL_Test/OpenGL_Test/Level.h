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

	void Draw();

	bool AddGirder( BOLT* Bolt1, float x, float y );
	bool AddGirder( BOLT* Bolt1, BOLT* Bolt2 );
	bool RemoveGirder();
	bool RemoveBolt( BOLT* bolt );

};
