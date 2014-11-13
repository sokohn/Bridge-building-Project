#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>

using namespace std;

void changeViewport(int w, int h)
{
	glViewport(0,0,w,h);
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//actually draw code goes here
	 glLoadIdentity();                       // Reset The View

	glutSwapBuffers();
}

int main( int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );

	glutInitWindowSize( 800, 600 );
	glutCreateWindow( "Hello assholes" );
	glutReshapeFunc( changeViewport );
	glutDisplayFunc( render );

	GLenum err = glewInit();
	if( err != GLEW_OK )
	{
		fprintf(stderr, "GLEW error");
		return 1;
	}

	glutMainLoop();
	return 0;

}