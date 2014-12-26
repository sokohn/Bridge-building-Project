#include "Util.h"

class LEVEL;

class ACTION
{
public:
	virtual void Perform(LEVEL* Level) { ; }
	virtual void Undo(LEVEL* Level) { ; }
};

class ADD_GIRDER : public ACTION
{
public:
	ADD_GIRDER(float X1, float Y1, float X2, float Y2);
	void Perform(LEVEL* Level);
	void Undo(LEVEL* Level);
	void FlipX();
	void FlipY();

	LOCATION B1;
	LOCATION B2;
protected:
	bool AddedB1;
	bool AddedB2;
};

class REMOVE_GIRDER : public ACTION
{
public:
	REMOVE_GIRDER(float X1, float Y1, float X2, float Y2);
	void Perform(LEVEL* Level);
	void Undo(LEVEL* Level);

	LOCATION B1;
	LOCATION B2;
};

class REMOVE_BOLT : public ACTION
{
public:
	REMOVE_BOLT(float X, float Y);
	void Perform(LEVEL* Level);
	void Undo(LEVEL* Level);

	LOCATION B1;
protected:
	LOCATION* ConnectedBolts;
	int NumConnectedBolts;
};