// a file to contain various helper structs and consts that don't a better place to live
#pragma once

const int GridSpacing = 20;
const int MaxGirderLength = 169;
extern float ZoomLevel;

struct Color
{
	float red;
	float green;
	float blue;
	float alpha;

	Color()
	{
		Reset();
	}

	void Reset()
	{
		red = 0.0f;
		green = 0.0f;
		blue = 0.0f;
		alpha = 1.0;
	}
};

struct MOUSE
{
	float x;
	float y;
};

struct SCREEN
{
	float width;
	float height;
};

extern SCREEN Screen;

bool ShowStress();

float RoundToNearestGridMarker(float loc);

bool IsSimulating();
void SetSimulating(bool Sim);

bool IsPaused();
void SetPaused(bool Pause);

float maxf(float x, float y);
float minf(float x, float y);