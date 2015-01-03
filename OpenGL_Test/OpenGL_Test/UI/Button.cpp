#include "Button.h"
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <math.h>
#include <cstring>

extern float CameraX;
extern float CameraY;

//////////////////////////////////////////////////////////////////////////
////////////////////////////////TEXT_NODE/////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TEXT_NODE::TEXT_NODE(char* _Title, float X, float Y)
{
	Location.x = X;
	Location.y = Y;
	Title = _Title;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TEXT_NODE::TEXT_NODE(char* _Title, Vector2D Loc)
{
	Location = Loc;
	Title = _Title;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TEXT_NODE::Draw()
{
	float OffsetX = 0;
	float OffsetY = 0;

	OffsetX = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (unsigned char*)Title) / 2;
	OffsetY = glutBitmapHeight(GLUT_BITMAP_HELVETICA_18) / 2;

	glPushMatrix();
	glLoadIdentity();
	glColor3f(0.5f, 0.5f, 0.5f);

	glRasterPos2f(CameraX + (Location.x - OffsetX) / ZoomLevel, 2 / ZoomLevel + CameraY + (Location.y - OffsetY) / ZoomLevel);
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)Title);
	glPopMatrix();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool BUTTON_BASE::Intersects(float X, float Y)
{
	if (fabs(Location.x - X) > TextWidth / 2)
	{
		return false;
	}
	if (fabs(Location.y - Y) > TextHeight / 2)
	{
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void BUTTON_BASE::Hover()
{ 
	MouseHover = true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void BUTTON_BASE::Unhover()
{ 
	MouseHover = false;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BUTTON::BUTTON(char* _Title, float X, float Y, ClickFunc _Func)
{
	Location.x = X;
	Location.y = Y;
	Title = _Title;
	clickFunc = _Func;

	TextWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (unsigned char*)Title);
	TextHeight = 18;//glutBitmapHeight(GLUT_BITMAP_HELVETICA_18);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BUTTON::BUTTON(char* _Title, Vector2D Loc, ClickFunc _Func)
{
	Location = Loc;
	Title = _Title;
	clickFunc = _Func;

	TextWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (unsigned char*)Title);
	TextHeight = 18;// glutBitmapHeight(GLUT_BITMAP_HELVETICA_18);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void BUTTON::Draw()
{
	float TextX = CameraX + (Location.x - TextWidth / 2) / ZoomLevel;
	if (MouseHover)
	{
		glColor3f(0.2f, 0.2f, 0.0f);
	}
	else
	{
		glColor3f(0.0f, 0.0f, 0.0f);
	}
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
	glVertex3f(CameraX - TextPaddingSpace / ZoomLevel + (Location.x - TextWidth / 2) / ZoomLevel, CameraY - TextPaddingSpace / ZoomLevel + (Location.y - TextHeight / 2) / ZoomLevel, 0.0f);
	glVertex3f(CameraX + TextPaddingSpace / ZoomLevel + (Location.x + TextWidth / 2) / ZoomLevel, CameraY - TextPaddingSpace / ZoomLevel + (Location.y - TextHeight / 2) / ZoomLevel, 0.0f);
	glVertex3f(CameraX + TextPaddingSpace / ZoomLevel + (Location.x + TextWidth / 2) / ZoomLevel, CameraY + TextPaddingSpace / ZoomLevel + (Location.y + TextHeight / 2) / ZoomLevel, 0.0f);
	glVertex3f(CameraX - TextPaddingSpace / ZoomLevel + (Location.x - TextWidth / 2) / ZoomLevel, CameraY + TextPaddingSpace / ZoomLevel + (Location.y + TextHeight / 2) / ZoomLevel, 0.0f);
	glEnd();

	if (MouseHover)
	{
		glColor3f(0.8,0.8,0.5);
	}
	else
	{
		glColor3f(0.5f, 0.5f, 0.5f);
	}
	glBegin(GL_LINE_LOOP);
	glVertex3f(CameraX - TextPaddingSpace / ZoomLevel + (Location.x - TextWidth / 2) / ZoomLevel, CameraY - TextPaddingSpace / ZoomLevel + (Location.y - TextHeight / 2) / ZoomLevel, 0.0f);
	glVertex3f(CameraX + TextPaddingSpace / ZoomLevel + (Location.x + TextWidth / 2) / ZoomLevel, CameraY - TextPaddingSpace / ZoomLevel + (Location.y - TextHeight / 2) / ZoomLevel, 0.0f);
	glVertex3f(CameraX + TextPaddingSpace / ZoomLevel + (Location.x + TextWidth / 2) / ZoomLevel, CameraY + TextPaddingSpace / ZoomLevel + (Location.y + TextHeight / 2) / ZoomLevel, 0.0f);
	glVertex3f(CameraX - TextPaddingSpace / ZoomLevel + (Location.x - TextWidth / 2) / ZoomLevel, CameraY + TextPaddingSpace / ZoomLevel + (Location.y + TextHeight / 2) / ZoomLevel, 0.0f);
	glEnd();

	glRasterPos2f(CameraX + (Location.x - TextWidth / 2) / ZoomLevel, 2 / ZoomLevel + CameraY + (Location.y - TextHeight / 2) / ZoomLevel);
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)Title);

	glEnd();
}